#ifndef __TJC_HUART_HMI_H
#define __TJC_HUART_HMI_H

#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include "dma.h"
#include <stdint.h>

#define RING_BUF_SIZE 256
#define DMA_BUF_SIZE  64



typedef struct {
    uint8_t buffer[RING_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} hmi_FIFO_t;

#pragma pack(push, 1)

typedef struct {
    uint8_t header[2]; // e.g. 0x55 0xAA
    uint8_t command;
    uint8_t load1;
    uint8_t load2;
    uint8_t tail;      // e.g. 0xDE
} HMIFrame_t;

#pragma pack(pop)

typedef struct {
    UART_HandleTypeDef* huart;  

    uint8_t dma_rx_buf[DMA_BUF_SIZE];
    uint8_t dma_tx_buf[DMA_BUF_SIZE];

    hmi_FIFO_t rx_fifo;
    hmi_FIFO_t tx_fifo;

    volatile uint8_t tx_busy; 

    uint8_t tx_command; 
    uint8_t tx_load[2];
    uint8_t rx_command; 
    uint8_t rx_load[2];
} HMIContext;

extern HMIContext g_HMI;

void HMI_Task_Init(UART_HandleTypeDef *huart);
void HMI_Task_Loop(void);

// HAL 相关的回调接口
// void HMI_Timer_Callback_Wrapper(void);
void HMI_UartRx_Callback_Wrapper(UART_HandleTypeDef *huart, uint16_t size);
void HMI_UartTxCplt_Callback_Wrapper(UART_HandleTypeDef *huart);
void HMI_UartError_Callback_Wrapper(UART_HandleTypeDef *huart);

#endif
