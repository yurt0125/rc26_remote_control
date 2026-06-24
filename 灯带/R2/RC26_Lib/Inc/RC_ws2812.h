#pragma once

#include <stdint.h>
#include "spi.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "RC_task.h"
#include "RC_timer.h"

// 根据spi频率的不同，需要更改编码0和编码1的字节大小
//            编码 0 : 11100000
#define CODE_0      0xE0
//            编码 1 : 11111100
#define CODE_1      0xFC

// ws2812b灯珠数量
#define WS2812B_AMOUNT     20

#ifdef __cplusplus
namespace ws2812
{
    // LED颜色缓存
    struct ColorCache
    {
        uint8_t R;
        uint8_t G;
        uint8_t B;
    };

    // 信号类型
    enum class Signal : uint8_t
    {
        RED,
        GREEN,
        BLUE,
        WHITE,
        FLASH_ONCE
    };

    class Ws2812B : public task::ManagedTask
    {
    public:
        Ws2812B(SPI_HandleTypeDef* hspi_);
        ~Ws2812B() = default;

        // 公共接口：设置常亮颜色
        void SetRed();
        void SetGreen();
        void SetBlue();
        void SetWhite();

        // 公共接口：当前颜色闪烁一次（灭→恢复）
        void FlashOnce();

        // 设置全部灯为指定颜色
        void SetAllColor(uint8_t r, uint8_t g, uint8_t b);

    protected:
        void Task_Process() override;

    private:
        // SPI 句柄
        SPI_HandleTypeDef* hspi;

        // FreeRTOS 队列句柄
        QueueHandle_t signal_queue;

        // 常量
        static constexpr uint8_t RESET_BYTES = 100;

        // SPI 数据缓存 (DMA 需要放在 AXI SRAM)
        static uint8_t spi_buf[WS2812B_AMOUNT * 24 + RESET_BYTES];

        // 灯条颜色显存
        ColorCache color_cache[WS2812B_AMOUNT];

        // 将单颗灯珠的 RGB 值编码为 SPI 波形数据
        void EncodeLed(uint16_t led_index, uint8_t r, uint8_t g, uint8_t b);

        // 通过 SPI DMA 发送灯条数据
        void SpiSend();

        // LED 内部函数
        void ApplyColor(Signal color);
        void LedOff();

        // 状态机变量
        uint8_t flag = 0;           // 0=常亮; 1=闪烁灭; 2=闪烁恢复
        uint32_t start_time = 0;
        Signal current_color = Signal::RED;   // 初始颜色

        // 闪烁间隔 (系统 tick)
        static constexpr uint32_t TIME_INTERVAL = 80000;
    };

    // 全局实例指针，供 C 兼容接口使用
    extern Ws2812B* g_ws2812b_instance;
}

extern "C" {
#endif

// C 兼容接口
void WS2812B_SetRed(void);
void WS2812B_SetGreen(void);
void WS2812B_SetBlue(void);
void WS2812B_SetWhite(void);
void WS2812B_FlashOnce(void);

#ifdef __cplusplus
}
#endif






