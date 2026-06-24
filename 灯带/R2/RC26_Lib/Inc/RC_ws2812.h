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

    // 信号/状态类型
    enum class Signal : uint8_t
    {
        NORMAL,
        WAIT,
        SUCCESS,
        FAIL
    };

    class Ws2812B : public task::ManagedTask
    {
    public:
        Ws2812B(SPI_HandleTypeDef* hspi_);
        ~Ws2812B() = default;

        // 公共接口：向队列发送信号
        void SendFail();
        void SendSuccess();

        // 设置全部灯为指定颜色
        void SetAllColor(uint8_t r, uint8_t g, uint8_t b);

    protected:
        void Task_Process() override;

    private:
        // SPI 句柄
        SPI_HandleTypeDef* hspi;

        // FreeRTOS 队列句柄
        QueueHandle_t signal_queue;

        // SPI 数据缓存 (每颗灯珠 24 字节)
        uint8_t spi_buf[WS2812B_AMOUNT * 24];

        // 灯条颜色显存
        ColorCache color_cache[WS2812B_AMOUNT];

        // 将单颗灯珠的 RGB 值编码为 SPI 波形数据
        void EncodeLed(uint16_t led_index, uint8_t r, uint8_t g, uint8_t b);

        // 通过 SPI DMA 发送灯条数据
        void SpiSend();

        // LED 显示函数
        void LedFail();
        void LedWait();
        void LedSuccess();
        void LedNormal();
        void LedOff();

        // 状态机变量
        uint8_t flag = 0;
        uint32_t start_time = 0;

        // 闪烁间隔 (系统 tick)
        static constexpr uint32_t TIME_INTERVAL = 80000;

        // 复位信号字节数
        static constexpr uint8_t RESET_BYTES = 100;
    };

    // 全局实例指针，供 C 兼容接口使用
    extern Ws2812B* g_ws2812b_instance;
}

extern "C" {
#endif

// C 兼容接口
void WS2812B_Send_SUCCESS(void);
void WS2812B_Send_FAIL(void);

#ifdef __cplusplus
}
#endif






