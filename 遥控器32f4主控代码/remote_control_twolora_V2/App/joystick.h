#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#include "main.h"
#include "Datapool.h"
#include "tim.h"
extern uint16_t joystick_AdcBuf[4]; // 摇杆 ADC DMA 缓冲区

void Joystick_Task_Init(ADC_HandleTypeDef* hadc_);

void Joystick_Task_Loop(void);

void JOY_AdcConvCplt_Callback_Wrapper(ADC_HandleTypeDef* hadc_);

#endif
