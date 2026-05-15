#include "RC_dock_control.h"

#include <cstring>

namespace dock
{

DockControl::AxisRx::AxisRx(DockControl &owner_, UART_HandleTypeDef &huart_)
	: serial::UartRx(huart_, rx_buf, sizeof(rx_buf), true, true), owner(owner_)
{
}

void DockControl::AxisRx::Uart_Rx_It_Process(uint8_t *buf_, uint16_t len_)
{
	owner.OnAxisRx(buf_, len_);
}

DockControl::IrRx::IrRx(DockControl &owner_, UART_HandleTypeDef &huart_)
	: serial::UartRx(huart_, rx_buf, sizeof(rx_buf), true, true), owner(owner_)
{
}

void DockControl::IrRx::Uart_Rx_It_Process(uint8_t *buf_, uint16_t len_)
{
	owner.OnIrRx(buf_, len_);
}

DockControl::DockControl(tim::Tim *tim_, UART_HandleTypeDef &axis_huart_, UART_HandleTypeDef &ir_huart_)
	: task::ManagedTask("DockCtrl", 28, 256, task::TASK_PERIOD, 1),
	  axis_rx(*this, axis_huart_),
	  ir_rx(*this, ir_huart_)
{
	axis_rx.Uart_Rx_Start();
	ir_rx.Uart_Rx_Start();
}

void DockControl::Enable()
{
	manual_enable = true;
}

void DockControl::Disable()
{
	manual_enable = false;
	is_dock = false;
	dock_timeout = false;
}

bool DockControl::IsEnabled() const
{
	return manual_enable;
}

void DockControl::OnAxisRx(uint8_t *buf_, uint16_t len_)
{
	for (uint16_t i = 0; i < len_; i++)
	{
		uint8_t byte = buf_[i];

		switch (axis_rx_state)
		{
		case AXIS_WAITING_FOR_HEAD_0:
			if (byte == AXIS_FRAME_HEAD_0)
			{
				axis_rx_state = AXIS_WAITING_FOR_HEAD_1;
			}
			break;

		case AXIS_WAITING_FOR_HEAD_1:
			if (byte == AXIS_FRAME_HEAD_1)
			{
				axis_rx_state = AXIS_WAITING_FOR_DATA;
				axis_data_index = 0;
			}
			else
			{
				if (byte == AXIS_FRAME_HEAD_0)
				{
					axis_rx_state = AXIS_WAITING_FOR_HEAD_1;
				}
				else
				{
					axis_rx_state = AXIS_WAITING_FOR_HEAD_0;
				}
			}
			break;

		case AXIS_WAITING_FOR_DATA:
			axis_data_buffer[axis_data_index++] = byte;
			if (axis_data_index >= AXIS_DATA_LEN)
			{
				axis_rx_state = AXIS_WAITING_FOR_TAIL_0;
			}
			break;

		case AXIS_WAITING_FOR_TAIL_0:
			if (byte == AXIS_FRAME_TAIL_0)
			{
				axis_rx_state = AXIS_WAITING_FOR_TAIL_1;
			}
			else
			{
				axis_rx_state = AXIS_WAITING_FOR_HEAD_0;
			}
			break;

		case AXIS_WAITING_FOR_TAIL_1:
			if (byte == AXIS_FRAME_TAIL_1)
			{
				std::memcpy(&axis_data_, axis_data_buffer, sizeof(Camera_Data_t));
				dock_pos[0] = axis_data_.x;
				dock_pos[1] = axis_data_.y;
				dock_pos[2] = axis_data_.z;
				dock_yaw = axis_data_.yaw;
				axis_last_update_time_ = HAL_GetTick();
				axis_data_valid = true;
				axis_frame_seq_++;
				last_axis_tick = axis_last_update_time_;
				axis_rx_state = AXIS_WAITING_FOR_HEAD_0;
			}
			else
			{
				axis_rx_state = AXIS_WAITING_FOR_HEAD_0;
			}
			break;

		default:
			axis_rx_state = AXIS_WAITING_FOR_HEAD_0;
			break;
		}
	}
}

void DockControl::OnIrRx(uint8_t *buf_, uint16_t len_)
{
	(void)buf_;
	last_ir_tick = HAL_GetTick();
	(void)len_;
}


void DockControl::Task_Process()
{
	if (!manual_enable)
	{
		dock_timeout = true;
		return;
	}

	uint32_t now = HAL_GetTick();

	if ((now - last_axis_tick > AXIS_TIMEOUT_MS) || (now - last_ir_tick > IR_TIMEOUT_MS))
	{
        dock_timeout = true;
		return;
	}

    dock_timeout = false;
	// Motor_Ctrl_Process();
    return;
}

Camera_Data_t DockControl::GetAxisData() const
{
	return axis_data_;
}

uint32_t DockControl::GetAxisFrameSeq() const
{
	return axis_frame_seq_;
}

bool DockControl::IsAxisConnected() const
{
	return (HAL_GetTick() - axis_last_update_time_) < 500;
}

}