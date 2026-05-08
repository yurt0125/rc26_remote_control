#include "button.h"

// 按键数组实例，包含 PE7-14（机器人控制） 和 PC0-3（十字按键）
Button_t Buttons[BUTTON_COUNT] = {
    //端口，引脚，有效电平，状态，计时器，事件 //对应数据池位数
    {GPIOE, GPIO_PIN_7,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//button_Bufbit16
    {GPIOE, GPIO_PIN_8,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//button_Bufbit15
    {GPIOE, GPIO_PIN_9,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//button_Bufbit14
    {GPIOE, GPIO_PIN_10, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//button_Bufbit13
    {GPIOE, GPIO_PIN_11, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//button_Bufbit12
    {GPIOE, GPIO_PIN_12, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//button_Bufbit11
    {GPIOE, GPIO_PIN_13, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//button_Bufbit10
    {GPIOE, GPIO_PIN_14, GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//button_Bufbit9
    {GPIOC, GPIO_PIN_0,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//button_Bufbit8
    {GPIOC, GPIO_PIN_1,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//button_Bufbit7
    {GPIOC, GPIO_PIN_2,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP},//button_Bufbit6
    {GPIOC, GPIO_PIN_3,  GPIO_PIN_SET, 0, 0, BTN_EVENT_UP} //button_Buf[1]5
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

        button_Buf &= ~mask; // 清除旧状态
        if (Buttons[i].event == BTN_EVENT_DOWN) {
            button_Buf |= (0x01 << (15 - i));
        } 
    }

}

