#include "RC_lora.h"
uint16_t joystick[4];
uint16_t key;
uint8_t KFS1_1, KFS1_2, KFS1_3;
uint8_t KFS2_1, KFS2_2, KFS2_3, KFS2_4;
uint8_t KFSf_1;
uint8_t color;
uint8_t page;
uint16_t key_pressed_count;   // 当前帧中被按下的按键个数 (0~16)
uint16_t key_down_count;     // 累计检测到的按键按下次数（上升沿计数）
uint16_t key_last_status;    // 上一帧的按键状态，用于边沿检测
uint16_t joystick1;
uint16_t joystick2;
uint16_t joystick3;
uint16_t joystick4;			
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
        GetRecvJoystickData(joystick);
        key = GetRecvAllKeyData();
				page = GetPage();
			
				color = GetColor();
			
				joystick1=joystick[0];
				joystick2=joystick[1];
				joystick3=joystick[2];
				joystick4=joystick[3];			

        KFS1_1 = GetRecvFKFS1Data(1);
        KFS1_2 = GetRecvFKFS1Data(2);
        KFS1_3 = GetRecvFKFS1Data(3);

        KFS2_1 = GetRecvFKFS2Data(1);
        KFS2_2 = GetRecvFKFS2Data(2);
        KFS2_3 = GetRecvFKFS2Data(3);
        KFS2_4 = GetRecvFKFS2Data(4);

        KFSf_1 = GetRecvFKFSfData(1);

        // 查询16个按键状态并统计按下个数（发送端已完成去抖）
        uint16_t key_status = GetKeyStatus();
        // key_pressed_count = 0;
        for (uint8_t i = 0; i < 16; i++) {
            if (key_status & (1 << i)) {
                key_pressed_count++;
            }
        }
        // 上升沿检测：只有从0变1时才累加按下次数（验证消抖用）
        uint16_t rising_edges = key_status & (~key_last_status);
        for (uint8_t i = 0; i < 16; i++) {
            if (rising_edges & (1 << i)) {
                key_down_count++;
            }
        }
        key_last_status = key_status;    }

}

void Lora_communication::Tim_It_Process() {
    // 这个回调依附于底层的硬件中断（比如你传进来的 tim7_1khz 1ms产生一次中断）
    timer_tick_count++;
    if (timer_tick_count >= 2) { // 计数达到 1ms 
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