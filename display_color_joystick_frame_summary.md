# 摇杆帧新增 display_color 字段说明

## 背景

原逻辑中，接收端通过摇杆帧的 `page` 字节高四位获取红蓝半场颜色，但只有当 `page` 低四位等于 `1` 时才更新颜色缓存。机器人或接收端重启后，如果遥控器没有重新进入 `page=1`，接收端只能继续使用默认颜色，不能可靠恢复显示屏上一次选择的红蓝半场。

## 修改目标

在发送端摇杆帧中新增独立颜色字段，保存并发送显示屏上一次发来的颜色值，保证接收端不再依赖 `page==1` 才能获得颜色。

字段名：`display_color`

## 发送端修改

工程：

`C:\Users\yurt\Desktop\rc26\遥控器32f4主控代码\remote_control_twolora_puls`

涉及文件：

- `Common/Datapool.h`
- `Common/Datapool.c`
- `App/tjc_huart_hmi.c`
- `App/communication.h`
- `App/communication.c`

主要变化：

- 新增全局缓存变量 `display_color`，默认值为 `1`。
- 显示屏发来 `PageFrame` 时，如果 `page_id` 低四位为 `1`，则把 `page_id >> 4` 保存到 `display_color`。
- `JoystickFrame_t` 新增 `uint8_t display_color` 字段。
- 每次发送摇杆帧时，把缓存的 `display_color` 填入新字段。

## 接收端修改

工程：

`C:\Users\yurt\Desktop\rc26\接收机h7代码\R1\RC10_LIB-FrameWork-main-v3\RC10_LIB-FrameWork-main`

涉及文件：

- `RC10_LIB/Module/Inc/Module_communication.h`
- `RC10_LIB/Module/Src/Module_communication.cpp`

主要变化：

- `JoystickFrame_t` 同步新增 `uint8_t display_color` 字段。
- 新增接收缓存 `rec_display_color`，默认值为 `1`。
- 解析摇杆帧时保存 `pFrame->display_color`。
- `GetColor()` 改为直接返回 `rec_display_color`，不再通过 `rec_page` 高四位且不再要求 `page==1`。

## 协议影响

摇杆帧长度增加 1 字节。

发送端和接收端必须同时更新固件，否则双方对 `JoystickFrame_t` 长度、CRC 位置和帧尾位置的理解会不一致，导致摇杆帧解析失败。

## 验证

已完成静态验证：

- 搜索确认发送端、接收端都已加入 `display_color` 字段。
- 搜索确认接收端旧的 `saved_color` 逻辑已移除。
- 对本次触碰文件运行 `git diff --check`，未发现新增空白错误。

未完成验证：

- 未进行 Keil 编译。
- 未进行烧录和实机通信测试。

