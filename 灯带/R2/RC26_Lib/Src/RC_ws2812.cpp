#include "RC_ws2812.h"
#include "spi.h"
#include "cmsis_os.h"

/*
@brief 用于驱动led灯条, 基于 ManagedTask 的 C++ 封装
@param 使用时需要将spi1配置为5mhz，使用主机发送即可
*/

namespace ws2812
{

Ws2812B* g_ws2812b_instance = nullptr;

// DMA 缓冲区
uint8_t Ws2812B::spi_buf[WS2812B_AMOUNT * 24 + RESET_BYTES];

Ws2812B::Ws2812B(SPI_HandleTypeDef* hspi_)
    : task::ManagedTask("Ws2812b", 20, 128, task::TASK_DELAY, 1)
    , hspi(hspi_)
    , signal_queue(nullptr)
    , flash_state(FlashState::STEADY)
    , start_time(0)
{
    // 初始化颜色缓存
    for (uint8_t i = 0; i < WS2812B_AMOUNT; i++)
    {
        color_cache[i].R = 0x00;
        color_cache[i].G = 0x00;
        color_cache[i].B = 0x00;
    }

    // 初始化 SPI 缓冲
    for (uint16_t i = 0; i < sizeof(spi_buf); i++)
    {
        spi_buf[i] = 0;
    }

    // 队列延迟到 Task_Process 首次运行时创建（此时 RTOS 已启动）
    // xQueueCreate 必须在 osKernelStart 之后才能调用

    // 设置全局实例指针，供兼容接口使用
    g_ws2812b_instance = this;
}

// 将单颗灯珠的 RGB 值逐位编码成 SPI 波形数据
void Ws2812B::EncodeLed(uint16_t led_index, uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t* pR = &spi_buf[led_index * 24 + 8];
    uint8_t* pG = &spi_buf[led_index * 24];
    uint8_t* pB = &spi_buf[led_index * 24 + 16];

    for (uint8_t i = 0; i < 8; i++)
    {
        if (g & 0x80)
            *pG = CODE_1;
        else
            *pG = CODE_0;

        if (r & 0x80)
            *pR = CODE_1;
        else
            *pR = CODE_0;

        if (b & 0x80)
            *pB = CODE_1;
        else
            *pB = CODE_0;

        r <<= 1;
        g <<= 1;
        b <<= 1;
        pR++;
        pG++;
        pB++;
    }
}

// 状态机任务处理
void Ws2812B::Task_Process()
{
    // 延迟初始化：首次运行时创建队列（此时 RTOS 已启动）
    if (signal_queue == nullptr)
    {
        signal_queue = xQueueCreate(10, sizeof(Command));
        if (signal_queue == nullptr)
        {
            Error_Handler();
        }
        // 初始化完成后刷一次初始颜色
        ApplyColor(current_color);
        SpiSend();
        return;
    }

    Command command;
    uint32_t current_time = timer::Timer::Get_TimeStamp();

    // 非阻塞接收队列消息
    if (xQueueReceive(signal_queue, &command, 0) == pdPASS)
    {
        switch (command.type)
        {
        case CommandType::SET_COLOR:
            current_color = command.color1;
            flash_state = FlashState::STEADY;
            ApplyColor(current_color);
            break;

        case CommandType::FLASH_ONCE:
            flash_color1 = command.color1;
            flash_color2 = command.color2;
            flash_state = FlashState::ONCE_COLOR1_FIRST;
            ApplyColor(flash_color1);
            start_time = current_time;
            break;

        case CommandType::FLASH_CONTINUOUS:
            flash_color1 = command.color1;
            flash_color2 = command.color2;
            flash_state = FlashState::CONTINUOUS_COLOR1;
            ApplyColor(flash_color1);
            start_time = current_time;
            break;
        }
    }

    // 定时状态转移：单次双闪或持续双色闪烁
    switch (flash_state)
    {
    case FlashState::ONCE_COLOR1_FIRST:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flash_state = FlashState::ONCE_COLOR2_FIRST;
            ApplyColor(flash_color2);
            start_time = current_time;
        }
        break;

    case FlashState::ONCE_COLOR2_FIRST:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flash_state = FlashState::ONCE_COLOR1_SECOND;
            ApplyColor(flash_color1);
            start_time = current_time;
        }
        break;

    case FlashState::ONCE_COLOR1_SECOND:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flash_state = FlashState::ONCE_COLOR2_SECOND;
            ApplyColor(flash_color2);
            start_time = current_time;
        }
        break;

    case FlashState::ONCE_COLOR2_SECOND:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flash_state = FlashState::STEADY;
            ApplyColor(current_color);
        }
        break;

    case FlashState::CONTINUOUS_COLOR1:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flash_state = FlashState::CONTINUOUS_COLOR2;
            ApplyColor(flash_color2);
            start_time = current_time;
        }
        break;

    case FlashState::CONTINUOUS_COLOR2:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flash_state = FlashState::CONTINUOUS_COLOR1;
            ApplyColor(flash_color1);
            start_time = current_time;
        }
        break;

    default:
        break;
    }

    SpiSend();
}

void Ws2812B::SpiSend()
{
    // 上一次 DMA 还没完成，跳过本轮
    if (HAL_SPI_GetState(hspi) != HAL_SPI_STATE_READY)
        return;

    // 将颜色缓存编码为 SPI 数据
    for (uint8_t iLED = 0; iLED < WS2812B_AMOUNT; iLED++)
    {
        EncodeLed(iLED, color_cache[iLED].R, color_cache[iLED].G, color_cache[iLED].B);
    }

    // 在 LED 数据后填充复位字节
    uint8_t* pReset = &spi_buf[WS2812B_AMOUNT * 24];
    for (uint8_t i = 0; i < RESET_BYTES; i++)
    {
        pReset[i] = 0x00;
    }

    // 刷新 DCache，确保 DMA 读到最新数据（STM32H7 必需）
    SCB_CleanDCache_by_Addr((uint32_t*)spi_buf, WS2812B_AMOUNT * 24 + RESET_BYTES);

    // DMA 一次性发送 LED 数据 + 复位信号
    HAL_SPI_Transmit_DMA(hspi, spi_buf, WS2812B_AMOUNT * 24 + RESET_BYTES);
}

// 设置全部灯为指定颜色
void Ws2812B::SetAllColor(uint8_t r, uint8_t g, uint8_t b)
{
    for (uint8_t i = 0; i < WS2812B_AMOUNT; i++)
    {
        color_cache[i].R = r;
        color_cache[i].G = g;
        color_cache[i].B = b;
    }
}

void Ws2812B::ApplyColor(Color color)
{
    switch (color)
    {
    case Color::RED:    SetAllColor((uint8_t)(0.972 * WS2812B_BRIGHTNESS), (uint8_t)(0.709 * WS2812B_BRIGHTNESS), 0x00); break;
    case Color::GREEN:  SetAllColor(0x00, WS2812B_BRIGHTNESS, 0x00); break;
    case Color::BLUE:   SetAllColor(0x00, 0x00, WS2812B_BRIGHTNESS); break;
    case Color::WHITE:  SetAllColor(WS2812B_BRIGHTNESS, WS2812B_BRIGHTNESS, WS2812B_BRIGHTNESS); break;
    case Color::YELLOW: SetAllColor(WS2812B_BRIGHTNESS, WS2812B_BRIGHTNESS, 0x00); break;
    case Color::NONE:   LedOff(); break;
    default: break;
    }
}

void Ws2812B::LedOff()
{
    SetAllColor(0x00, 0x00, 0x00);
}

void Ws2812B::SetColor(Color color)
{
    if (signal_queue == nullptr) return;
    Command command = {CommandType::SET_COLOR, color, Color::NONE};
    xQueueSend(signal_queue, &command, 0);
}

void Ws2812B::SetRed()
{
    SetColor(Color::RED);
}

void Ws2812B::SetGreen()
{
    SetColor(Color::GREEN);
}

void Ws2812B::SetBlue()
{
    SetColor(Color::BLUE);
}

void Ws2812B::SetWhite()
{
    SetColor(Color::WHITE);
}

void Ws2812B::SetYellow()
{
    SetColor(Color::YELLOW);
}

void Ws2812B::SetNone()
{
    SetColor(Color::NONE);
}

void Ws2812B::SendFlashCommand(CommandType type, Color color1, Color color2)
{
    if (signal_queue == nullptr) return;
    Command command = {type, color1, color2};
    xQueueSend(signal_queue, &command, 0);
}

void Ws2812B::FlashOnce(Color color1, Color color2)
{
    SendFlashCommand(CommandType::FLASH_ONCE, color1, color2);
}

void Ws2812B::FlashContinuous(Color color1, Color color2)
{
    SendFlashCommand(CommandType::FLASH_CONTINUOUS, color1, color2);
}

} // namespace ws2812

namespace
{
ws2812::Color WS2812B_ToCppColor(WS2812B_Color color)
{
    switch (color)
    {
    case WS2812B_COLOR_RED:    return ws2812::Color::RED;
    case WS2812B_COLOR_GREEN:  return ws2812::Color::GREEN;
    case WS2812B_COLOR_BLUE:   return ws2812::Color::BLUE;
    case WS2812B_COLOR_WHITE:  return ws2812::Color::WHITE;
    case WS2812B_COLOR_YELLOW: return ws2812::Color::YELLOW;
    case WS2812B_COLOR_NONE:   return ws2812::Color::NONE;
    default:                   return ws2812::Color::NONE;
    }
}
}

/*--------------------------------------------------------------------------*/
// C 兼容接口
extern "C" {

void WS2812B_SetRed(void)
{
    if (ws2812::g_ws2812b_instance != nullptr)
        ws2812::g_ws2812b_instance->SetRed();
}

void WS2812B_SetGreen(void)
{
    if (ws2812::g_ws2812b_instance != nullptr)
        ws2812::g_ws2812b_instance->SetGreen();
}

void WS2812B_SetBlue(void)
{
    if (ws2812::g_ws2812b_instance != nullptr)
        ws2812::g_ws2812b_instance->SetBlue();
}

void WS2812B_SetWhite(void)
{
    if (ws2812::g_ws2812b_instance != nullptr)
        ws2812::g_ws2812b_instance->SetWhite();
}

void WS2812B_SetYellow(void)
{
    if (ws2812::g_ws2812b_instance != nullptr)
        ws2812::g_ws2812b_instance->SetYellow();
}

void WS2812B_SetNone(void)
{
    if (ws2812::g_ws2812b_instance != nullptr)
        ws2812::g_ws2812b_instance->SetNone();
}

void WS2812B_FlashOnce(WS2812B_Color color1, WS2812B_Color color2)
{
    if (ws2812::g_ws2812b_instance != nullptr)
        ws2812::g_ws2812b_instance->FlashOnce(WS2812B_ToCppColor(color1),
                                              WS2812B_ToCppColor(color2));
}

void WS2812B_FlashContinuous(WS2812B_Color color1, WS2812B_Color color2)
{
    if (ws2812::g_ws2812b_instance != nullptr)
        ws2812::g_ws2812b_instance->FlashContinuous(WS2812B_ToCppColor(color1),
                                                    WS2812B_ToCppColor(color2));
}

} // extern "C"
