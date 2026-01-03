
#include "uart_dma.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/uhci.h"
#include "string.h"
#include "driver/gpio.h"

#define TXD_PIN (CONFIG_UARTDMA_UART_TXD)
#define RXD_PIN (CONFIG_UARTDMA_UART_RXD)

// 接收回调结构体定义，如果要使用双buf接收，那么需要定义两个buf空间
typedef struct
{
    QueueHandle_t queue;     // 用于接收DMA传输完成事件的队列句柄
    uint8_t recv_buf1[UART_DMA_RX_BUF_SIZE]; // 指向接收缓冲区的指针，用于存储DMA接收到的数据
    uint8_t recv_buf2[UART_DMA_RX_BUF_SIZE]; // 指向接收缓冲区的指针，用于存储DMA接收到的数据
    bool use_buf;            // 确定双buf的接收
    uint32_t data_len;       // 接收缓冲区中当前已接收的数据长度
} dual_buf_uhci_ctx_t;

dual_buf_uhci_ctx_t ctx;                 // 定义ctx回调参数
uhci_controller_handle_t uhci_ctrl;

static bool uhci_tx_callback(uhci_controller_handle_t uhci_ctrl, const uhci_tx_done_event_data_t *edata, void *user_ctx)
{

    return true;
}

static bool uhci_rx_callback(uhci_controller_handle_t uhci_ctrl, const uhci_rx_event_data_t *edata, void *user_ctx)
{
    dual_buf_uhci_ctx_t *ctx = (dual_buf_uhci_ctx_t *)user_ctx;
    uart_dma_receive_event_t evt;

    ctx->data_len = edata->recv_size;

    if (ctx->use_buf)
    {
        // 用于复制数据
        memcpy(ctx->recv_buf1, edata->data, ctx->data_len);
    }
    else
    {
        // 用于复制数据
        memcpy(ctx->recv_buf2, edata->data, ctx->data_len);
    }

    ctx->use_buf = !ctx->use_buf;

    evt = edata->flags.totally_received ? UHCI_EVT_EOF : UHCI_EVT_PARTIAL_DATA;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(ctx->queue, &evt, &xHigherPriorityTaskWoken);
    return xHigherPriorityTaskWoken == pdTRUE;
}

void uart_dma_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = CONFIG_UARTDMA_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // ESP_ERROR_CHECK(uart_set_rx_timeout(UART_NUM_1, 10));

    // 配置 UHCI 控制器，用于 UART DMA 传输
    uhci_controller_config_t uhci_cfg = {
        .uart_port = UART_NUM_1,       // 指定使用的 UART 端口
        .tx_trans_queue_depth = 4,        // 发送传输队列深度（最多支持 4 个同时在队列中的传输任务）
        .max_receive_internal_mem = UART_DMA_RX_BUF_SIZE, // UHCI 内部最多接收数据的内存大小
        .max_transmit_size = UART_DMA_TX_BUF_SIZE,        // 单次传输的最大数据长度
        .dma_burst_size = 32,             // DMA 传输突发长度，单位为字节
        .rx_eof_flags.idle_eof = 1,       // 空闲帧也会作为接收结束标志
        .rx_eof_flags.rx_brk_eof = 1, 
    };
    ESP_ERROR_CHECK(uhci_new_controller(&uhci_cfg, &uhci_ctrl));
    ctx.queue = xQueueCreate(4, sizeof(uart_dma_receive_event_t));
    ctx.use_buf = true;
    // 注册中断回调事件
    uhci_event_callbacks_t cbs = {
        .on_rx_trans_event = uhci_rx_callback,
        .on_tx_trans_done = uhci_tx_callback,
    };
    ESP_ERROR_CHECK(uhci_register_event_callbacks(uhci_ctrl, &cbs, &ctx));
    // 启动接收函数
    ESP_ERROR_CHECK(uhci_receive(uhci_ctrl, ctx.recv_buf1, UART_DMA_RX_BUF_SIZE));
}

int uart_dma_sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

void uart_dma_start_receive()
{
    while(!is_uhci_fsm_enable(uhci_ctrl));
    ESP_ERROR_CHECK(uhci_receive(uhci_ctrl, ctx.use_buf ? ctx.recv_buf1 : ctx.recv_buf2, UART_DMA_RX_BUF_SIZE));
}

void uart_dma_transmit(uint8_t *write_buffer, size_t write_size)
{
    ESP_ERROR_CHECK(uhci_transmit(uhci_ctrl, write_buffer, write_size));
}

void uart_dma_wait_all_tx_transaction_done(int timeout_ms)
{
    ESP_ERROR_CHECK(uhci_wait_all_tx_transaction_done(uhci_ctrl, -1));
}

QueueHandle_t uart_dma_get_receive_event_queue()
{
    return ctx.queue;
}

void uart_dma_get_received_data(uint32_t *data_addr, uint32_t *len)
{
    if (ctx.use_buf)
        *data_addr = (uint32_t)ctx.recv_buf2;
    else    
        *data_addr = (uint32_t)ctx.recv_buf1;

    *len = ctx.data_len;
}