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
#endif
