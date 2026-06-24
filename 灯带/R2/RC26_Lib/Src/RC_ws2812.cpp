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
        color_cache[i].R = 0x10;
        color_cache[i].G = 0x10;
        color_cache[i].B = 0x10;
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
        return;  // 初始化完成后直接返回，下个周期再处理
    }

    Signal signal;
    uint32_t current_time = timer::Timer::Get_TimeStamp();

    // 非阻塞接收队列消息
    if (xQueueReceive(signal_queue, &signal, 0) == pdPASS)
    {
        // 快速抢占：偶数 flag 时收到 FAIL 信号，立即跳转
        if (flag % 2 == 0 && signal == Signal::FAIL)
        {
            flag = 1;
            LedFail();
            start_time = current_time;
        }

        switch (flag)
        {
        case 0:
            if (signal == Signal::NORMAL)
            {
                LedNormal();
            }
            else if (signal == Signal::WAIT)
            {
                LedWait();
            }
            else if (signal == Signal::SUCCESS)
            {
                flag = 2;
                LedSuccess();
                start_time = current_time;
            }
            else if (signal == Signal::FAIL)
            {
                flag = 1;
                LedFail();
                start_time = current_time;
            }
            break;
        }
    }

    // 定时状态转移（不依赖队列消息也能推进）
    switch (flag)
    {
    case 2:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flag = 4;
            LedOff();
            start_time = current_time;
        }
        break;

    case 4:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flag = 6;
            LedSuccess();
            start_time = current_time;
        }
        break;

    case 6:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flag = 0;
            LedOff();
            start_time = current_time;
        }
        break;

    case 1:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flag = 3;
            LedOff();
            start_time = current_time;
        }
        break;

    case 3:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flag = 5;
            LedFail();
            start_time = current_time;
        }
        break;

    case 5:
        if (current_time - start_time > TIME_INTERVAL)
        {
            flag = 0;
            LedOff();
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
    static const uint8_t reset_buf[RESET_BYTES] = {0};

    // 将颜色缓存编码为 SPI 数据
    for (uint8_t iLED = 0; iLED < WS2812B_AMOUNT; iLED++)
    {
        EncodeLed(iLED, color_cache[iLED].R, color_cache[iLED].G, color_cache[iLED].B);
    }

    // 阻塞发送 LED 数据，确保发完
    HAL_SPI_Transmit(hspi, spi_buf, sizeof(spi_buf), HAL_MAX_DELAY);

    // 阻塞发送复位信号（大于 50us 的低电平）
    HAL_SPI_Transmit(hspi, (uint8_t*)reset_buf, RESET_BYTES, HAL_MAX_DELAY);
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

void Ws2812B::LedFail()
{
    SetAllColor(0x08, 0x00, 0x00); // red
}

void Ws2812B::LedWait()
{
    SetAllColor(0x03, 0x03, 0x03); // white
}

void Ws2812B::LedSuccess()
{
    SetAllColor(0x00, 0x08, 0x00); // green
}

void Ws2812B::LedNormal()
{
    SetAllColor(0x00, 0x00, 0x08); // blue
}

void Ws2812B::LedOff()
{
    SetAllColor(0x00, 0x00, 0x00); // off
}

void Ws2812B::SendFail()
{
    if (signal_queue == nullptr) return;  // 队列尚未创建，忽略
    Signal signal = Signal::FAIL;
    xQueueSend(signal_queue, &signal, 0);
}

void Ws2812B::SendSuccess()
{
    if (signal_queue == nullptr) return;  // 队列尚未创建，忽略
    Signal signal = Signal::SUCCESS;
    xQueueSend(signal_queue, &signal, 0);
}

} // namespace ws2812

/*--------------------------------------------------------------------------*/
// C 兼容接口
extern "C" {

void WS2812B_Send_SUCCESS(void)
{
    if (ws2812::g_ws2812b_instance != nullptr)
    {
        ws2812::g_ws2812b_instance->SendSuccess();
    }
}

void WS2812B_Send_FAIL(void)
{
    if (ws2812::g_ws2812b_instance != nullptr)
    {
        ws2812::g_ws2812b_instance->SendFail();
    }
}

} // extern "C"


