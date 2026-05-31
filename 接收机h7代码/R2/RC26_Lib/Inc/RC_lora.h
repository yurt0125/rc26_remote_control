#pragma once

#include "RC_communication.h"
#include "RC_serial.h"
#include "RC_task.h"
#include "RC_gpio_exti.h" // 引入 EXTI 支持
#include "RC_tim.h"       // 引入硬件定时器支持

#ifdef __cplusplus

namespace communication {

class Lora_communication : public Communication, public serial::UartRx, public task::ManagedTask, public gpio::GpioExti, public tim::TimHandler {
public:
    Lora_communication(UART_HandleTypeDef* tx_huart, UART_HandleTypeDef* rx_huart,
         GPIO_TypeDef* tx_aux_gpio_port, uint16_t tx_aux_gpio_pin,
          GPIO_TypeDef* rx_aux_gpio_port, uint16_t rx_aux_gpio_pin,
           tim::Tim* timer);
    ~Lora_communication();

    void Init();

    /**
     * @brief 查询指定索引的按键是否被按下
     * @param key_index 按键索引 (0~15)，对应 rec_send_key 的 bit0~bit15
     * @return true 表示该按键按下，false 表示未按下
     */
    bool IsKeyPressed(uint8_t key_index) {
        return Communication::GetRecvKeyData(key_index);
    }

    /**
     * @brief 获取全部16个按键的原始位图
     * @return uint16_t 每一位代表一个按键状态
     */
    uint16_t GetKeyStatus(void) {
        return Communication::GetRecvAllKeyData();
    }

    /**
     * @brief 获取接收到的 page 值
     * @return uint8_t 当前帧的 page 字段
     */
    uint8_t GetPage(void) {
        return Communication::GetPage();
    }

protected:
    virtual void Comm_TxUseTxDMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size) override;
    virtual void Uart_Rx_It_Process(uint8_t* buf_, uint16_t len_) override;
    virtual void Task_Process() override;
    
    // GPIO 外部中断回调函数
    virtual void EXTI_Prosess() override;
    
    // 硬件定时器 1ms 周期性回调函数
    virtual void Tim_It_Process() override;

private:
    UART_HandleTypeDef* lora_tx_huart;
    UART_HandleTypeDef* lora_rx_huart;
    GPIO_TypeDef* lora_aux_port;
    uint16_t lora_aux_pin;
    uint32_t timer_tick_count; // 用于1ms定时器计数
    
    // 底层数据缓冲区
    uint8_t tx_ring_buffer[RING_BUF_SIZE];
    uint8_t rx_ring_buffer[RING_BUF_SIZE];

    // 【修改】强制向 32 字节对齐
    alignas(32) uint8_t tx_dma_buffer[DMA_BUF_SIZE];
    alignas(32) uint8_t rx_dma_buffer[DMA_BUF_SIZE];
};

}

#endif // __cplusplus
