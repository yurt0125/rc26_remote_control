#include "tjc_huart_hmi.h"
#include <string.h>

HMIContext g_HMI = {0};

/* ================= 环形缓冲区 (FIFO) 操作 ================= */
// 数据存入尾部 (tail)
static void FIFO_Push(hmi_FIFO_t* fifo, uint8_t data) {
    uint16_t next = (fifo->tail + 1) % RING_BUF_SIZE;
    if (next != fifo->head) { 
        fifo->buffer[fifo->tail] = data;
        fifo->tail = next;
    }
}

// 主循环从头部 (head) 取出处理
static int FIFO_Pop(hmi_FIFO_t* fifo, uint8_t* data) {
    if (fifo->head == fifo->tail) return 0; 
    *data = fifo->buffer[fifo->head];
    fifo->head = (fifo->head + 1) % RING_BUF_SIZE;
    return 1;
}

static uint16_t FIFO_Count(hmi_FIFO_t* fifo) {
    return (fifo->tail + RING_BUF_SIZE - fifo->head) % RING_BUF_SIZE;
}
/* ========================================================= */

void HMI_Task_Init(UART_HandleTypeDef *huart)
{
    g_HMI.huart = huart;
    g_HMI.tx_busy = 0;
    
    // 初始化一些摇杆测试数据
    g_HMI.tx_command = 0xFF;
    g_HMI.tx_load[0] = 0xFF;
    g_HMI.tx_load[1] = 0xFF;
    g_HMI.rx_command = 0xFF;
    g_HMI.rx_load[0] = 0xFF;
    g_HMI.rx_load[1] = 0xFF;
    
    // 挂载 DMA 接收空闲中断至 DMA 专用连续缓冲区
    HAL_UARTEx_ReceiveToIdle_DMA(g_HMI.huart, g_HMI.dma_rx_buf, DMA_BUF_SIZE);
}

void HMI_Task_Loop(void)
{
    if (g_HMI.tx_busy == 0 && FIFO_Count(&g_HMI.tx_fifo) > 0) {
        HMI_UartTxCplt_Callback_Wrapper(g_HMI.huart); 
    } 
    // 不断处理 RX FIFO 里面收到的数据，直到剩余数据不足一帧长度
    while (FIFO_Count(&g_HMI.rx_fifo) >= sizeof(HMIFrame_t)) {
        
        uint16_t t_head = g_HMI.rx_fifo.head;
        uint8_t byte1 = g_HMI.rx_fifo.buffer[t_head];
        uint8_t byte2 = g_HMI.rx_fifo.buffer[(t_head + 1) % RING_BUF_SIZE];
        
        // 查找帧头 0x55 0xAA
        if (byte1 == 0x55 && byte2 == 0xAA) {
            uint8_t frame_buf[sizeof(HMIFrame_t)];
            uint16_t p = t_head;
            
            // 复制疑似一帧的所有数据
            for(int i = 0; i < sizeof(HMIFrame_t); i++) {
                frame_buf[i] = g_HMI.rx_fifo.buffer[p];
                p = (p + 1) % RING_BUF_SIZE;
            }
            
            HMIFrame_t* pFrame = (HMIFrame_t*)frame_buf;
            
            // 验证帧尾是否对应 0xED
            if (pFrame->tail == 0xED) {
                // 提取解包好的 XYZ 数据 (均为 16位 uint16_t 数据)
                g_HMI.rx_command = pFrame->command;
                g_HMI.rx_load[0] = pFrame->load1;
                g_HMI.rx_load[1] = pFrame->load2;
                
                // 将 FIFO 头部读取指针越过已经正确消费的这一帧
                g_HMI.rx_fifo.head = p; 
            } else {
                // 坏帧，跳过头部第一个错误字节，继续往后寻找
                g_HMI.rx_fifo.head = (g_HMI.rx_fifo.head + 1) % RING_BUF_SIZE;
            }
        } else {
            // 没有找到帧头，抛弃头部第一字节，继续循环寻头
            g_HMI.rx_fifo.head = (g_HMI.rx_fifo.head + 1) % RING_BUF_SIZE;
        }
    }
}

// void HMI_Timer_Callback_Wrapper(void)
// {
        
// 		JoystickFrame_t frame;
//     frame.header[0] = 0xAA;
//     frame.header[1] = 0x55;
//     // 把 4 个16位的摇杆通道数据打包
//     frame.ch1 = g_HMI.tx_load[0];
//     frame.ch2 = g_HMI.tx_load[1];
//     frame.ch3 = g_HMI.tx_load[2];
//     frame.ch4 = g_HMI.tx_load[3];
//     frame.crc = crc8((uint8_t*)&frame + 2, sizeof(JoystickFrame_t) - 2);
//     frame.tail = 0xDE;

//     uint8_t* ptr = (uint8_t*)&frame;
//     for (int i = 0; i < sizeof(JoystickFrame_t); i++) {
//         FIFO_Push(&g_HMI.tx_fifo, ptr[i]);
//     }
        
//     // 尝试拉起发送：如果底部DMA空闲，且队列里有东西
//     if (g_HMI.tx_busy == 0 && FIFO_Count(&g_HMI.tx_fifo) > 0) {
//        HMI_UartTxCplt_Callback_Wrapper(g_HMI.huart); 
//     }
// }


void HMI_SendData(uint8_t command, uint8_t load1, uint8_t load2)
{
    if (command ==0x00) return;

    __disable_irq(); 
    g_HMI.tx_command = command;
    g_HMI.tx_load[0] = load1;
    g_HMI.tx_load[1] = load2;
    __enable_irq();

    if (g_HMI.tx_busy == 0 && FIFO_Count(&g_HMI.tx_fifo) > 0) {
        HMI_UartTxCplt_Callback_Wrapper(g_HMI.huart); 
    }
}

void HMI_UartTxCplt_Callback_Wrapper(UART_HandleTypeDef *huart)
{
    if (huart == g_HMI.huart) {
        uint16_t count = FIFO_Count(&g_HMI.tx_fifo);
        if (count > 0) {
            if (count > DMA_BUF_SIZE) count = DMA_BUF_SIZE;
            
            // 将欲发送的队列数据腾出到 DMA 使用的固定线性数组中
            for (uint16_t i = 0; i < count; i++) {
                FIFO_Pop(&g_HMI.tx_fifo, &g_HMI.dma_tx_buf[i]);
            }
            
            g_HMI.tx_busy = 1; // 锁定发送状态
            HAL_UART_Transmit_DMA(g_HMI.huart, g_HMI.dma_tx_buf, count);
        } else {
            // TX FIFO 为空，回到空闲状态
            g_HMI.tx_busy = 0;
        }
    }
}

void HMI_UartRx_Callback_Wrapper(UART_HandleTypeDef *huart, uint16_t size)
{
    if (g_HMI.huart == huart) {
        // DMA 中断来了，将其固定缓存段里收到的数据复制到业务层的 RX 环形缓冲区中
        for (uint16_t i = 0; i < size; i++) {
            FIFO_Push(&g_HMI.rx_fifo, g_HMI.dma_rx_buf[i]);
        }
        
        // 立即开启下一次 DMA 接收，防止漏包
        HAL_UARTEx_ReceiveToIdle_DMA(g_HMI.huart, g_HMI.dma_rx_buf, DMA_BUF_SIZE);
    }
}

void HMI_UartError_Callback_Wrapper(UART_HandleTypeDef *huart)
{
    if (g_HMI.huart == huart) {
        // 如果断线或者产生溢出错误，需解挂并重启接收以自我恢复
        HAL_UARTEx_ReceiveToIdle_DMA(g_HMI.huart, g_HMI.dma_rx_buf, DMA_BUF_SIZE);
    }
}


