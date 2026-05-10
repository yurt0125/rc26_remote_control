-- Simple Attitude Display Script for CRSF 0x1E Frame
-- Compatible with Radiomaster Pocket / EdgeTX

-- 直接读取，不需要 getTelemetryId
local function run(event)
    -- Clear screen
    lcd.clear()

    -- 直接 getValue 读取（不需要 ID）
   -- 试试这些名字
local pitch = getValue("Ptch") or getValue("pitch") or 0
local roll = getValue("Roll") or getValue("roll") or 0  
local yaw = getValue("Yaw") or getValue("Hdg") or getValue("yaw") or 0
    -- 标题（RadioMaster Pocket 屏幕小，调小字体）
    lcd.drawText(64, 0, "ATTITUDE", MIDSIZE + CENTER + INVERS)
    
    -- 显示数值（Y坐标适配 128x64 屏幕）
    lcd.drawText(0, 16, string.format("Pitch:%.1f", pitch), SMLSIZE)
    lcd.drawText(0, 28, string.format("Roll:%.1f", roll), SMLSIZE)
    lcd.drawText(0, 40, string.format("Yaw:%.1f", yaw), SMLSIZE)

    return 0
end

return { run = run }