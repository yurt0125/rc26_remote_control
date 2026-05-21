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
    {GPIOE, GPIO_PIN_1,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit15
    {GPIOE, GPIO_PIN_2,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit14
    {GPIOE, GPIO_PIN_3,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit13
    {GPIOC, GPIO_PIN_0,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_UP},//tx_button_statebit12
    {GPIOC, GPIO_PIN_1,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_UP},//tx_button_statebit11
    {GPIOC, GPIO_PIN_2,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_UP},//tx_button_statebit10
    {GPIOC, GPIO_PIN_3,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_UP},//tx_button_statebit9
    {GPIOE, GPIO_PIN_7,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit8
    {GPIOE, GPIO_PIN_8,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit7
    {GPIOE, GPIO_PIN_9,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit6
    {GPIOE, GPIO_PIN_10, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP}, //tx_button_statebit5
    {GPIOA, GPIO_PIN_8, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit4
    {GPIOA, GPIO_PIN_9, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit3
    {GPIOA, GPIO_PIN_10, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//tx_button_statebit2
    {GPIOA, GPIO_PIN_11, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP} //tx_button_statebit1
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
    // 这里可以添加一些按键相关的循环处理代码
    uint32_t current_tick = HAL_GetTick();
    
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
                
            case 1: // 消抖确认
                if (is_pressed) {
                    if ((current_tick - btn->start_tick) >= BTN_DEBOUNCE_TIME) {
                        btn->event = BTN_EVENT_DOWN;
                        btn->state = 2;

                        // PC0-3 / PE13-14：数据设置界面按键，消抖确认后发送单次
                        switch (i) {
                            case 2:  HMI_SendSettingFrame(2, 0, 0); break; // PE2
                            case 3:  HMI_SendSettingFrame(3, 0, 0); break; // PE3
                            case 4:  HMI_SendSettingFrame(1, 1, 0); break; // PC0
                            case 6:  HMI_SendSettingFrame(1, 2, 0); break; // PC1
                            case 7: HMI_SendSettingFrame(1, 3, 0); break; // PC2
                            case 5: HMI_SendSettingFrame(1, 4, 0); break; // PC3
                            default: break;
                        }
                    }
                } else {
                    btn->state = 0; // 抖动，重置状态
                }
                break;
                
            case 2: // 等待按键完全松开，避免重复发生事件
                if (!is_pressed) {
                    btn->event = BTN_EVENT_UP;
                    btn->state = 0; // 完全松开，回到空闲状态
                }
                break;
                
            default:
                btn->state = 0;
                break;
        }
        
        uint16_t mask = 0x0001 << ((15 - i));

        tx_button_state &= ~mask; // 清除旧状态
        if (Buttons[i].event == BTN_EVENT_DOWN) {
            tx_button_state |= (0x01 << (15 - i));
        } 
    }

}

