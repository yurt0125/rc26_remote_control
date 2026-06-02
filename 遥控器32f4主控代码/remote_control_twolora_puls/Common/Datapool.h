#ifndef __DATAPOOL_H
#define __DATAPOOL_H

#include "main.h"

extern uint16_t adc_key_val[2];
extern uint16_t joystick_Buf[4];
extern uint16_t tx_button_state;
extern uint32_t tx_stamp;
extern uint32_t rx_stamp;
extern uint16_t tx_cnt;
extern uint16_t rx_cnt;
extern uint32_t last_tx_stamp;
extern uint32_t last_rx_stamp;

extern volatile uint8_t hmi_state; // 启动界面:0  数据设置:1  数据显示:2  发送命令:3
extern volatile uint8_t timer_tick_count;

extern uint8_t KFS_load1;
extern uint8_t KFS_load2;
#endif
