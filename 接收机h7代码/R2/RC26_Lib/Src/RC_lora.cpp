#include "RC_lora.h"
uint16_t joystick[4];
uint16_t key;
uint8_t command, KFS1, KFS2;
namespace communication {

Lora_communication::Lora_communication(UART_HandleTypeDef* tx_huart, UART_HandleTypeDef* rx_huart,
     GPIO_TypeDef* tx_aux_gpio_port, uint16_t tx_aux_gpio_pin,
      GPIO_TypeDef* rx_aux_gpio_port, uint16_t rx_aux_gpio_pin,
       tim::Tim* timer)
    : Communication(tx_huart, rx_huart, 
        tx_ring_buffer, tx_dma_buffer, rx_ring_buffer, rx_dma_buffer,
        tx_aux_gpio_port, tx_aux_gpio_pin, rx_aux_gpio_port, rx_aux_gpio_pin),
      UartRx(*rx_huart, rx_dma_buffer, DMA_BUF_SIZE, true, true), 
      ManagedTask("LoraTask", osPriorityNormal, 256, task::TASK_PERIOD, 5),
      GpioExti(tx_aux_gpio_pin), // 注册引脚所在的外部中断
      TimHandler(timer),      // 注册挂载到硬件1ms定时器类
      timer_tick_count(0)
{
    lora_tx_huart = tx_huart;
    lora_rx_huart = rx_huart;
    lora_aux_port = tx_aux_gpio_port;
    lora_aux_pin = tx_aux_gpio_pin;
}

Lora_communication::~Lora_communication() {
}

void Lora_communication::Init() {
    Uart_Rx_Start();
}

void Lora_communication::Comm_TxUseTxDMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size) {
    if (huart == lora_tx_huart) {
        HAL_UART_Transmit_DMA(huart, data, size);
    }
}

void Lora_communication::Uart_Rx_It_Process(uint8_t* buf_, uint16_t len_) {
		volatile int test = 1;
    // 串口收到数据，压入业务侧接收环形缓冲区
    Comm_RxDMAToRxBuffer(lora_rx_huart, len_);
}

void Lora_communication::Task_Process() {
    // 循环解析收到的数据
    if (Comm_Task_Loop()) {
//        uint16_t joystick[4];
//        uint16_t key;
        GetRecvData(joystick, key);
//        static uint8_t command, KFS1, KFS2;
        GetSettingData(command, KFS1, KFS2);
    }

}

void Lora_communication::Tim_It_Process() {
    // 这个回调依附于底层的硬件中断（比如你传进来的 tim7_1khz 1ms产生一次中断）
    timer_tick_count++;
    if (timer_tick_count >= 1) { // 计数达到 2ms 
        timer_tick_count = 0;
        Comm_SendAxisDataToTxBuffer(1, 2, 5 ,1,1,1,0xBB,0xCC,0xDD);
    }
}

void Lora_communication::EXTI_Prosess() {
    // 当该引脚触发中断（如上升沿，具体通过CubeMX或HAL初始化配置），就会进入此函数
    // 调用基类的发送缓冲区转DMA发送接口
    Comm_TxBufferToTxDMA(lora_tx_huart);
}

}
