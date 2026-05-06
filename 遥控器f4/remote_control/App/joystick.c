#include "joystick.h"

uint16_t joystick_AdcBuf[4]; // 摇杆 ADC DMA 缓冲区
static ADC_HandleTypeDef* hadc=0;

void Joystick_Task_Init(ADC_HandleTypeDef* hadc_)
{
    hadc = hadc_;

    // 这里可以添加一些摇杆相关的初始化代码

    HAL_ADC_Start_DMA(hadc,(uint32_t *)joystick_AdcBuf,sizeof(joystick_AdcBuf) / sizeof(joystick_AdcBuf[0]));
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_4,90);
}

void Joystick_Task_Loop(void)
{
    // 这里可以添加一些摇杆相关的循环处理代码
}

void JOY_AdcConvCplt_Callback_Wrapper(ADC_HandleTypeDef* hadc_)
{
    if(hadc_ == hadc) {
        // 这里可以添加一些摇杆相关的 ADC 转换完成后的处理代码
        joystick_Buf[0] = joystick_AdcBuf[0];
        joystick_Buf[1] = joystick_AdcBuf[1];
        joystick_Buf[2] = joystick_AdcBuf[2];
        joystick_Buf[3] = joystick_AdcBuf[3];
    }
}