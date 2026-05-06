-- ========== 测试代码：3页面自动轮播，每5秒切换 ==========

local pages = {
    [0] = "PAGE:ALLSTOP",
    [1] = "PAGE:MANUAL", 
    [2] = "PAGE:AUTO",
}

local currentIdx = 0
local lastSwitchTime = 0
local INTERVAL = 5000  -- 5秒 = 5000毫秒

local function getTimeMs()
    return getTime() * 10  -- OpenTX时间单位是10ms
end

local function run(event)
    local now = getTimeMs()
    
    -- 5秒到，切换下一页
    if now - lastSwitchTime >= INTERVAL then
        currentIdx = currentIdx + 1
        if currentIdx > 2 then
            currentIdx = 0  -- 循环回第一页
        end
        lastSwitchTime = now
        lcd.clear()
    end
    
    -- 居中显示标题
    local title = pages[currentIdx]
    lcd.drawText(64, 28, title, MIDSIZE + CENTER + INVERS)
    
    -- 底部显示倒计时
    local remain = math.ceil((INTERVAL - (now - lastSwitchTime)) / 1000)
    lcd.drawText(64, 56, "Next: " .. remain .. "s", SMLSIZE + CENTER)
    
    return 0
end

return { run = run }