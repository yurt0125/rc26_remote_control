#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include "dma.h"
#include <stdint.h>
#include "Datapool.h"
#include "stm32f4xx.h"
#include "tjc_huart_hmi.h"

#define RING_BUF_SIZE 512
#define DMA_BUF_SIZE  128
#define COMM_COMMAND_COUNT 12U

#ifndef COMM_TX_BUSY_TIMEOUT_MS
#define COMM_TX_BUSY_TIMEOUT_MS 50U
#endif

//测试通过 本机rx_drop_cnt累加
#ifndef COMM_TEST_BLOCK_RX_PARSE
// 置 1：停止消费 RX FIFO，但 DMA 接收回调仍继续写入。
// 现象：接收数据停止更新；FIFO 填满后 rx_fifo.drop_cnt 持续增加。
#define COMM_TEST_BLOCK_RX_PARSE 0
#endif

//测试通过 接收机rx_crc_error_cnt累加
#ifndef COMM_TEST_CORRUPT_TX_CRC
// 置 1：将所有发送帧的正确 CRC 按位取反，持续发送 CRC 错误帧。
// 现象：接收机拒绝这些帧，接收机的 rx_crc_error_cnt 持续增加。
#define COMM_TEST_CORRUPT_TX_CRC 0
#endif

//测试通过 本机tx_error_cnt=1
#ifndef COMM_TEST_FORCE_TX_DMA_FAIL_ONCE
// 置 1：第一次启动 TX DMA 时模拟返回 HAL_BUSY，之后恢复真实 HAL 调用。
// 现象：tx_error_cnt 增加 1，tx_busy 被清零，后续发送仍可继续。
#define COMM_TEST_FORCE_TX_DMA_FAIL_ONCE 0
#endif

//测试通过 本机rx_error_cnt=1
#ifndef COMM_TEST_FORCE_RX_DMA_START_FAIL_ONCE
// 置 1：第一次启动 RX DMA 时模拟返回 HAL_BUSY，之后恢复真实 HAL 调用。
// 现象：rx_error_cnt 增加 1；需要再次调用 RX DMA 启动函数才能开始接收。
#define COMM_TEST_FORCE_RX_DMA_START_FAIL_ONCE 0
#endif

/*时序图
AUX低电平代表busy，高电平代码空闲

时刻1：初始状态AUX高电平，tx_busy=0(锁)(空闲)

时刻2：1ms定时器轮询，发现tx_busy=0,向发送缓存区发送数据，并且触发DMA发送，tx_busy=1(忙)

时刻3：1ms定时器轮询，发现tx_busy=1，退出

时刻4：1ms定时器轮询，发现tx_busy=1，退出

时刻5：1ms定时器轮询，发现tx_busy=1，退出

时刻6：AUX上升沿触发，如果发送缓存区不为空：触发DMA发送；如果发送缓存区为空：tx_busy=0

时刻7：1ms定时器轮询，发现tx_busy=0,向发送缓存区发送数据，并且触发DMA发送，tx_busy=1(忙)

时刻8：循环*/

/* 环形缓冲区结构 */
typedef struct {
    uint8_t buffer[RING_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t drop_cnt; // 缓冲区满时丢包计数
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
    uint8_t page;
    uint8_t crc;
    uint8_t tail;      // e.g. 0xDE
} JoystickFrame_t;

//发送帧：发送设置好的KFS位置
typedef struct {
    uint8_t header[2]; // e.g. 0xAA 0x66
    uint8_t command;   // e.g. 0x01表示发送KFS位置
    uint8_t load1;     // e.g. comand=0x01时，load1表示高四位为索引为1的位置，低四位为索引为0位置
    uint8_t load2;     // e.g. comand=0x01时，load2表示高四位为索引为3的位置，低四位为索引为2位置
    uint8_t crc;
    uint8_t tail;      // e.g. 0xDE
} CommSettingFrame_t;

//发送帧： 串口屏发送的命令，转发给机器人
typedef struct {
    uint8_t header[2]; // 0xAA 0x77
    uint8_t command;   //0-99分别表示不同的命令，在机器人自己查表
    uint8_t load1;     //发送的次数，累计值，8位 0-255
    uint8_t load2;     //置空，保留扩展
    uint8_t crc;
    uint8_t tail;      // e.g. 0xDE
} CommCommandFrame_t;

// 接收帧：XYZ (3 x 16位 有符号)
typedef struct {
    uint8_t header[2]; // e.g. 0x55 0xAA
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t status;   // bit5-3:夹爪状态 bit2-1：吸盘状态 bit0:自动模式状态
    uint8_t mode;
    uint8_t command1;
    uint8_t command2;
    uint8_t KFS_want_place1;//（高四位为索引为1的位置，低四位为索引为0位置）
    uint8_t KFS_want_place2;//（低四位为索引为2位置）
    uint8_t spear;     //0x00-0x07分别表示不同的武器头夹取状态
    uint8_t KFS_Keepplace; //KFS存储区
    uint8_t crc;
    uint8_t tail;      // e.g. 0xED
} XYZFrame_t;

#pragma pack(pop)

/* 全局通信上下文 */
typedef struct {
    UART_HandleTypeDef* txhuart;  
    UART_HandleTypeDef* rxhuart;    

    /* 专供 DMA 直接收发的线性缓冲区 */
    uint8_t dma_rx_buf[DMA_BUF_SIZE];
    uint8_t dma_tx_buf[DMA_BUF_SIZE];

    /* 用于业务逻辑和DMA之间解耦的环形缓冲区 */
    comm_FIFO_t rx_fifo;
    comm_FIFO_t tx_fifo;

    volatile uint8_t tx_busy; // 发送忙碌标志
    volatile uint32_t tx_busy_start_tick;
    volatile uint16_t tx_timeout_recovery_cnt;
    volatile uint16_t tx_error_cnt;
    volatile uint16_t rx_error_cnt;
    volatile uint16_t rx_crc_error_cnt;

    /* 解析出来/待发送的业务数据 */
    uint16_t send_joystick[4]; 
    uint16_t send_key;
    uint8_t send_page;
    int16_t recv_x;
    int16_t recv_y;
    int16_t recv_z;
    uint8_t recv_status;
    uint8_t recv_mode;
    uint8_t recv_command1;
    uint8_t recv_command2;
    uint8_t recv_KFS_want_place1;
    uint8_t recv_KFS_want_place2;
    uint8_t recv_spear;
    uint8_t recv_KFS_Keepplace;

    /* 按键统计（发送侧） */
    uint16_t key_pressed_count;   // 当前帧被按下的按键个数
    uint16_t key_down_count;      // 累计上升沿次数
    uint16_t key_last_status;     // 上一帧按键状态
} CommContext;

extern CommContext g_Comm;

void Communication_Task_Init(UART_HandleTypeDef *txhuart, UART_HandleTypeDef *rxhuart);
void Communication_Task_Loop(void);

// 通用数据发送接口：把任何自定义的帧压入发送队列
void Communication_SendData(const uint8_t* data, uint16_t len);

// 业务层接口：设置当前要发送的摇杆和按键数据
void Communication_SetJoystickAndKeyData(uint16_t ch1, uint16_t ch2, uint16_t ch3, uint16_t ch4, uint16_t key, uint8_t page);

// 排队发送 SettingFrame：与其他重发帧轮转，三个100ms周期各发送一次
void Communication_SendSettingFrame(uint8_t command, uint8_t load1, uint8_t load2);

// 排队发送 CommandFrame：与其他重发帧轮转，三个100ms周期各发送一次
void Communication_SendCommandFrame(uint8_t command, uint8_t load1, uint8_t load2);

//真正通过DMA发送数据的函数，任务调用
uint16_t Communication_GetRxDropCnt(void);
uint16_t Communication_GetTxDropCnt(void);
uint16_t Communication_GetTxErrorCnt(void);
uint16_t Communication_GetTxTimeoutRecoveryCnt(void);
uint16_t Communication_GetRxErrorCnt(void);
uint16_t Communication_GetRxCrcErrorCnt(void);

// 通信统计快照全局变量，由 Communication_Task_Loop() 周期性更新
extern volatile uint16_t g_comm_rx_drop_cnt;
extern volatile uint16_t g_comm_tx_drop_cnt;
extern volatile uint16_t g_comm_tx_error_cnt;
extern volatile uint16_t g_comm_tx_timeout_recovery_cnt;
extern volatile uint16_t g_comm_rx_error_cnt;
extern volatile uint16_t g_comm_rx_crc_error_cnt;

void TxBufferToDMA(UART_HandleTypeDef *txhuart);

// HAL 中断相关的回调接口
void Comm_Timer_Callback_Wrapper(void);
void Comm_UartRx_Callback_Wrapper(UART_HandleTypeDef *rxhuart, uint16_t size);
void Comm_UartError_Callback_Wrapper(UART_HandleTypeDef *errorhuart);

#endif
