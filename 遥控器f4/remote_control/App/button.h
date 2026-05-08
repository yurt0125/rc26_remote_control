#ifndef __BUTTON_H
#define __BUTTON_H

#include "main.h"
#include "Datapool.h"
#include "tim.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include "Datapool.h"

#define BUTTON_COUNT 12

// 时间参数定义 (单位: ms)
#define BTN_DEBOUNCE_TIME     20    // 消抖时间
#define BTN_LONG_CLICK_TIME   1000  // 长按判定时间
#define BTN_DOUBLE_CLICK_TIME 250   // 双击间隔判定时间

// 按键事件枚举
typedef enum {
    BTN_EVENT_NONE = 0,     // 无事件
    BTN_EVENT_DOWN,         // 按下
    BTN_EVENT_UP,           // 抬起
    BTN_EVENT_SINGLE_CLICK, // 单击
    BTN_EVENT_DOUBLE_CLICK, // 双击
    BTN_EVENT_LONG_PRESS    // 长按
} ButtonEvent_t;

// 按键结构体
typedef struct {
    GPIO_TypeDef* port;     // GPIO端口
    uint16_t pin;           // GPIO引脚
    uint8_t active_level;   // 有效电平（按下时的电平）
    
    uint8_t state;          // 状态机当前状态
    uint32_t start_tick;    // 状态计时器
    ButtonEvent_t event;    // 当前触发的事件
} Button_t;

// 外部提供的按键数组
extern Button_t Buttons[BUTTON_COUNT];

void Button_Task_Init(void);
void Button_Task_Loop(void);
ButtonEvent_t Button_GetEvent(uint8_t btn_id);

#endif
