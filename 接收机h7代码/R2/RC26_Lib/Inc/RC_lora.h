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

    /**
     * @brief 获取接收到的命令帧数据（串口屏转发）
     * @param command 存放命令的变量
     * @param load1 存放负载1的变量（累计次数）
     * @param load2 存放负载2的变量（保留扩展）
     */
    void GetChosenCommandAndCnt(uint8_t& command, uint8_t& load1, uint8_t& load2) {
        Communication::GetRecvCommandFrameData(command, load1, load2);
    }

    // ---- 发送数据 setter 接口（外部调用修改发送参数）----

    void SetSendAxisData(uint16_t x, uint16_t y, uint16_t z) {
        send_x = x; send_y = y; send_z = z;
    }

    void SetSendStatus(uint8_t gripper_status, uint8_t suction_cup_status, uint8_t automatic_status) {
        send_gripper_status = gripper_status;
        send_suction_cup_status = suction_cup_status;
        send_automatic_status = automatic_status;
    }

    void SetSendMode(uint8_t mode) { send_mode = mode; }

    void SetSendCommand(uint8_t command1, uint8_t command2) {
        chosen_command = command1; chosen_command_cnt = command2;
    }

    void SetSendWantKFSData(uint8_t KFS_want_place1, uint8_t KFS_want_place2) {
        send_kfs_want_place1 = KFS_want_place1;
        send_kfs_want_place2 = KFS_want_place2;
        
    }
		
		void SetSendSpearData(uint8_t spear)
		{
        send_spear = spear;			
		}
		
		void SetSendKeepKFSData(uint8_t KFS_Keepplace)
		{
        send_kfs_keepplace = KFS_Keepplace;			
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

    // ---- 待发送的数据（由外部 setter 修改，定时器中断中发送）----
    uint16_t send_x, send_y, send_z;
    uint8_t send_gripper_status;
    uint8_t send_suction_cup_status;
    uint8_t send_automatic_status;
    uint8_t send_mode;
    uint8_t chosen_command, chosen_command_cnt, recv_command_load2;
    uint8_t send_kfs_want_place1, send_kfs_want_place2;
    uint8_t send_spear;
    uint8_t send_kfs_keepplace;
};

}

#endif // __cplusplus
