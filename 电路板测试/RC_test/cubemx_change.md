# CubeMX 手动修改清单

## 目的

将 FDCAN1、FDCAN2、FDCAN3 统一配置为 Classic CAN、500 kbps，并为每路配置发送 FIFO。本文档只记录人工修改步骤，`RC_test.ioc` 仍由操作者在 CubeMX 中修改和保存。

## 修改步骤

1. 用 STM32CubeMX 打开 `RC_test.ioc`。
2. 依次打开 FDCAN1、FDCAN2 和 FDCAN3 的 Parameter Settings。
3. 对三路 FDCAN 分别设置下表参数：

| 参数 | 设置值 |
|---|---:|
| Frame Format | Classic CAN |
| Mode | Normal |
| Nominal Prescaler | 25 |
| Nominal Sync Jump Width | 1 |
| Nominal Time Seg1 | 8 |
| Nominal Time Seg2 | 2 |
| Tx FIFO Queue Elements Number | 3 |
| Tx FIFO/Queue Mode | FIFO |
| Tx Element Size | 8 bytes |
| Auto Retransmission | Disable（保持现有测试策略） |

位时间计算：

```text
FDCAN kernel clock = 137.5 MHz
Bit rate = 137.5 MHz / [25 × (1 + 8 + 2)] = 500 kbit/s
Sample point = (1 + 8) / (1 + 8 + 2) = 81.8%
```

4. 如果 CubeMX 页面可以设置 Message RAM Offset，配置为：

| 外设 | Message RAM Offset（word） |
|---|---:|
| FDCAN1 | 0 |
| FDCAN2 | 64 |
| FDCAN3 | 128 |

5. 保存 `.ioc` 并重新生成代码。必须启用 **Keep User Code**，避免覆盖 `main.c` 的测试逻辑。

## Message RAM Offset 注意事项

某些 CubeMX 版本不提供 Message RAM Offset 输入项。如果无法在图形界面设置，每次生成代码后都要人工核对 `Core/Src/fdcan.c`，并保持：

```c
hfdcan1.Init.MessageRAMOffset = 0;
hfdcan2.Init.MessageRAMOffset = 64;
hfdcan3.Init.MessageRAMOffset = 128;
```

这三个参数不建议仅放在函数的 `USER CODE BEGIN ... Init 1` 区域，因为随后的 CubeMX 赋值会再次覆盖它们。应在生成后直接核对 HAL 初始化赋值，或在各 `MX_FDCANx_Init()` 的 `USER CODE BEGIN ... Init 2` 中加入初始化结果检查。

## 生成后验收清单

打开 `Core/Src/fdcan.c`，确认三路都满足：

```c
Init.FrameFormat = FDCAN_FRAME_CLASSIC;
Init.Mode = FDCAN_MODE_NORMAL;
Init.NominalPrescaler = 25;
Init.NominalSyncJumpWidth = 1;
Init.NominalTimeSeg1 = 8;
Init.NominalTimeSeg2 = 2;
Init.TxBuffersNbr = 0;
Init.TxFifoQueueElmtsNbr = 3;
Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
Init.TxElmtSize = FDCAN_DATA_BYTES_8;
```

最后再确认三路 Offset 分别为 0、64、128。如任何一项不符，不要直接上板测试。
