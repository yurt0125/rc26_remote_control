# RC26 电路板测试工程检查报告

> MCU：STM32H723ZGTx（LQFP144）  
> CubeMX：6.17.0，STM32Cube FW_H7 V1.13.0  
> 最后静态复核：2026-06-27

## 1. 当前结论

工程已具备上板测试所需的基本配置和应用层测试逻辑：3 路 FDCAN 各发送一帧，10 路 UART/USART 各执行一次阻塞发送。

本结论仅表示源码和 CubeMX 生成配置的静态检查通过。当前环境未找到 Keil/Arm Compiler，因此没有声称已完成编译、烧录或硬件实测。上板前仍必须按 `test_process.md` 完成 Keil 编译和接线检查。

## 2. CubeMX 配置检查

### 2.1 系统配置

| 项目 | 当前值 | 结果 |
|---|---:|---|
| MCU | STM32H723ZGTx | 通过 |
| HSE | 25 MHz | 通过 |
| SYSCLK | 550 MHz | 通过 |
| HCLK | 275 MHz | 通过 |
| FDCAN kernel clock | 137.5 MHz | 通过 |
| UART kernel clock | 137.5 MHz | 通过 |
| HAL timebase | TIM6 | 通过 |
| I-Cache / D-Cache | Enabled | 通过 |
| SWD | PA13 / PA14 | 通过 |

### 2.2 FDCAN 配置

`.ioc` 与最新生成的 `Core/Src/fdcan.c` 相符：

| 项目 | FDCAN1 | FDCAN2 | FDCAN3 |
|---|---:|---:|---:|
| RX / TX | PA11 / PA12 | PB12 / PB13 | PF6 / PF7 |
| GPIO AF | AF9 | AF9 | AF2 |
| Frame / Mode | Classic / Normal | Classic / Normal | Classic / Normal |
| Nominal Prescaler | 25 | 25 | 25 |
| Nominal SJW | 1 | 1 | 1 |
| Nominal TimeSeg1 | 8 | 8 | 8 |
| Nominal TimeSeg2 | 2 | 2 | 2 |
| Nominal bit rate | 500 kbps | 500 kbps | 500 kbps |
| TX FIFO elements | 3 | 3 | 3 |
| TX element size | 8 bytes | 8 bytes | 8 bytes |
| Message RAM Offset | 0 | 64 | 128 |

波特率计算：`137.5 MHz / [25 × (1 + 8 + 2)] = 500 kbps`。三路 Message RAM 起始 Offset 不同，本测试配置下不会互相覆盖。

### 2.3 UART/USART 配置

全部配置为 115200-8N1、TX/RX、无硬件流控。

| 外设 | TX | RX | GPIO AF |
|---|---|---|---|
| UART4 | PA0 | PA1 | AF8 |
| UART5 | PC12 | PD2 | AF8 |
| UART7 | PE8 | PE7 | AF7 |
| UART8 | PE1 | PE0 | AF8 |
| UART9 | PG1 | PG0 | AF11 |
| USART1 | PB14 | PB15 | AF4 |
| USART2 | PA2 | PA3 | AF7 |
| USART3 | PB10 | PB11 | AF7 |
| USART6 | PC6 | PC7 | AF7 |
| USART10 | PE3（AF11） | PE2（AF4） | TX/RX 使用不同 AF |

USART10 的 PE3 TX 与 PE2 RX 分别使用 AF11 和 AF4，这与当前 CubeMX 生成代码一致，不应在接线表中统一写成 AF11。

## 3. 应用层修改记录

`Core/Src/main.c` 已完成：

- 去除 Banner 的错误固定发送长度，统一按字符串实际长度发送，消除越界读取。
- 分别检查 FDCAN1/2/3 的 `HAL_FDCAN_Start()` 结果。
- CAN 日志区分 `QUEUE [OK]` 和 `TX [OK]`。前者只表示进入 TX FIFO，后者表示对应 TX buffer 的 transmission occurred 标志已置位。
- CAN 发送等待设置 100 ms 超时；超时后撤销请求，并输出 Last Error Code 和 Bus-Off 状态。
- 保留 3 路 CAN 和 10 路串口的单次测试。测试完成后不自动重复发送。

CAN 测试帧：

| 属性 | 值 |
|---|---|
| ID | Standard `0x123` |
| Frame | Classic data frame |
| DLC | 8 |
| Data | `RC26_CAN` |

串口发送文本格式：`RC26 UART Test: <interface>\r\n`。UART4 同时承担日志和自身 TX 测试，所以该端口会看到混合输出，属于预期行为。

## 4. 正确接线原则

- MCU 的 FDCAN_TX/FDCAN_RX 必须连到 CAN 收发器的 TXD/RXD。
- USB-CAN 只连收发器总线侧的 CAN_H/CAN_L，不得直接 PA11/PA12、PB12/PB13 或 PF6/PF7。
- USB-CAN 必须与板卡共地，并使用正确终端电阻。
- USB-CAN 应设为 Classic CAN、500 kbps、标准帧、正常模式。Listen-only 无法提供 ACK，不适合本发送确认测试。

## 5. 通过标准

不能仅根据 HAL 返回值判定新焊电路板通过。最终通过必须同时满足：

- Keil 编译 0 Error，且无未确认警告。
- FDCAN1/2/3 在 USB-CAN 上均实际收到 ID `0x123`、DLC 8、数据 `RC26_CAN`。
- 10 路串口 TX 均通过 USB-TTL 或示波器/逻辑分析仪实际观测到完整数据。
- UART4 日志不得出现未解释的 `[FAIL]`、`[TIMEOUT]` 或 Bus-Off。

## 6. 文档索引

- CubeMX 手动配置及生成后核对：`cubemx_change.md`
- 实际接线、操作顺序、记录表与故障排查：`test_process.md`

## 7. 未在本环境完成的项目

- Keil/Arm Compiler 构建。
- ST-Link 烧录。
- 实际串口波形/数据验证。
- USB-CAN 实际收帧验证。

以上项目必须由上板测试阶段完成，不得根据静态检查推定已通过。
