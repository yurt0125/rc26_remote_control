#include "comm_wrapper.h"
#include "communication.h"

// 静态分配所需的环形缓冲区和 DMA 缓冲区
static uint8_t app_tx_ring_buf[RING_BUF_SIZE];
static uint8_t app_tx_dma_buf[DMA_BUF_SIZE];
static uint8_t app_rx_ring_buf[RING_BUF_SIZE];
static uint8_t app_rx_dma_buf[DMA_BUF_SIZE];

// 创建派生类，实现基类中的纯虚函数
class CommunicationImpl : public communication::Communication {
public:
    CommunicationImpl(UART_HandleTypeDef *txhuart, UART_HandleTypeDef *rxhuart,
                      uint8_t *tx_ring, uint8_t *tx_dma, uint8_t *rx_ring, uint8_t *rx_dma)
        : Communication(txhuart, rxhuart, tx_ring, tx_dma, rx_ring, rx_dma) {}

    // 重点：实现抽象类中虚约束（物理底层发送）
    virtual void Comm_TxUseTxDMA(UART_HandleTypeDef * huart, uint8_t* data, uint16_t size) {
        // 调用 HAL 库触发 DMA 发送
        HAL_UART_Transmit_DMA(huart, data, size); 
    }
};

// 声明全局单例指针
static CommunicationImpl* g_comm = NULL;

// 提供给 C 语言环境 (如 main.c) 调用的普通 C 函数
extern "C" {
    void CommWrapper_Init(UART_HandleTypeDef *txhuart, UART_HandleTypeDef *rxhuart) {
        if (!g_comm) {
            // 利用局部静态变量初始化实例（避免外部重复初始化造成内存或状态异常）
            static CommunicationImpl instance(txhuart, rxhuart, 
                                              app_tx_ring_buf, app_tx_dma_buf, 
                                              app_rx_ring_buf, app_rx_dma_buf);
            g_comm = &instance;

            // 开启首次 DMA 空闲中断接收
            HAL_UARTEx_ReceiveToIdle_DMA(rxhuart, app_rx_dma_buf, DMA_BUF_SIZE);
        }
    }

    bool CommWrapper_Task_Loop(void) {
        if (g_comm) return g_comm->Comm_Task_Loop();
        return false;
    }

    void CommWrapper_RxDMAToRxBuffer(UART_HandleTypeDef *rxhuart, uint16_t size) {
        if (g_comm) g_comm->Comm_RxDMAToRxBuffer(rxhuart, size);
    }

    void CommWrapper_TxBufferToTxDMA(UART_HandleTypeDef *txhuart) {
        if (g_comm) g_comm->Comm_TxBufferToTxDMA(txhuart);
    }

    void CommWrapper_SendAxisData(uint16_t x, uint16_t y, uint16_t z) {
        if (g_comm) g_comm->Comm_SendAxisDataToTxBuffer(x, y, z);
    }

    void CommWrapper_GetRecvData(uint16_t* joystick, uint16_t* key) {
        if (g_comm) g_comm->GetRecvData(joystick, key);
    }
}
