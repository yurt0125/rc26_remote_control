#ifndef __DATAPOOL_H
#define __DATAPOOL_H

#include "main.h"

extern volatile uint16_t adc_key_val[2];
extern volatile uint16_t joystick_Buf[4];
extern volatile uint16_t tx_button_state;
extern volatile uint32_t tx_stamp;
extern volatile uint32_t rx_stamp;
extern volatile uint16_t tx_cnt;
extern volatile uint16_t rx_cnt;
extern volatile uint32_t last_tx_stamp;
extern volatile uint32_t last_rx_stamp;
extern volatile uint8_t timer_tick_count;
extern volatile uint8_t hmi_state; // 启动界面:0  数据设置:1  数据显示:2  发送命令:3
#endif
