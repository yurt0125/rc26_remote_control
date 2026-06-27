# RC26 新焊电路板测试流程

## 1. 测试目标与工具

本流程验证 3 路 FDCAN 发送和 10 路 UART/USART 阻塞发送。不测试 MCU 串口接收、CAN 接收或内部回环。

准备：

- ST-Link/DAP-Link 和 Keil MDK-ARM。
- USB-CAN 工具。
- USB-TTL 串口工具，可逐路复用。
- 合适的 CAN 收发器、CAN_H/CAN_L 总线和 120 Ω 终端电阻。
- 所有仪器与被测板共地。

## 2. 编译前准备

1. 按 `cubemx_change.md` 在 CubeMX 中修改三路 FDCAN，保存 `.ioc` 并生成代码。
2. 重新打开 `Core/Src/fdcan.c`，检查 500 kbps、3 个 TX FIFO 元素和 Message RAM Offset 0/64/128 没有被覆盖。
3. 检查 `Core/Src/main.c` 中仍存在 FDCAN1/2/3 以及全部 10 路串口测试。
4. 打开 `MDK-ARM/RC_test.uvprojx`，Build，要求 0 Error。警告应逐条确认，不得直接忽略。
5. 烧录程序，暂不要复位运行，先完成接线。

## 3. 串口接线与测试

串口参数统一为 115200 baud、8 data bits、no parity、1 stop bit、no flow control。USB-TTL RX 接被测 TX，GND 接 GND。本测试只发送，USB-TTL TX 可以不接。

| 接口 | TX | RX（本测试不使用） | 预期发送文本 |
|---|---|---|---|
| UART4 | PA0 | PA1 | `RC26 UART Test: UART4` |
| UART5 | PC12 | PD2 | `RC26 UART Test: UART5` |
| UART7 | PE8 | PE7 | `RC26 UART Test: UART7` |
| UART8 | PE1 | PE0 | `RC26 UART Test: UART8` |
| UART9 | PG1 | PG0 | `RC26 UART Test: UART9` |
| USART1 | PB14 | PB15 | `RC26 UART Test: USART1` |
| USART2 | PA2 | PA3 | `RC26 UART Test: USART2` |
| USART3 | PB10 | PB11 | `RC26 UART Test: USART3` |
| USART6 | PC6 | PC7 | `RC26 UART Test: USART6` |
| USART10 | PE3 | PE2 | `RC26 UART Test: USART10` |

UART4 同时是日志口和被测口，因此会同时看到 Banner、CAN 结果、UART4 测试字符串和其他串口状态，这是预期现象。

串口通过条件：

- 对应 TX 引脚可收到完整的预期文本和 `\r\n`。
- 波特率正确，无乱码、无多余随机字节。
- UART4 日志中该路显示 `[OK]`。注意：该 `[OK]` 只表示 HAL 阻塞发送完成，仍需 USB-TTL 实际收到数据才能判定硬件通过。

## 4. CAN 接线与测试

STM32 的 FDCAN_TX/FDCAN_RX 是逻辑电平信号，**不能直接 USB-CAN 的 CAN_H/CAN_L**。必须经过板载或外接 CAN 收发器。

| 接口 | MCU RX | MCU TX | USB-CAN 接线 |
|---|---|---|---|
| FDCAN1 | PA11 | PA12 | 对应收发器总线端 CAN_H/CAN_L |
| FDCAN2 | PB12 | PB13 | 对应收发器总线端 CAN_H/CAN_L |
| FDCAN3 | PF6 | PF7 | 对应收发器总线端 CAN_H/CAN_L |

接线步骤：

1. 确认收发器供电、工作模式以及 standby/enable 引脚状态正确。
2. USB-CAN CAN_H 接收发器 CAN_H，CAN_L 接 CAN_L，并共地。
3. 总线两个物理末端各放置 120 Ω；断电测量 CAN_H 与 CAN_L 之间通常约为 60 Ω。
4. USB-CAN 设为 Classic CAN、500 kbps、Standard ID，启用正常模式，不要使用 listen-only，以便向节点返回 ACK。
5. 一次只对应验证一路 CAN，复位板卡触发一次测试。

每路预期收到：

| 属性 | 预期值 |
|---|---|
| Frame | Classic CAN data frame |
| ID | Standard `0x123` |
| DLC | 8 |
| Data ASCII | `RC26_CAN` |
| Data HEX | `52 43 32 36 5F 43 41 4E` |

CAN 通过条件：USB-CAN 收到上述完整帧，且 UART4 日志对应路显示 `QUEUE [OK], TX [OK]`。`QUEUE [OK]` 只代表已加入发送 FIFO，不能单独作为硬件通过依据。

## 5. 执行顺序

1. 完成 UART4 日志口、当前待测串口和当前待测 CAN 接线。
2. 打开 UART4 串口终端和 USB-CAN 接收窗口。
3. 复位被测板。程序仅执行一遍，完成后停在主循环中，不会自动重复发送。
4. 记录三路 CAN 和十路串口结果。切换物理接线后需要再次复位。

## 6. 建议记录表

| 接口 | HAL/日志结果 | 工具实际收到 | 最终结论 | 备注 |
|---|---|---|---|---|
| FDCAN1 |  |  |  |  |
| FDCAN2 |  |  |  |  |
| FDCAN3 |  |  |  |  |
| UART4 |  |  |  |  |
| UART5 |  |  |  |  |
| UART7 |  |  |  |  |
| UART8 |  |  |  |  |
| UART9 |  |  |  |  |
| USART1 |  |  |  |  |
| USART2 |  |  |  |  |
| USART3 |  |  |  |  |
| USART6 |  |  |  |  |
| USART10 |  |  |  |  |

## 7. 常见故障

| 现象 | 检查项 |
|---|---|
| FDCAN start `[FAIL]` | 检查 HAL 错误码、CubeMX 生成参数和 Message RAM Offset |
| `QUEUE [FAIL]` | 检查 TX FIFO 数量是否为 3、FDCAN 是否启动 |
| `TX [TIMEOUT]` | 检查收发器使能、波特率、CAN_H/CAN_L、共地和 ACK |
| `TX [FAIL] LEC=3` | 通常为 ACK 错误；确认 USB-CAN 不是 listen-only |
| USB-CAN 收不到帧 | 检查 500 kbps、标准帧、收发器、终端电阻和实际板端网标 |
| 串口乱码 | 检查 115200-8N1、共地、TTL 电平以及 TX 引脚 |
| 某路串口无数据 | 对照引脚表，检查焊接、连通性和外部收发器使能 |
