#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include <uros_network_interfaces.h>
#include "micro_ros_comm.h"
#include "uart_dma.h"
#include "comm.h"

static void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        char *data_to_send = "Hello world";
        uart_dma_transmit((uint8_t *)data_to_send, strlen(data_to_send));
        uart_dma_wait_all_tx_transaction_done(-1);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(UART_DMA_RX_BUF_SIZE + 1);
    uint32_t len;
    Comm_Cmd_e frameCmd;
    uint8_t *pFrameData = NULL;
    int64_t ts_last_imu_cmd = -1;
    while (1)
    {
        uart_dma_receive_event_t evt;
        if (xQueueReceive(uart_dma_get_receive_event_queue(), &evt, portMAX_DELAY) == pdTRUE)
        {
            if (evt == UHCI_EVT_EOF)
            {
                uart_dma_start_receive();
            }
            uart_dma_get_received_data((uint32_t *)&data, &len);
            
            if (len > 0)
            {
                data[len] = 0;
                // ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", len, data);
                // ESP_LOGI(RX_TASK_TAG, "%d", len);

                for (uint32_t i = 0; i < len; ++i)
                {
                    if (ParseFrame(data[i], &frameCmd, (uint32_t*)&pFrameData))
                    {
                        switch (frameCmd)
                        {
                        case COMM_CMD_TEST:
                        {
                            Comm_Data_Test_st* pData = (Comm_Data_Test_st*)pFrameData;
                            ESP_LOGI(RX_TASK_TAG, "Get CMD_TEST, data:%d %d %.2f", pData->u8Num,
                                     pData->u16Num, pData->fNum);
                                    //  publish_int32(pData->u8Num); // panic happen
                                    update_int32(pData->u8Num);
                            break;
                        }

                        case COMM_CMD_IMU:
                        {
                            Comm_Data_Imu_st* pData = (Comm_Data_Imu_st*)pFrameData;
                            // ESP_LOGI(RX_TASK_TAG, "Get CMD_IMU, data:%.2f %.2f %.2f %.2f %.2f %.2f", 
                            //     pData->fAccX, pData->fAccY, pData->fAccZ, pData->fGyroX, pData->fGyroY, pData->fGyroZ);
                            update_imu_data(pData->fAccX, pData->fAccY, pData->fAccZ, pData->fGyroX, pData->fGyroY, pData->fGyroZ);
                            // ESP_LOGI(RX_TASK_TAG, "1");
                            int64_t ts = esp_timer_get_time();
                            if (ts_last_imu_cmd != -1 && ts - ts_last_imu_cmd > 5000)
                                ESP_LOGW(RX_TASK_TAG, "too long, %dms", (ts - ts_last_imu_cmd)/1000);
                            ts_last_imu_cmd = ts;
                            break;
                        }
                        
                        default:
                            break;
                        }
                    }
                }
            }
        }
    }
    free(data);
}

void app_main(void)
{
    uart_dma_init();
    ESP_ERROR_CHECK(uros_network_interface_initialize()); // 初始化网络，连接WiFi信号

    xTaskCreate(rx_task, "uart_rx_task", CONFIG_EXAMPLE_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(tx_task, "uart_tx_task", CONFIG_EXAMPLE_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);
    xTaskCreatePinnedToCore(micro_ros_task,
                            "micro_ros_task",
                            16000,  // 保持 16000 不变
                            NULL,
                            14,  // 优先级设为 14（12~15 区间内）
                            NULL,
                            1  // 绑定到 APP_CPU（核心 1）
                            );
}
