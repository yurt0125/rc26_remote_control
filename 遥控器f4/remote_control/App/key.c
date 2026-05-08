#include "key.h"

uint16_t key_AdcBuf[2]; // 按键 ADC DMA 缓冲区
static ADC_HandleTypeDef* hadc=0;

void Key_Task_Init(ADC_HandleTypeDef* hadc_)
{
    hadc = hadc_;

    // 这里可以添加一些按键相关的初始化代码

    HAL_ADC_Start_DMA(hadc,(uint32_t *)key_AdcBuf,sizeof(key_AdcBuf) / sizeof(key_AdcBuf[0]));
//		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
//    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_4,90);
}

void Key_Task_Loop(void)
{
    // 这里可以添加一些按键相关的循环处理代码
}

void KEY_AdcConvCplt_Callback_Wrapper(ADC_HandleTypeDef* hadc_)
{
    if(hadc_ == hadc) {
        // 这里可以添加一些按键相关的 ADC 转换完成后的处理代码
        key_Buf[0] = key_AdcBuf[0];
        key_Buf[1] = key_AdcBuf[1];
			
				if(key_Buf[0]>3686)
				{
					button_Buf &= 0xFFF3; // 清除旧状态
					button_Buf |= 0x8;		//赋值bit3-2：10
				}else if(key_Buf[0]>409&&key_Buf[0]<=3686)
				{
					button_Buf &= 0xFFF3; // 清除旧状态
					button_Buf |= 0x4;		//赋值bit3-2：01
				}else
				{
					button_Buf &= 0xFFF3; // 清除旧状态
				}
				
				if(key_Buf[1]>3686)
				{
					button_Buf &= 0xFFFC; // 清除旧状态
					button_Buf |= 0x2;		//赋值bit1-0：10
				}else if(key_Buf[1]>409&&key_Buf[1]<=3686)
				{
					button_Buf &= 0xFFFC; // 清除旧状态
					button_Buf |= 0x1;		//赋值bit1-0：01
				}else
				{
					button_Buf &= 0xFFFC; // 清除旧状态
				}
    }
}