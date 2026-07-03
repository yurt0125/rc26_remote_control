#include "tjc_huart_hmi.h"
#include "Datapool.h"
#include <string.h>

HMIContext g_HMI = {0};

/* ================= 环形缓冲区 (FIFO) 操作 ================= */
// 数据存入尾部 (tail)
static void FIFO_Push(hmi_FIFO_t* fifo, uint8_t data) {
    uint16_t next = (fifo->tail + 1) % RING_BUF_SIZE;
    if (next != fifo->head) { 
        fifo->buffer[fifo->tail] = data;
        fifo->tail = next;
    } else {
        fifo->drop_cnt++; // 缓冲区满，丢包并计数
    }
}

// 主循环从头部 (head) 取出处理
static int FIFO_Pop(hmi_FIFO_t* fifo, uint8_t* data) {
    if (fifo->head == fifo->tail) return 0; 
    *data = fifo->buffer[fifo->head];
    fifo->head = (fifo->head + 1) % RING_BUF_SIZE;
    return 1;
}

static uint16_t FIFO_Count(hmi_FIFO_t* fifo) {
    return (fifo->tail + RING_BUF_SIZE - fifo->head) % RING_BUF_SIZE;
}
/* ========================================================= */

void HMI_Task_Init(UART_HandleTypeDef *huart)
{
    g_HMI.huart = huart;
    g_HMI.tx_busy = 0;
    g_HMI.last_data_valid = 0;
    g_HMI.rx_fifo.drop_cnt = 0;
    g_HMI.tx_fifo.drop_cnt = 0;
    
    // 挂载 DMA 接收空闲中断至 DMA 专用连续缓冲区
    HAL_UARTEx_ReceiveToIdle_DMA(g_HMI.huart, g_HMI.dma_rx_buf, DMA_BUF_SIZE);
    for (uint8_t i = 0; i < COMM_COMMAND_COUNT; i++)
    {
    Communication_SendCommandFrame(i,0,0);
    }
}

static void HMI_StartTx(void)
{
    // 原子抢占发送器：关中断保护 tx_busy 和 FIFO 操作不被其他任务/ISR 打断
    __disable_irq();

    if (g_HMI.tx_busy) {
        __enable_irq();
        return;
    }
    g_HMI.tx_busy = 1;

    uint16_t count = FIFO_Count(&g_HMI.tx_fifo);
    if (count > 0) {
        if (count > DMA_BUF_SIZE) count = DMA_BUF_SIZE;

        for (uint16_t i = 0; i < count; i++) {
            FIFO_Pop(&g_HMI.tx_fifo, &g_HMI.dma_tx_buf[i]);
        }

        HAL_UART_Transmit_DMA(g_HMI.huart, g_HMI.dma_tx_buf, count);
        // tx_busy 保持 1，等 TxCplt ISR 来清
    } else {
        g_HMI.tx_busy = 0;
    }

    __enable_irq();
}

void HMI_Task_Loop(void)
{
    // TX: 如果发送空闲且 tx_fifo 有数据，拉起 DMA 发送
    if (g_HMI.tx_busy == 0 && FIFO_Count(&g_HMI.tx_fifo) > 0) {
        HMI_StartTx(); 
    }

    // RX: 需要至少 2 字节才能判断帧头类型
    while (FIFO_Count(&g_HMI.rx_fifo) >= 2) {
        uint16_t t_head = g_HMI.rx_fifo.head;
        uint8_t  byte1  = g_HMI.rx_fifo.buffer[t_head];
        uint8_t  byte2  = g_HMI.rx_fifo.buffer[(t_head + 1) % RING_BUF_SIZE];

        uint8_t frame_type    = 0; // 1=Page, 2=Setting, 3=Data(CC), 4=Command(DD)
        uint8_t frame_size    = 0;
        uint8_t expected_tail = 0;

        // 根据第二字节区分帧类型
        if (byte1 == 0x55) {
            if (byte2 == 0xAA) {
                frame_type    = 1;
                frame_size    = sizeof(PageFrame_t);
                expected_tail = 0x0A;
            } else if (byte2 == 0xBB) {
                frame_type    = 2;
                frame_size    = sizeof(SettingFrame_t);
                expected_tail = 0x0B;
            } else if (byte2 == 0xCC) {
                frame_type    = 3;
                frame_size    = sizeof(DataFrame_t);
                expected_tail = 0x0C;
            } else if (byte2 == 0xDD) {
                frame_type    = 4;
                frame_size    = sizeof(CommandFrame_t);
                expected_tail = 0x0D;
            }
        }

        if (frame_type == 0) {
            // 未识别的帧头，丢弃首字节继续搜寻
            g_HMI.rx_fifo.head = (g_HMI.rx_fifo.head + 1) % RING_BUF_SIZE;
            continue;
        }

        // FIFO 中数据不足完整一帧，等待下次 DMA 补货
        if (FIFO_Count(&g_HMI.rx_fifo) < frame_size) {
            break;
        }

        // 将疑似帧拷贝到线性缓冲区（最大 16 字节足够覆盖所有帧类型）
        uint8_t  frame_buf[16];
        uint16_t p = t_head;
        for (int i = 0; i < frame_size; i++) {
            frame_buf[i] = g_HMI.rx_fifo.buffer[p];
            p = (p + 1) % RING_BUF_SIZE;
        }

        // 校验帧尾
        if (frame_buf[frame_size - 1] != expected_tail) {
            // 坏帧，跳过首字节继续寻头
            g_HMI.rx_fifo.head = (g_HMI.rx_fifo.head + 1) % RING_BUF_SIZE;
            continue;
        }

        // --- 帧校验通过，按类型与当前界面分发处理 ---
        switch (frame_type) {
            case 1: { // PageFrame (0x55 0xAA) — 任意界面均可接收
                PageFrame_t* pf = (PageFrame_t*)frame_buf;
                uint8_t prev_state = hmi_state;
                g_HMI.page_id = pf->page_id;
                hmi_state = pf->page_id;

                // 离开数据显示页面时，清空发送队列并中止 DMA
                // 防止残留 DataFrame 发到其他页面导致屏幕协议错乱、按键失效
                if (prev_state == 2 && hmi_state != 2) {
                    __disable_irq();
                    g_HMI.tx_fifo.head = g_HMI.tx_fifo.tail;
                    __enable_irq();
                    HAL_UART_AbortTransmit(g_HMI.huart);
                    g_HMI.tx_busy = 0;  // Abort 不会触发 TxCplt，必须手动清除
                    // 注意：不要在此处调用 ReceiveToIdle_DMA，否则 gState 变为 BUSY_RX，
                    // 导致后续 HAL_UART_Transmit_DMA 因 gState != READY 而静默失败
                }

                // 切到数据显示页面时，若有缓存数据则主动刷新一帧
                if (prev_state != 2 && hmi_state == 2 && g_HMI.last_data_valid) {
                    HMI_SendDataFrame(g_HMI.data_send_x, g_HMI.data_send_y, g_HMI.data_send_z,
                                      g_HMI.data_send_status, g_HMI.data_send_mode,
                                      g_HMI.data_send_command[0], g_HMI.data_send_command[1],
                                      g_HMI.data_send_KFS_want_place1, g_HMI.data_send_KFS_want_place2,
                                      g_HMI.data_send_spear, g_HMI.data_send_KFS_Keepplace);
                }
                break;
            }
            case 2: { // SettingFrame (0x55 0xBB) — 仅数据设置界面(page 1)
								uint8_t hmi_state_withmask=hmi_state&0x0F;
                if (hmi_state_withmask == 1) {
                    SettingFrame_t* sf = (SettingFrame_t*)frame_buf;
                    g_HMI.setting_rx_command  = sf->command;
                    g_HMI.setting_rx_load[0]  = sf->load1;
                    g_HMI.setting_rx_load[1]  = sf->load2;
										KFS_load1=g_HMI.setting_rx_load[0];
										KFS_load2=g_HMI.setting_rx_load[1];
                    Communication_SendSettingFrame(g_HMI.setting_rx_command, g_HMI.setting_rx_load[0], g_HMI.setting_rx_load[1]);
                }
                break;
            }
            // case 3: { // DataFrame (0x55 0xCC) — 仅数据显示界面(page 2)
            //     if (hmi_state == 2) {
            //         DataFrame_t* df = (DataFrame_t*)frame_buf;
            //         g_HMI.data_send_x             = df->x;
            //         g_HMI.data_send_y             = df->y;
            //         g_HMI.data_send_z             = df->z;
            //         g_HMI.data_send_status        = df->status;
            //         g_HMI.data_send_mode          = df->mode;
            //         g_HMI.data_send_command[0] = df->send_command1;
            //         g_HMI.data_send_command[1] = df->send_command2;
            //     }
            //     break;
            // }
            case 4: { // CommandFrame (0x55 0xDD) — 仅发送命令界面(page 3)
               if (hmi_state == 3||hmi_state == 2) { // 发送命令界面(page 3) 和 数据显示界面(page 2) 都可接收 CommandFrame
                   CommandFrame_t* cf = (CommandFrame_t*)frame_buf;
                   g_HMI.rx_command = cf->command;
                   g_HMI.rx_load[0] = cf->load1;
                   g_HMI.rx_load[1] = cf->load2;
                   Communication_SendCommandFrame(g_HMI.rx_command, g_HMI.rx_load[0], g_HMI.rx_load[1]);
               }
               break;
           }
        }

        // 消费完整一帧，移动 head 到帧尾之后
        g_HMI.rx_fifo.head = p;
    }
}

// void HMI_Timer_Callback_Wrapper(void)
// {
        
// 		JoystickFrame_t frame;
//     frame.header[0] = 0xAA;
//     frame.header[1] = 0x55;
//     // 把 4 个16位的摇杆通道数据打包
//     frame.ch1 = g_HMI.tx_load[0];
//     frame.ch2 = g_HMI.tx_load[1];
//     frame.ch3 = g_HMI.tx_load[2];
//     frame.ch4 = g_HMI.tx_load[3];
//     frame.crc = crc8((uint8_t*)&frame + 2, sizeof(JoystickFrame_t) - 2);
//     frame.tail = 0xDE;

//     uint8_t* ptr = (uint8_t*)&frame;
//     for (int i = 0; i < sizeof(JoystickFrame_t); i++) {
//         FIFO_Push(&g_HMI.tx_fifo, ptr[i]);
//     }
        
//     // 尝试拉起发送：如果底部DMA空闲，且队列里有东西
//     if (g_HMI.tx_busy == 0 && FIFO_Count(&g_HMI.tx_fifo) > 0) {
//        HMI_UartTxCplt_Callback_Wrapper(g_HMI.huart); 
//     }
// }



// 发送 SettingFrame (0x55 0xBB) — 仅数据设置界面(page 1) 发送
void HMI_SendSettingFrame(uint8_t command, uint8_t load1, uint8_t load2)
{
		uint8_t hmi_state_withmask=hmi_state&0x0F;
    if (hmi_state_withmask != 1) return;

    SettingFrame_t frame;
    frame.header[0] = 0x55;
    frame.header[1] = 0xBB;
    frame.command   = command;
    frame.load1     = load1;
    frame.load2     = load2;
    frame.tail      = 0x0B;

    __disable_irq();
    g_HMI.setting_tx_command  = command;
    g_HMI.setting_tx_load[0]  = load1;
    g_HMI.setting_tx_load[1]  = load2;
    __enable_irq();

    uint8_t* ptr = (uint8_t*)&frame;
    for (int i = 0; i < sizeof(SettingFrame_t); i++) {
        FIFO_Push(&g_HMI.tx_fifo, ptr[i]);
    }

    if (g_HMI.tx_busy == 0) {
        HMI_StartTx();
    }
}

// 发送 DataFrame (0x55 0xEE) — 仅数据显示界面(page 2)和命令发送界面(page 3) 发送
void HMI_ButtonTransmitFrame(uint8_t command, uint8_t load1, uint8_t load2)
{
    if (hmi_state != 2 && hmi_state != 3) return;

    SettingFrame_t frame;
    frame.header[0] = 0x55;
    frame.header[1] = 0xEE;
    frame.command   = command;
    frame.load1     = load1;
    frame.load2     = load2;
    frame.tail      = 0x0E;

    // __disable_irq();
    // g_HMI.setting_tx_command  = command;
    // g_HMI.setting_tx_load[0]  = load1;
    // g_HMI.setting_tx_load[1]  = load2;
    // __enable_irq();

    uint8_t* ptr = (uint8_t*)&frame;
    for (int i = 0; i < sizeof(SettingFrame_t); i++) {
        FIFO_Push(&g_HMI.tx_fifo, ptr[i]);
    }

    if (g_HMI.tx_busy == 0) {
        HMI_StartTx();
    }
}

// 发送 DataFrame (0x55 0xCC) — 仅数据显示界面(page 2) 发送
void HMI_SendDataFrame(int16_t x, int16_t y, int16_t z,
                       uint8_t status, uint8_t mode,
                       uint8_t send_cmd1, uint8_t send_cmd2,
                       uint8_t KFS_want_place1, uint8_t KFS_want_place2,
                       uint8_t spear, uint8_t KFS_Keepplace)
{
    // 无条件缓存最新一帧数据，供切回页面时刷新
    __disable_irq();
    g_HMI.data_send_x                = x;
    g_HMI.data_send_y                = y;
    g_HMI.data_send_z                = z;
    g_HMI.data_send_status           = status;
    g_HMI.data_send_mode             = mode;
    g_HMI.data_send_command[0]       = send_cmd1;
    g_HMI.data_send_command[1]       = send_cmd2;
    g_HMI.data_send_KFS_want_place1  = KFS_want_place1;
    g_HMI.data_send_KFS_want_place2  = KFS_want_place2;
    g_HMI.data_send_spear            = spear;
    g_HMI.data_send_KFS_Keepplace    = KFS_Keepplace;
    g_HMI.last_data_valid            = 1;
    __enable_irq();

    // 仅在数据显示页面才实际发送到屏幕
    if (hmi_state != 2) return;

    // 限制发送频率：周期不小于 50ms (限制在约 20Hz 刷新率)
    static uint32_t last_send_tick = 0;
    uint32_t current_tick = HAL_GetTick();
    if ((current_tick - last_send_tick) < 50) {
        return;
    }
    last_send_tick = current_tick;

    DataFrame_t frame;
    frame.header[0]        = 0x55;
    frame.header[1]        = 0xCC;
    frame.x                = x;
    frame.y                = y;
    frame.z                = z;
    frame.status           = status;
    frame.mode             = mode;
    frame.send_command1    = send_cmd1;
    frame.send_command2    = send_cmd2;
    frame.KFS_want_place1  = KFS_want_place1;
    frame.KFS_want_place2  = KFS_want_place2;
    frame.spear            = spear;
    frame.KFS_Keepplace    = KFS_Keepplace;
    frame.tail             = 0x0C;
    uint8_t *ptr = (uint8_t *)&frame;
    for (int i = 0; i < sizeof(DataFrame_t); i++) {
        FIFO_Push(&g_HMI.tx_fifo, ptr[i]);
    }

    if (g_HMI.tx_busy == 0) {
        HMI_StartTx();
    }
}



void HMI_UartTxCplt_Callback_Wrapper(UART_HandleTypeDef *huart)
{
    if (huart != g_HMI.huart) return;

    // ISR 上下文：DMA 刚完成，清除忙标志
    g_HMI.tx_busy = 0;

    uint16_t count = FIFO_Count(&g_HMI.tx_fifo);
    if (count > 0) {
        if (count > DMA_BUF_SIZE) count = DMA_BUF_SIZE;

        for (uint16_t i = 0; i < count; i++) {
            FIFO_Pop(&g_HMI.tx_fifo, &g_HMI.dma_tx_buf[i]);
        }

        g_HMI.tx_busy = 1;
        HAL_UART_Transmit_DMA(g_HMI.huart, g_HMI.dma_tx_buf, count);
    }
}

void HMI_UartRx_Callback_Wrapper(UART_HandleTypeDef *huart, uint16_t size)
{
    if (g_HMI.huart == huart) {
        // DMA 中断来了，将其固定缓存段里收到的数据复制到业务层的 RX 环形缓冲区中
        for (uint16_t i = 0; i < size; i++) {
            FIFO_Push(&g_HMI.rx_fifo, g_HMI.dma_rx_buf[i]);
        }
        
        // 立即开启下一次 DMA 接收，防止漏包
        HAL_UARTEx_ReceiveToIdle_DMA(g_HMI.huart, g_HMI.dma_rx_buf, DMA_BUF_SIZE);
    }
}

void HMI_UartError_Callback_Wrapper(UART_HandleTypeDef *huart)
{
    if (g_HMI.huart == huart) {
        // UART 出错时强制复位发送状态，防止 tx_busy 卡死导致屏幕冻结
        g_HMI.tx_busy = 0;
        HAL_UART_AbortTransmit(g_HMI.huart);
        // 重启接收 DMA
        HAL_UARTEx_ReceiveToIdle_DMA(g_HMI.huart, g_HMI.dma_rx_buf, DMA_BUF_SIZE);
    }
}

