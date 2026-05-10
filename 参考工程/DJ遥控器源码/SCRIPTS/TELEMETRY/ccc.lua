local function run(event)
    -- Read custom telemetry data
    local angle1 = getValue('0C10')  -- Subtype 0x10 for angle1
    local angle2 = getValue('0C11')  -- Subtype 0x11 for angle2
    local ratio = getValue('0C12')   -- Subtype 0x12 for ratio

    -- Display on screen
    lcd.clear()
    lcd.drawText(1, 1, "Angle1: " .. (angle1 or "N/A"), SMLSIZE)
    lcd.drawText(1, 15, "Angle2: " .. (angle2 or "N/A"), SMLSIZE)
    lcd.drawText(1, 29, "Ratio: " .. (ratio or "N/A"), SMLSIZE)

    return 0
end

return { run = run }