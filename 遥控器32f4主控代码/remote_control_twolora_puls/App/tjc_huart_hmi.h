#ifndef __TJC_HUART_HMI_H
#define __TJC_HUART_HMI_H

#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include "dma.h"
#include <stdint.h>
#include "communication.h"
#define RING_BUF_SIZE 512
#define DMA_BUF_SIZE  128



typedef struct {
    uint8_t buffer[RING_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t drop_cnt; // 缓冲区满时丢包计数
} hmi_FIFO_t;

#pragma pack(push, 1)

//Page Frame
typedef struct {
    uint8_t header[2]; // e.g. 0x55 0xAA
    uint8_t page_id;   // e.g. 0x00-0x03,00表示主页，01-03分别表示表示数据设置，数据显示，发送命令界面
    uint8_t tail;      // e.g. 0x0A
} PageFrame_t;

//Setting Frame
typedef struct {
    uint8_t header[2]; // e.g. 0x55 0xBB
    uint8_t command;
    uint8_t load1;
    uint8_t load2;
    uint8_t tail;      // e.g. 0x0B
} SettingFrame_t;

//Data Frame
typedef struct {
    uint8_t header[2]; // e.g. 0x55 0xCC
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t status;   // bit5-3:夹爪状态 bit2-1：吸盘状态 bit0:自动模式状态
    uint8_t mode;
    uint8_t send_command1;
    uint8_t send_command2;
    uint8_t KFS_want_place1;//（高四位为索引为1的位置，低四位为索引为0位置）
    uint8_t KFS_want_place2;//（低四位为索引为2位置）
    uint8_t spear;     //0x00-0x07分别表示不同的武器头夹取状态
    uint8_t KFS_Keepplace; //KFS存储区
    uint8_t tail;      // e.g. 0x0C
} DataFrame_t;

//Command Frame
typedef struct {
    uint8_t header[2]; // e.g. 0x55 0xDD
    uint8_t command;
    uint8_t load1;
    uint8_t load2;
    uint8_t tail;      // e.g. 0x0D
} CommandFrame_t;

#pragma pack(pop)

typedef struct {
    UART_HandleTypeDef* huart;  

    uint8_t dma_rx_buf[DMA_BUF_SIZE];
    uint8_t dma_tx_buf[DMA_BUF_SIZE];

    hmi_FIFO_t rx_fifo;
    hmi_FIFO_t tx_fifo;

    volatile uint8_t tx_busy; 

    // PageFrame (0x55 0xAA) 接收存储
    uint8_t page_id;

    // SettingFrame (0x55 0xBB) 接收存储
    uint8_t setting_tx_command; 
    uint8_t setting_tx_load[2];
    uint8_t setting_rx_command; 
    uint8_t setting_rx_load[2];

    // DataFrame (0x55 0xCC) 接收存储
    int16_t data_send_x;
    int16_t data_send_y;
    int16_t data_send_z;
    uint8_t  data_send_status;
    uint8_t  data_send_mode;
    uint8_t  data_send_command[2];
    uint8_t  data_send_KFS_want_place1;
    uint8_t  data_send_KFS_want_place2;
    uint8_t  data_send_spear;
    uint8_t  data_send_KFS_Keepplace;

    // CommandFrame (0x55 0xDD) 接收存储
    uint8_t rx_command;
    uint8_t rx_load[2];

    // 最近一帧 DataFrame 有效标记（切回 page2 时主动刷新用）
    uint8_t last_data_valid;
} HMIContext;

extern HMIContext g_HMI;

void HMI_Task_Init(UART_HandleTypeDef *huart);
void HMI_Task_Loop(void);

// 发送接口 — 仅数据设置(0xBB) 和 数据显示(0xCC) 需要发送
void HMI_SendSettingFrame(uint8_t command, uint8_t load1, uint8_t load2);
void HMI_SendDataFrame(int16_t x, int16_t y, int16_t z,
                       uint8_t status, uint8_t mode,
                       uint8_t send_cmd1, uint8_t send_cmd2,
                       uint8_t KFS_want_place1, uint8_t KFS_want_place2,
                       uint8_t spear, uint8_t KFS_Keepplace);
void HMI_ButtonTransmitFrame(uint8_t command, uint8_t load1, uint8_t load2);

// HAL 相关的回调接口
// void HMI_Timer_Callback_Wrapper(void);
void HMI_UartRx_Callback_Wrapper(UART_HandleTypeDef *huart, uint16_t size);
void HMI_UartTxCplt_Callback_Wrapper(UART_HandleTypeDef *huart);
void HMI_UartError_Callback_Wrapper(UART_HandleTypeDef *huart);

#endif
