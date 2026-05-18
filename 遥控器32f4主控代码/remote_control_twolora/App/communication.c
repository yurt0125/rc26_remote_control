#include "communication.h"
#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

CommContext g_Comm = {0};

/* ================= 环形缓冲区 (FIFO) 操作 ================= */
// 数据存入尾部 (tail)
static void FIFO_Push(comm_FIFO_t* fifo, uint8_t data) {
    uint16_t next = (fifo->tail + 1) % RING_BUF_SIZE;
    if (next != fifo->head) { 
        fifo->buffer[fifo->tail] = data;
        fifo->tail = next;
    } else {
        fifo->drop_cnt++; // 缓冲区满，丢包并计数
    }
}

// 主循环从头部 (head) 取出处理
static int FIFO_Pop(comm_FIFO_t* fifo, uint8_t* data) {
    if (fifo->head == fifo->tail) return 0; 
    *data = fifo->buffer[fifo->head];
    fifo->head = (fifo->head + 1) % RING_BUF_SIZE;
    return 1;
}

static uint16_t FIFO_Count(comm_FIFO_t* fifo) {
    return (fifo->tail + RING_BUF_SIZE - fifo->head) % RING_BUF_SIZE;
}
/* ========================================================= */

//0xD5
static const uint8_t crc8_dvb_table[256] = {
    0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54, 0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
    0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06, 0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
    0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0, 0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
    0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2, 0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
    0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9, 0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
    0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B, 0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
    0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D, 0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
    0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F, 0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
    0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB, 0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
    0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9, 0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
    0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F, 0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
    0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D, 0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
    0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26, 0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
    0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74, 0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
    0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82, 0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
    0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0, 0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
};

uint8_t crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;// initial value
    for (uint8_t i = 0; i < len; i++) {
        crc = crc8_dvb_table[crc ^ data[i]];
    }
    return crc;
}

void Communication_Task_Init(UART_HandleTypeDef *txhuart, UART_HandleTypeDef *rxhuart)
{
    g_Comm.txhuart = txhuart;
    g_Comm.rxhuart = rxhuart;
    g_Comm.tx_busy = 0;
    g_Comm.rx_fifo.drop_cnt = 0;
    g_Comm.tx_fifo.drop_cnt = 0;
    
    // 初始化一些摇杆测试数据
    g_Comm.send_joystick[0] = 0x3412;
    g_Comm.send_joystick[1] = 0x7856;
    g_Comm.send_joystick[2] = 0xBC9A;
    g_Comm.send_joystick[3] = 0xF0DE;

    g_Comm.send_key = 0x3412;
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    
    // 挂载 DMA 接收空闲中断至 DMA 专用连续缓冲区
    HAL_UARTEx_ReceiveToIdle_DMA(g_Comm.rxhuart, g_Comm.dma_rx_buf, DMA_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(g_Comm.rxhuart->hdmarx, DMA_IT_HT);
}

void Communication_Task_Loop(void)
{
    // 不断处理 RX FIFO 里面收到的数据，直到剩余数据不足一帧长度
    while (FIFO_Count(&g_Comm.rx_fifo) >= sizeof(XYZFrame_t)) {
        
        uint16_t t_head = g_Comm.rx_fifo.head;
        uint8_t byte1 = g_Comm.rx_fifo.buffer[t_head];
        uint8_t byte2 = g_Comm.rx_fifo.buffer[(t_head + 1) % RING_BUF_SIZE];
        
        // 查找帧头 0x55 0xAA
        if (byte1 == 0x55 && byte2 == 0xAA) {
            uint8_t frame_buf[sizeof(XYZFrame_t)];
            uint16_t p = t_head;
            
            // 复制疑似一帧的所有数据
            for(int i = 0; i < sizeof(XYZFrame_t); i++) {
                frame_buf[i] = g_Comm.rx_fifo.buffer[p];
                p = (p + 1) % RING_BUF_SIZE;
            }
            
            XYZFrame_t* pFrame = (XYZFrame_t*)frame_buf;
            
            // 验证帧尾是否对应 0xED
            if (pFrame->tail == 0xED && pFrame->crc == crc8(frame_buf+2, sizeof(XYZFrame_t) - 4)) {
                // 提取解包好的 XYZ 数据 (均为 16位 uint16_t 数据)
                g_Comm.recv_x = pFrame->x;
                g_Comm.recv_y = pFrame->y;
                g_Comm.recv_z = pFrame->z;
                g_Comm.recv_status = pFrame->status;
                g_Comm.recv_mode = pFrame->mode;
                g_Comm.recv_command1 = pFrame->command1;
                g_Comm.recv_command2 = pFrame->command2;

                // 将 FIFO 头部读取指针越过已经正确消费的这一帧
                g_Comm.rx_fifo.head = p; 
				rx_stamp=HAL_GetTick();
				rx_cnt++;
                HMI_SendDataFrame(g_Comm.recv_x, g_Comm.recv_y, g_Comm.recv_z,
                                  g_Comm.recv_status, g_Comm.recv_mode,
                                  g_Comm.recv_command1, g_Comm.recv_command2);
            } else {
                // 坏帧，跳过头部第一个错误字节，继续往后寻找
                g_Comm.rx_fifo.head = (g_Comm.rx_fifo.head + 1) % RING_BUF_SIZE;
            }
        } else {
            // 没有找到帧头，抛弃头部第一字节，继续循环寻头
            g_Comm.rx_fifo.head = (g_Comm.rx_fifo.head + 1) % RING_BUF_SIZE;
        }
    }
}

void Comm_Timer_Callback_Wrapper(void)
{
    Communication_SetJoystickAndKeyData(joystick_Buf[0],joystick_Buf[1],joystick_Buf[2],joystick_Buf[3],tx_button_state);
		JoystickFrame_t frame;
    frame.header[0] = 0xAA;
    frame.header[1] = 0x55;
    // 把 4 个16位的摇杆通道数据打包
    frame.ch1 = g_Comm.send_joystick[0];
    frame.ch2 = g_Comm.send_joystick[1];
    frame.ch3 = g_Comm.send_joystick[2];
    frame.ch4 = g_Comm.send_joystick[3];
    frame.key = g_Comm.send_key;
    frame.crc = crc8((uint8_t*)&frame + 2, sizeof(JoystickFrame_t) - 4);
    frame.tail = 0xDE;

    uint8_t* ptr = (uint8_t*)&frame;
    for (int i = 0; i < sizeof(JoystickFrame_t); i++) {
        FIFO_Push(&g_Comm.tx_fifo, ptr[i]);
    }
        
    // 如果底部DMA且模块空闲（AUX为高电平），且队列里有东西，则通知任务立即发送
    if (g_Comm.tx_busy == 0 && FIFO_Count(&g_Comm.tx_fifo) > 0) {
        if(HAL_GPIO_ReadPin(TXSX1281_AUX_GPIO_Port, TXSX1281_AUX_Pin) == GPIO_PIN_SET)
        {
            extern osThreadId_t TxBufferToDMAHandle;
            if (xPortIsInsideInterrupt()) {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                vTaskNotifyGiveFromISR((TaskHandle_t)TxBufferToDMAHandle, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            } else {
                xTaskNotifyGive((TaskHandle_t)TxBufferToDMAHandle);
            }
        }
    }
}


void Communication_SendData(const uint8_t* data, uint16_t len)
{
    if (data == NULL || len == 0) return;

    __disable_irq(); 
    for (uint16_t i = 0; i < len; i++) {
        FIFO_Push(&g_Comm.tx_fifo, data[i]);
    }
    __enable_irq();

    if (g_Comm.tx_busy == 0 && FIFO_Count(&g_Comm.tx_fifo) > 0) {
        if(HAL_GPIO_ReadPin(TXSX1281_AUX_GPIO_Port, TXSX1281_AUX_Pin) == GPIO_PIN_SET)
        {
            extern osThreadId_t TxBufferToDMAHandle;
            if (xPortIsInsideInterrupt()) {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                vTaskNotifyGiveFromISR((TaskHandle_t)TxBufferToDMAHandle, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            } else {
                xTaskNotifyGive((TaskHandle_t)TxBufferToDMAHandle);
            }
        }
    }
}

void Communication_SetJoystickAndKeyData(uint16_t ch1, uint16_t ch2, uint16_t ch3, uint16_t ch4, uint16_t TX_Button_State)
{
    g_Comm.send_joystick[0] = ch1;
    g_Comm.send_joystick[1] = ch2;
    g_Comm.send_joystick[2] = ch3;
    g_Comm.send_joystick[3] = ch4;
    g_Comm.send_key = TX_Button_State;
}

void TxBufferToDMA(UART_HandleTypeDef *txhuart)
{
    if (txhuart != g_Comm.txhuart) return;

    // 关中断：防止 DMA 完成 ISR 或 AUX EXTI 与任务并发操作 FIFO
    __disable_irq();

    uint16_t count = FIFO_Count(&g_Comm.tx_fifo);
    if (count > 0) {
        if (count > DMA_BUF_SIZE) count = DMA_BUF_SIZE;

        for (uint16_t i = 0; i < count; i++) {
            FIFO_Pop(&g_Comm.tx_fifo, &g_Comm.dma_tx_buf[i]);
        }

        g_Comm.tx_busy = 1;
        HAL_UART_Transmit_DMA(g_Comm.txhuart, g_Comm.dma_tx_buf, count);
        tx_cnt++;
    } else {
        g_Comm.tx_busy = 0;
    }

    __enable_irq();
}

void Comm_UartRx_Callback_Wrapper(UART_HandleTypeDef *rxhuart, uint16_t size)
{
    if (g_Comm.rxhuart == rxhuart) {
        // DMA 中断来了，将其固定缓存段里收到的数据复制到业务层的 RX 环形缓冲区中
        for (uint16_t i = 0; i < size; i++) {
            FIFO_Push(&g_Comm.rx_fifo, g_Comm.dma_rx_buf[i]);
        }
        
        // 立即开启下一次 DMA 接收，防止漏包
        HAL_UARTEx_ReceiveToIdle_DMA(g_Comm.rxhuart, g_Comm.dma_rx_buf, DMA_BUF_SIZE);
        __HAL_DMA_DISABLE_IT(g_Comm.rxhuart->hdmarx, DMA_IT_HT);
    }
}

void Comm_UartError_Callback_Wrapper(UART_HandleTypeDef *errorhuart)
{
    if (g_Comm.rxhuart == errorhuart) {
        // 如果断线或者产生溢出错误，需解挂并重启接收以自我恢复
        HAL_UARTEx_ReceiveToIdle_DMA(g_Comm.rxhuart, g_Comm.dma_rx_buf, DMA_BUF_SIZE);
    }
    else if (g_Comm.txhuart == errorhuart)
    {
        // 发送过程中出错，直接标记发送空闲，等待下次发送触发
        g_Comm.tx_busy = 0;
    }
}


