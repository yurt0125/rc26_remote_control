#pragma once

#include "usart.h"

#define RING_BUF_SIZE 256
#define DMA_BUF_SIZE  64

#ifdef __cplusplus

namespace communication{
    typedef struct {
    uint8_t* buffer;
    volatile uint16_t head;
    volatile uint16_t tail;
    } comm_FIFO_t;

    /* 强制一字节对齐的数据帧 */
    #pragma pack(push, 1)

    // 接收帧：摇杆4个通道 (4 x 16位)
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

    // 发送帧：XYZ (3 x 16位)
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
        UART_HandleTypeDef* txhuart;
        UART_HandleTypeDef* rxhuart;  

        /* 专供 DMA 直接收发的线性缓冲区 */
        uint8_t* dma_rx_buf;
        uint8_t* dma_tx_buf;

        /* 用于业务逻辑和DMA之间解耦的环形缓冲区 */
        comm_FIFO_t rx_fifo;
        comm_FIFO_t tx_fifo;

        volatile uint8_t tx_busy; // 发送忙碌标志

        /* 解析出来/待发送的业务数据 */
        uint16_t send_xyz[3]; 
        uint16_t rec_joystick[4];
        uint16_t rec_send_key;
    } CommContext;

    class Communication {
    public:
        Communication(UART_HandleTypeDef *txhuart,UART_HandleTypeDef *rxhuart,
            uint8_t *tx_ring_buf,uint8_t *tx_dma_buf,uint8_t *rx_ring_buf,uint8_t *rx_dma_buf);
        ~Communication();

        void Comm_Task_Loop(void);

        void Comm_TxBufferToTxDMA(UART_HandleTypeDef *txhuart);

        void Comm_SendAnyDataToTxBuffer(const uint8_t* data, uint16_t len);

        void Comm_SetAxisData(uint16_t x, uint16_t y, uint16_t z);

        void Comm_RxDMAToRxBuffer(UART_HandleTypeDef *rxhuart, uint16_t size);

        // void Comm_UartError_Callback_Wrapper(UART_HandleTypeDef *errorhuart);

        // virtual void Communication_RX_DMA(void huart, uint8_t* data, uint16_t size)=0;

        virtual void Comm_TxUseTxDMA(void huart, uint8_t* data, uint16_t size)=0;
    private:
        void FIFO_Push(comm_FIFO_t* fifo, uint8_t data);

        int FIFO_Pop(comm_FIFO_t* fifo, uint8_t* data);

        uint16_t FIFO_Count(comm_FIFO_t* fifo);

        uint8_t crc8(const uint8_t *data, uint8_t len);
        
        CommContext g_Comm;
    protected:
    };
}

#endif

