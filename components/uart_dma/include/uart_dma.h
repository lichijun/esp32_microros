#pragma once

#include "freertos/FreeRTOS.h"

#define UART_DMA_RX_BUF_SIZE 1024
#define UART_DMA_TX_BUF_SIZE 1024

typedef enum {
    UHCI_EVT_PARTIAL_DATA,
    UHCI_EVT_EOF,
} uart_dma_receive_event_t;

void uart_dma_init(void);
int uart_dma_sendData(const char* logName, const char* data);
void uart_dma_start_receive();
void uart_dma_transmit(uint8_t *write_buffer, size_t write_size);
void uart_dma_wait_all_tx_transaction_done(int timeout_ms);
QueueHandle_t uart_dma_get_receive_event_queue();
void uart_dma_get_received_data(uint32_t *data_addr, uint32_t *len);