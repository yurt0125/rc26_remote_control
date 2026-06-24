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
    , flag(0)
    , start_time(0)
    , signal_queue(nullptr)
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
        signal_queue = xQueueCreate(10, sizeof(Signal));
        if (signal_queue == nullptr)
        {
            Error_Handler();
        }
        // 初始化完成后刷一次初始颜色
        ApplyColor(current_color);
        SpiSend();
        return;
    }

    Signal signal;
    uint32_t current_time = timer::Timer::Get_TimeStamp();

    // 非阻塞接收队列消息
    if (xQueueReceive(signal_queue, &signal, 0) == pdPASS)
    {
        if (signal == Signal::FLASH_ONCE)
        {
            // 闪烁：从常亮状态切换到绿色
            if (flag == 0)
            {
                flag = 1;
                ApplyColor(Signal::GREEN);
                start_time = current_time;
            }
        }
        else
        {
            // 颜色信号：切换到对应颜色并常亮
            current_color = signal;
            flag = 0;
            ApplyColor(signal);
        }
    }

    // 定时状态转移：双闪序列 (绿→原色→绿→原色)
    switch (flag)
    {
    case 1:  // 绿色 → 恢复颜色
        if (current_time - start_time > TIME_INTERVAL)
        {
            flag = 2;
            ApplyColor(current_color);
            start_time = current_time;
        }
        break;

    case 2:  // 恢复 → 第二次绿色
        if (current_time - start_time > TIME_INTERVAL)
        {
            flag = 3;
            ApplyColor(Signal::GREEN);
            start_time = current_time;
        }
        break;

    case 3:  // 绿色 → 恢复颜色
        if (current_time - start_time > TIME_INTERVAL)
        {
            flag = 4;
            ApplyColor(current_color);
            start_time = current_time;
        }
        break;

    case 4:  // 恢复 → 常亮
        if (current_time - start_time > TIME_INTERVAL)
        {
            flag = 0;
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

void Ws2812B::ApplyColor(Signal color)
{
    switch (color)
    {
    case Signal::RED:   SetAllColor(0xFF, 0x00, 0x00); break;
    case Signal::GREEN: SetAllColor(0x00, 0xFF, 0x00); break;
    case Signal::BLUE:  SetAllColor(0x00, 0x00, 0xFF); break;
    case Signal::WHITE: SetAllColor(0xFF, 0xFF, 0xFF); break;
    default: break;
    }
}

void Ws2812B::LedOff()
{
    SetAllColor(0x00, 0x00, 0x00);
}

void Ws2812B::SetRed()
{
    if (signal_queue == nullptr) return;
    Signal signal = Signal::RED;
    xQueueSend(signal_queue, &signal, 0);
}

void Ws2812B::SetGreen()
{
    if (signal_queue == nullptr) return;
    Signal signal = Signal::GREEN;
    xQueueSend(signal_queue, &signal, 0);
}

void Ws2812B::SetBlue()
{
    if (signal_queue == nullptr) return;
    Signal signal = Signal::BLUE;
    xQueueSend(signal_queue, &signal, 0);
}

void Ws2812B::SetWhite()
{
    if (signal_queue == nullptr) return;
    Signal signal = Signal::WHITE;
    xQueueSend(signal_queue, &signal, 0);
}

void Ws2812B::FlashOnce()
{
    if (signal_queue == nullptr) return;
    Signal signal = Signal::FLASH_ONCE;
    xQueueSend(signal_queue, &signal, 0);
}

} // namespace ws2812

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

void WS2812B_FlashOnce(void)
{
    if (ws2812::g_ws2812b_instance != nullptr)
        ws2812::g_ws2812b_instance->FlashOnce();
}

} // extern "C"


