#include "Lora_communication.h"

namespace communication {

Lora_communication::Lora_communication(UART_HandleTypeDef* tx_huart, UART_HandleTypeDef* rx_huart, GPIO_TypeDef* aux_gpio_port, uint16_t aux_gpio_pin, tim::Tim* timer)
    : Communication(tx_huart, rx_huart, tx_ring_buffer, tx_dma_buffer, rx_ring_buffer, rx_dma_buffer),
      UartRx(*rx_huart, rx_dma_buffer, DMA_BUF_SIZE, true, true), 
      ManagedTask("LoraTask", osPriorityNormal, 256, task::TASK_PERIOD, 5),
      GpioExti(aux_gpio_pin), // 注册引脚所在的外部中断
      TimHandler(timer),      // 注册挂载到硬件1ms定时器类
      lora_tx_huart(tx_huart),
      lora_rx_huart(rx_huart),
      lora_aux_port(aux_gpio_port),
      lora_aux_pin(aux_gpio_pin),
      timer_tick_count(0)
{
    Uart_Rx_Start();
}

Lora_communication::~Lora_communication() {
}

void Lora_communication::Comm_TxUseTxDMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size) {
    if (huart == lora_tx_huart) {
        HAL_UART_Transmit_DMA(huart, data, size);
    }
}

void Lora_communication::Uart_Rx_It_Process(uint8_t* buf_, uint16_t len_) {
    // 串口收到数据，压入业务侧接收环形缓冲区
    Comm_RxDMAToRxBuffer(lora_rx_huart, len_);
}

void Lora_communication::Task_Process() {
    // 循环解析收到的数据
    if (Comm_Task_Loop()) {
        // 解析到了有效的一帧数据
        // 数据已经被更新到 Communication 的相关内部变量中
        // 在这里获取摇杆/按键数据，对接具体的业务逻辑
    }

    // 在这里实现周期的发送任务逻辑
    // 举例：
    // Comm_SendAxisDataToTxBuffer(100, 200, 300);
}

void Lora_communication::Tim_It_Process() {
    // 这个回调依附于底层的硬件中断（比如你传进来的 tim7_1khz 1ms产生一次中断）
    timer_tick_count++;
    if (timer_tick_count >= 10) { // 计数达到 10ms 
        timer_tick_count = 0;
        if (HAL_GPIO_ReadPin(lora_aux_port, lora_aux_pin) == GPIO_PIN_SET) {
            Comm_TxBufferToTxDMA(lora_tx_huart);
        }
    }
}

void Lora_communication::EXTI_Prosess() {
    // 当该引脚触发中断（如上升沿，具体通过CubeMX或HAL初始化配置），就会进入此函数
    // 调用基类的发送缓冲区转DMA发送接口
    Comm_TxBufferToTxDMA(lora_tx_huart);
}

}
