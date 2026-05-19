#pragma once

#include "RC_tim.h"
#include "RC_serial.h"
#include "RC_task.h"

constexpr uint32_t IR_TIMEOUT_MS = 500;    // 红外超时时间
constexpr uint32_t AXIS_TIMEOUT_MS = 500;  // 三轴+yaw数据超时时间

#ifdef __cplusplus

// 摄像头数据结构
struct Camera_Data_t {
       float x;
       float y;
       float z;
       float yaw;
};

// 摄像头帧结构
static constexpr uint8_t AXIS_FRAME_HEAD_0 = 0xAA;
static constexpr uint8_t AXIS_FRAME_HEAD_1 = 0xBB;
static constexpr uint8_t AXIS_FRAME_TAIL_0 = 0xCC;
static constexpr uint8_t AXIS_FRAME_TAIL_1 = 0xDD;
static constexpr uint8_t AXIS_DATA_LEN = 16;

//摄像头数据接收状态
enum AxisRxState
{
      AXIS_WAITING_FOR_HEAD_0,
      AXIS_WAITING_FOR_HEAD_1,
      AXIS_WAITING_FOR_DATA,
      AXIS_WAITING_FOR_TAIL_0,
      AXIS_WAITING_FOR_TAIL_1
};

namespace dock
{
    class DockControl : public task::ManagedTask
    {
    public:
        DockControl(tim::Tim *tim_, UART_HandleTypeDef &axis_huart_, UART_HandleTypeDef &ir_huart_);//定时器推荐周期为1ms
        ~DockControl() override = default;

        //是否启用自动脚本
        void Enable();
        void Disable();
        bool IsEnabled() const;

        //获取三轴+yaw数据（只读）
        Camera_Data_t GetAxisData() const;
        uint32_t GetAxisFrameSeq() const;
        bool IsAxisConnected() const;

    protected:
        void Task_Process() override;

    private:
        // 三轴+yaw数据串口接收类
        class AxisRx final : public serial::UartRx
        {
        public:
                AxisRx(DockControl &owner_, UART_HandleTypeDef &huart_);

        private:
                void Uart_Rx_It_Process(uint8_t *buf_, uint16_t len_) override;

                DockControl &owner;
                uint8_t rx_buf[64] = {0};
        };

        // 红外数据串口接收类
        class IrRx final : public serial::UartRx
        {
        public:
                IrRx(DockControl &owner_, UART_HandleTypeDef &huart_);

        private:
                void Uart_Rx_It_Process(uint8_t *buf_, uint16_t len_) override;

                DockControl &owner;
                uint8_t rx_buf[32] = {0};
        };

        // 三轴+yaw串口数据处理函数和红外串口数据处理函数
        void OnAxisRx(uint8_t *buf_, uint16_t len_);
        void OnIrRx(uint8_t *buf_, uint16_t len_);

        //根据三轴+yaw数据和红外数据控制底座与龙门架运动的函数
        void Motor_Ctrl_Process();

        AxisRx axis_rx;
        IrRx ir_rx;

        //自动脚本相关
        bool is_dock = false;   // 是否对接成功
        bool dock_timeout = false;   // 是否超时
        bool manual_enable = false; // 手动开关：true允许自动控制
        float dock_pos[3] = {0};
        float dock_yaw = 0;
        uint32_t last_axis_tick = 0;
        uint32_t last_ir_tick = 0;
        
        // 三轴+yaw数据接收状态和缓存
        AxisRxState axis_rx_state = AXIS_WAITING_FOR_HEAD_0;
        uint8_t axis_data_buffer[AXIS_DATA_LEN] = {0};
        uint8_t axis_data_index = 0;
        Camera_Data_t axis_data_ = {0.0f, 0.0f, 0.0f, 0.0f};
        bool axis_data_valid = false;
        uint32_t axis_frame_seq_ = 0;
        uint32_t axis_last_update_time_ = 0;
    };
}
#endif