#ifndef __KEY_H
#define __KEY_H

#include "main.h"
#include "Datapool.h"
#include "tim.h"

extern uint16_t key_AdcBuf[2]; // °´¼ü ADC DMA »º³åÇø

void Key_Task_Init(ADC_HandleTypeDef* hadc_);

void Key_Task_Loop(void);

void KEY_AdcConvCplt_Callback_Wrapper(ADC_HandleTypeDef* hadc_);

#endif
