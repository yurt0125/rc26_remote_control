local function run(event)
    lcd.clear()
    
    -- 直接读取 0x14 的值
    local rssi1 = getValue("1RSS") or 0
    local rssi2 = getValue("2RSS") or 0
    local lq = getValue("LQ") or 0
    local rsnr = getValue("RSNR") or 0
    
    lcd.drawText(0, 0, "RSSI1: " .. rssi1, MIDSIZE)
    lcd.drawText(0, 16, "RSSI2: " .. rssi2, MIDSIZE)
    lcd.drawText(0, 32, "LQ: " .. lq, MIDSIZE)
    lcd.drawText(0, 48, "RSNR: " .. rsnr, MIDSIZE)
    
    return 0
end