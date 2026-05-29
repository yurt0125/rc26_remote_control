#include "gpio_button.h"
#include "tjc_huart_hmi.h"

//key_1-6 : PE0-PE5 //使用key1-4
//cross_key_1-4 : PC0-PC3 //使用cross_key1-4
//Dswtich1-8 : PE7-PE14 //使用Dswitch1-4
//Tswitch1-2 : PA8-PA11 //使用Tswitch1-2

// 按键数组实例，包含 PE7-14（机器人控制） 和 PC0-3（十字按键）
Button_t Buttons[BUTTON_COUNT] = {
    //端口，引脚，有效电平，状态，计时器，事件 //对应数据池位数
    {GPIOE, GPIO_PIN_0,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit16
    {GPIOE, GPIO_PIN_5,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit15
    {GPIOE, GPIO_PIN_2,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit14
    {GPIOE, GPIO_PIN_4,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit13
    {GPIOC, GPIO_PIN_0,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_UP},//tx_button_statebit12
    {GPIOC, GPIO_PIN_1,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_UP},//tx_button_statebit11
    {GPIOC, GPIO_PIN_2,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_UP},//tx_button_statebit10
    {GPIOC, GPIO_PIN_3,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_UP},//tx_button_statebit9
    {GPIOE, GPIO_PIN_7,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit8
    {GPIOE, GPIO_PIN_9,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit7
    {GPIOE, GPIO_PIN_12,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit6
    {GPIOE, GPIO_PIN_14, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP}, //tx_button_statebit5
    {GPIOA, GPIO_PIN_8, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit4
    {GPIOA, GPIO_PIN_9, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit3
    {GPIOB, GPIO_PIN_6, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit2
    {GPIOB, GPIO_PIN_7, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP} //tx_button_statebit1
};

void Button_Task_Init(void)
{
    // 这里可以添加一些按键相关的初始化代码
    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    for(int i = 0; i < BUTTON_COUNT; i++) {
        Buttons[i].state = 0;
        Buttons[i].start_tick = 0;
        Buttons[i].event = BTN_EVENT_UP;
    }
}
    
static uint8_t Is_Button_Pressed(Button_t* btn) {
    return (HAL_GPIO_ReadPin(btn->port, btn->pin) == btn->active_level);
}

void Button_Task_Loop(void)
{
    // 状态说明：
    // 0:空闲等待按下  1:按下消抖中(20ms)  2:确认按住  3:释放消抖中(20ms)
    uint32_t current_tick = HAL_GetTick();
    uint16_t new_button_state = 0;  // 先构建局部变量，最后一次性原子写入，避免 ISR 读到中间态
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        Button_t *btn = &Buttons[i];
        uint8_t is_pressed = Is_Button_Pressed(btn);
        
        switch (btn->state) {
            case 0: // 空闲状态，等待按键按下
                if (is_pressed) {
                    btn->start_tick = current_tick;
                    btn->state = 1;
                }
                break;
                
            case 1: // 按下消抖确认
                if (is_pressed) {
                    if ((current_tick - btn->start_tick) >= BTN_DEBOUNCE_TIME) {
                        btn->event = BTN_EVENT_DOWN;
                        btn->state = 2; // 确认按下，进入按住状态

                        // PC0-3 / PE13-14：数据设置界面按键，消抖确认后发送单次
                        switch (i) {
                            case 0:  HMI_SendSettingFrame(2, 1, 0); break; // 设置R1KFS
                            case 1:  HMI_SendSettingFrame(6, 0, 0); break; // 重置
													  case 2:  HMI_SendSettingFrame(3, 0, 0); break; // 撤销
														case 3:  HMI_SendSettingFrame(5, 0, 0); break; // 发送
                            case 4:  HMI_SendSettingFrame(1, 1, 0); break; // PC0
                            case 5:  HMI_SendSettingFrame(1, 3, 0); break; // PC1
                            case 6:  HMI_SendSettingFrame(1, 4, 0); break; // PC2
                            case 7:  HMI_SendSettingFrame(1, 2, 0); break; // PC3
                            default: break;
                        }
                    }
                } else {
                    btn->state = 0; // 按下抖动，重置
                }
                break;
                
            case 2: // 确认按住状态，等待释放
                if (!is_pressed) {
                    btn->start_tick = current_tick;
                    btn->state = 3; // 进入释放消抖
                }
                break;

            case 3: // 释放消抖确认
                if (!is_pressed) {
                    if ((current_tick - btn->start_tick) >= BTN_DEBOUNCE_TIME) {
                        btn->event = BTN_EVENT_UP;
                        btn->state = 0; // 确认释放，回到空闲
                    }
                } else {
                    // 释放期间发生抖动反弹，回到按住状态
                    btn->state = 2;
                }
                break;
                
            default:
                btn->state = 0;
                break;
        }
        
        if (Buttons[i].event == BTN_EVENT_DOWN) {
            new_button_state |= (0x01 << (15 - i));
        }
    }

    tx_button_state = new_button_state; // 一次性原子写入，ISR 不会读到中间态
}

