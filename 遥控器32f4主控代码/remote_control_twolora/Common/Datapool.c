#include "Datapool.h"

uint16_t joystick_Buf[4] = {0};
uint16_t tx_button_state = 0;
uint16_t adc_key_val[2]={0};
uint32_t tx_stamp=0;
uint32_t rx_stamp=0;
uint16_t tx_cnt=0;
uint16_t rx_cnt=0;

uint32_t last_tx_stamp=0;
uint32_t last_rx_stamp=0;