#ifndef COMM_WRAPPER_H
#define COMM_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usart.h"
#include <stdbool.h>

// 1. 初始化通信模块
void CommWrapper_Init(UART_HandleTypeDef *txhuart, UART_HandleTypeDef *rxhuart);

// 2. 放在主循环 while (1) 里的业务轮询
bool CommWrapper_Task_Loop(void);

// 3. 串口接收 DMA/空闲中断 回调
void CommWrapper_RxDMAToRxBuffer(UART_HandleTypeDef *rxhuart, uint16_t size);

// 4. 串口发送完成中断 回调
void CommWrapper_TxBufferToTxDMA(UART_HandleTypeDef *txhuart);

// 5. 业务：发送 XYZ 坐标数据
void CommWrapper_SendAxisData(uint16_t x, uint16_t y, uint16_t z,uint8_t status, uint8_t mode, uint8_t command1, uint8_t command2);

// 6. 获取接收到的数据
void CommWrapper_GetRecvData(uint16_t* joystick, uint16_t* key);

#ifdef __cplusplus
}
#endif

#endif // COMM_WRAPPER_H
