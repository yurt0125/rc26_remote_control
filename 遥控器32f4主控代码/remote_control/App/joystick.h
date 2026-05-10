#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#include "main.h"
#include "Datapool.h"
#include "tim.h"
extern uint16_t joystick_AdcBuf[4]; // Ò¡¸Ë ADC DMA »º³åÇø

void Joystick_Task_Init(ADC_HandleTypeDef* hadc_);

void Joystick_Task_Loop(void);

void JOY_AdcConvCplt_Callback_Wrapper(ADC_HandleTypeDef* hadc_);

#endif
