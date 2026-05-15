#include "Datapool.h"

uint16_t joystick_Buf[4] = {0};
uint16_t button_Buf = 0;
uint16_t key_Buf[2]={0};
uint32_t tx_stamp=0;
uint32_t rx_stamp=0;
uint16_t tx_cnt=0;
uint16_t rx_cnt=0;

uint32_t last_tx_stamp=0;
uint32_t last_rx_stamp=0;