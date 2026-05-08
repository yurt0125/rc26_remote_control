#include "button.h"

// 按键数组实例，包含 PE7-14（机器人控制） 和 PC0-3（十字按键）
Button_t Buttons[BUTTON_COUNT] = {
    {GPIOE, GPIO_PIN_7,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE},//button_Buf[0]bit16-15
    {GPIOE, GPIO_PIN_8,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE},//button_Buf[0]bit14-13
    {GPIOE, GPIO_PIN_9,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE},//button_Buf[0]bit12-11
    {GPIOE, GPIO_PIN_10, GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE},//button_Buf[0]bit10-9
    {GPIOE, GPIO_PIN_11, GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE},//button_Buf[0]bit8-7
    {GPIOE, GPIO_PIN_12, GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE},//button_Buf[0]bit6-5
    {GPIOE, GPIO_PIN_13, GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE},//button_Buf[0]bit4-3
    {GPIOE, GPIO_PIN_14, GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE},//button_Buf[0]bit2-1
    {GPIOC, GPIO_PIN_0,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE},//button_Buf[1]bit16-15
    {GPIOC, GPIO_PIN_1,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE},//button_Buf[1]bit14-13
    {GPIOC, GPIO_PIN_2,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE},//button_Buf[1]bit12-11
    {GPIOC, GPIO_PIN_3,  GPIO_PIN_RESET, 0, 0, BTN_EVENT_NONE} //button_Buf[1]bit10-9
};

void Button_Task_Init(void)
{
    // 这里可以添加一些按键相关的初始化代码
    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    for(int i = 0; i < BUTTON_COUNT; i++) {
        Buttons[i].state = 0;
        Buttons[i].start_tick = 0;
        Buttons[i].event = BTN_EVENT_NONE;
    }
}

static bool Is_Button_Pressed(Button_t* btn) {
    return (HAL_GPIO_ReadPin(btn->port, btn->pin) == btn->active_level);
}

void Button_Task_Loop(void)
{
    // 这里可以添加一些按键相关的循环处理代码
    uint32_t current_tick = HAL_GetTick();
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        Button_t *btn = &Buttons[i];
        bool is_pressed = Is_Button_Pressed(btn);
        
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
                
            case 2: // 等待松开，同时检测长按
                if (!is_pressed) {
                    btn->event = BTN_EVENT_UP;
                    btn->start_tick = current_tick;
                    btn->state = 3; // 进入等待双击阶段
                } else {
                    if ((current_tick - btn->start_tick) >= BTN_LONG_CLICK_TIME) {
                        btn->event = BTN_EVENT_LONG_PRESS;
                        btn->state = 5; // 长按触发后，直接进入等待完全松开状态
                    }
                }
                break;
                
            case 3: // 等待第二次点击或确认单击
                if (is_pressed) {
                    btn->start_tick = current_tick;
                    btn->state = 4; // 进入二次消抖
                } else {
                    if ((current_tick - btn->start_tick) >= BTN_DOUBLE_CLICK_TIME) {
                        btn->event = BTN_EVENT_SINGLE_CLICK;
                        btn->state = 0; // 超时未第二次点击，确认为单击，返回空闲
                    }
                }
                break;
                
            case 4: // 第二次按下的消抖确认
                if (is_pressed) {
                    if ((current_tick - btn->start_tick) >= BTN_DEBOUNCE_TIME) {
                        btn->event = BTN_EVENT_DOUBLE_CLICK;
                        btn->state = 5; // 确认为双击，进入等待完全松开状态
                    }
                } else {
                    btn->state = 3; // 异常抖动，返回等待状态
                }
                break;
                
            case 5: // 等待按键完全松开，避免重复发生事件
                if (!is_pressed) {
                    btn->event = BTN_EVENT_UP;
                    btn->state = 0; // 完全松开，回到空闲状态
                }
                break;
                
            default:
                btn->state = 0;
                break;
        }
        if(i < 8) {
            ButtonEvent_t this_button_event = Button_GetEvent(i);
            uint16_t mask=0x0003 << ((7 - i) * 2); // 每个按钮占用2位
            switch (this_button_event) {
                case BTN_EVENT_NONE:
                    button_Buf[0] &= ~mask; // 清除对应位
                    break;
                case BTN_EVENT_SINGLE_CLICK:
                    button_Buf[0] &= ~mask; // 清除对应位
                    button_Buf[0] |= (0x01 << ((7 - i) * 2)); // 设置单击标志
                    break;
                case BTN_EVENT_DOUBLE_CLICK:
                    button_Buf[0] &= ~mask; // 清除对应位
                    button_Buf[0] |= (0x02 << ((7 - i) * 2)); // 设置双击标志
                    break;
                case BTN_EVENT_LONG_PRESS:
                    button_Buf[0] &= ~mask; // 清除对应位
                    button_Buf[0] |= (0x03 << ((7 - i) * 2)); // 设置长按标志
                    break;
                default:
                    break;
            }
        } else {
            ButtonEvent_t this_button_event = Button_GetEvent(i);
            uint16_t mask=0x0003 << ((7 - i + 8) * 2); // 每个按钮占用2位
            switch (this_button_event) {
                case BTN_EVENT_NONE:
                    button_Buf[1] &= ~mask; // 清除对应位
                    break;
                case BTN_EVENT_SINGLE_CLICK:
                    button_Buf[1] &= ~mask; // 清除对应位
                    button_Buf[1] |= (0x01 << ((7 - i + 8) * 2)); // 设置单击标志
                    break;
                case BTN_EVENT_DOUBLE_CLICK:
                    button_Buf[1] &= ~mask; // 清除对应位
                    button_Buf[1] |= (0x02 << ((7 - i + 8) * 2)); // 设置双击标志
                    break;
                case BTN_EVENT_LONG_PRESS:
                    button_Buf[1] &= ~mask; // 清除对应位
                    button_Buf[1] |= (0x03 << ((7 - i + 8) * 2)); // 设置长按标志
                    break;
                default:
                    break;
            }
        }
    }

}

ButtonEvent_t Button_GetEvent(uint8_t btn_id) {
    if (btn_id >= BUTTON_COUNT) return BTN_EVENT_NONE;
    
    ButtonEvent_t current_event = Buttons[btn_id].event;
    // 如果事件已经生成，读取后将其重置
    if(current_event == BTN_EVENT_SINGLE_CLICK || 
       current_event == BTN_EVENT_DOUBLE_CLICK || 
       current_event == BTN_EVENT_LONG_PRESS) {
        Buttons[btn_id].event = BTN_EVENT_NONE;
    }
    
    return current_event;
}
