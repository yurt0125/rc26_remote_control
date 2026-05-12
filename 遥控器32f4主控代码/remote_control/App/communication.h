#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include "dma.h"
#include <stdint.h>
#include "Datapool.h"
#include "stm32f4xx.h"
#define RING_BUF_SIZE 256
#define DMA_BUF_SIZE  64

/* 环形缓冲区结构 */
typedef struct {
    uint8_t buffer[RING_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} comm_FIFO_t;

/* 强制一字节对齐的数据帧 */
#pragma pack(push, 1)

// 发送帧：摇杆4个通道 (4 x 16位)
typedef struct {
    uint8_t header[2]; // e.g. 0xAA 0x55
    uint16_t ch1;
    uint16_t ch2;
    uint16_t ch3;
    uint16_t ch4;
    uint16_t key;
    uint8_t crc;
    uint8_t tail;      // e.g. 0xDE
} JoystickFrame_t;

// 接收帧：XYZ (3 x 16位)
typedef struct {
    uint8_t header[2]; // e.g. 0x55 0xAA
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint8_t crc;
    uint8_t tail;      // e.g. 0xED
} XYZFrame_t;

#pragma pack(pop)

/* 全局通信上下文 */
typedef struct {
    UART_HandleTypeDef* huart;  

    /* 专供 DMA 直接收发的线性缓冲区 */
    uint8_t dma_rx_buf[DMA_BUF_SIZE];
    uint8_t dma_tx_buf[DMA_BUF_SIZE];

    /* 用于业务逻辑和DMA之间解耦的环形缓冲区 */
    comm_FIFO_t rx_fifo;
    comm_FIFO_t tx_fifo;

    volatile uint8_t tx_busy; // 发送忙碌标志

    /* 解析出来/待发送的业务数据 */
    uint16_t send_joystick[4]; 
    uint16_t send_key;
    uint16_t recv_xyz[3];
} CommContext;

extern CommContext g_Comm;

void Communication_Task_Init(UART_HandleTypeDef *huart);
void Communication_Task_Loop(void);

// 通用数据发送接口：把任何自定义的帧压入发送队列
void Communication_SendData(const uint8_t* data, uint16_t len);

// 业务层接口：设置当前要发送的摇杆和按键数据
void Communication_SetJoystickAndKeyData(uint16_t ch1, uint16_t ch2, uint16_t ch3, uint16_t ch4, uint16_t key);

// HAL 中断相关的回调接口
void Comm_Timer_Callback_Wrapper(void);
void Comm_UartRx_Callback_Wrapper(UART_HandleTypeDef *huart, uint16_t size);
void Comm_UartTxCplt_Callback_Wrapper(UART_HandleTypeDef *huart);
void Comm_UartError_Callback_Wrapper(UART_HandleTypeDef *huart);

#endif
