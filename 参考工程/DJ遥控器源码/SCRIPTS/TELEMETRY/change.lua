-- ========== 12页面自动轮播（修复版）==========

local currentIdx = 0
local lastSwitchTime = 0
local INTERVAL = 5000

-- 页面标题
local titles = {
    [0] = "ALLSTOP",
    [1] = "RELOCATE", 
    [2] = "SET_KFS",
    [3] = "SET_SPEAR",
    [4] = "MANUAL_CHASSIS",
    [5] = "MANUAL_ARM",
    [6] = "MANUAL_WEAPONA",
    [7] = "MODE:MANUAL_WEAPONB",
    [8] = "AUTO_NONE",
    [9] = "AUTO_ARM",
    [10] = "AUTO_WEAPON_A",
    [11] = "AUTO_WEAPON_B",
}

-- 格式化float保留2位小数
local function f2(v) 
    return string.format("%.2f", v) 
end

-- 获取数据（测试用模拟值，实际换成getValue）
local function getLocate() 
    return 12.34, 56.78, 90.12 
end
local function getKFS() 
    return 100, 200 
end
local function getSpear() 
    return 1, 0 
end
local function getJoint() 
    return 45.67, 12.34, 0.00, 90.00 
end
local function getSUCKER() 
    return 1 
end
local function getClaw() 
    return 0 
end
local function getIsStart() 
    return 1 
end

-- 页面0: Locate + KFS + Spear
local function page0()
    local a, b, c = getLocate()
    local d, e = getKFS()
    local f, g = getSpear()
    lcd.drawText(0, 18, "Locate:" .. f2(a) .. " " .. f2(b) .. " " .. f2(c), SMLSIZE)
    lcd.drawText(0, 32, "KFS:" .. d .. " " .. e, SMLSIZE)
    lcd.drawText(0, 46, "Spear:" .. f .. " " .. g, SMLSIZE)
end

-- 页面1: Locate + KFS + Spear
local function page1()
    local a, b, c = getLocate()
    local d, e = getKFS()
    local f, g = getSpear()
    lcd.drawText(0, 18, "Locate:" .. f2(a) .. " " .. f2(b) .. " " .. f2(c), SMLSIZE)
    lcd.drawText(0, 32, "KFS:" .. d .. " " .. e, SMLSIZE)
    lcd.drawText(0, 46, "Spear:" .. f .. " " .. g, SMLSIZE)
end

-- 页面2: Locate + KFS + Spear
local function page2()
    local a, b, c = getLocate()
    local d, e = getKFS()
    local f, g = getSpear()
    lcd.drawText(0, 18, "Locate:" .. f2(a) .. " " .. f2(b) .. " " .. f2(c), SMLSIZE)
    lcd.drawText(0, 32, "KFS:" .. d .. " " .. e, SMLSIZE)
    lcd.drawText(0, 46, "Spear:" .. f .. " " .. g, SMLSIZE)
end

-- 页面3: Locate + KFS + Spear
local function page3()
    local a, b, c = getLocate()
    local d, e = getKFS()
    local f, g = getSpear()
    lcd.drawText(0, 18, "Locate:" .. f2(a) .. " " .. f2(b) .. " " .. f2(c), SMLSIZE)
    lcd.drawText(0, 32, "KFS:" .. d .. " " .. e, SMLSIZE)
    lcd.drawText(0, 46, "Spear:" .. f .. " " .. g, SMLSIZE)
end

-- 页面4: 只有Locate
local function page4()
    local a, b, c = getLocate()
    lcd.drawText(0, 24, "Locate:", MIDSIZE)
    lcd.drawText(0, 42, f2(a) .. " " .. f2(b) .. " " .. f2(c), MIDSIZE)
end

-- 页面5: Locate + Joint + SUCKER
local function page5()
    local a, b, c = getLocate()
    local d, e, f, g = getJoint()
    local h = getSUCKER()
    lcd.drawText(0, 16, "Locate:" .. f2(a) .. " " .. f2(b), SMLSIZE)
    lcd.drawText(0, 28, "Joint:" .. f2(d) .. " " .. f2(e), SMLSIZE)
    lcd.drawText(0, 40, "       " .. f2(f) .. " " .. f2(g), SMLSIZE)
    lcd.drawText(0, 52, "SUCKER:" .. h, SMLSIZE)
end

-- 页面6: Locate + Joint + Claw
local function page6()
    local a, b, c = getLocate()
    local d, e, f, g = getJoint()
    local h = getClaw()
    lcd.drawText(0, 16, "Locate:" .. f2(a) .. " " .. f2(b), SMLSIZE)
    lcd.drawText(0, 28, "Joint:" .. f2(d) .. " " .. f2(e), SMLSIZE)
    lcd.drawText(0, 40, "      " .. f2(f) .. " " .. f2(g), SMLSIZE)
    lcd.drawText(0, 52, "Claw:" .. h, SMLSIZE)
end

-- 页面7: Locate + Joint + Claw
local function page7()
    local a, b, c = getLocate()
    local d, e, f, g = getJoint()
    local h = getClaw()
    lcd.drawText(0, 16, "Locate:" .. f2(a) .. " " .. f2(b), SMLSIZE)
    lcd.drawText(0, 28, "Joint:" .. f2(d) .. " " .. f2(e), SMLSIZE)
    lcd.drawText(0, 40, "      " .. f2(f) .. " " .. f2(g), SMLSIZE)
    lcd.drawText(0, 52, "Claw:" .. h, SMLSIZE)
end

-- 页面8: Locate + KFS + Spear
local function page8()
    local a, b, c = getLocate()
    local d, e = getKFS()
    local f, g = getSpear()
    lcd.drawText(0, 18, "Locate:" .. f2(a) .. " " .. f2(b) .. " " .. f2(c), SMLSIZE)
    lcd.drawText(0, 32, "KFS:" .. d .. " " .. e, SMLSIZE)
    lcd.drawText(0, 46, "Spear:" .. f .. " " .. g, SMLSIZE)
end

-- 页面9: Locate + KFS + isStart
local function page9()
    local a, b, c = getLocate()
    local d, e = getKFS()
    local f = getIsStart()
    lcd.drawText(0, 18, "Locate:" .. f2(a) .. " " .. f2(b) .. " " .. f2(c), SMLSIZE)
    lcd.drawText(0, 32, "KFS:" .. d .. " " .. e, SMLSIZE)
    lcd.drawText(0, 46, "isStart:" .. f, SMLSIZE)
end

-- 页面10: Locate + Spear + isStart
local function page10()
    local a, b, c = getLocate()
    local d, e = getSpear()
    local f = getIsStart()
    lcd.drawText(0, 18, "Locate:" .. f2(a) .. " " .. f2(b) .. " " .. f2(c), SMLSIZE)
    lcd.drawText(0, 32, "Spear:" .. d .. " " .. e, SMLSIZE)
    lcd.drawText(0, 46, "isStart:" .. f, SMLSIZE)
end

-- 页面11: Locate + Spear + isStart
local function page11()
    local a, b, c = getLocate()
    local d, e = getSpear()
    local f = getIsStart()
    lcd.drawText(0, 18, "Locate:" .. f2(a) .. " " .. f2(b) .. " " .. f2(c), SMLSIZE)
    lcd.drawText(0, 32, "Spear:" .. d .. " " .. e, SMLSIZE)
    lcd.drawText(0, 46, "isStart:" .. f, SMLSIZE)
end

-- 页面函数表
local pages = {page0, page1, page2, page3, page4, page5, page6, page7, page8, page9, page10, page11}

local function run(event)
    local now = getTime() * 10
    
    -- 5秒切换
    if now - lastSwitchTime >= INTERVAL then
        currentIdx = currentIdx + 1
        if currentIdx > 11 then
            currentIdx = 0
        end
        lastSwitchTime = now
        lcd.clear()
    end
    
    -- 绘制标题
    lcd.drawText(64, 0, titles[currentIdx], SMLSIZE + CENTER + INVERS)
    
    -- 绘制页面内容（注意Lua索引从1开始，currentIdx是0-11，所以+1）
    local pageFunc = pages[currentIdx + 1]
    if pageFunc then
        pageFunc()
    end
    
    -- 右下角倒计时
    local remain = math.ceil((INTERVAL - (now - lastSwitchTime)) / 1000)
    lcd.drawText(126, 56, remain .. "s", SMLSIZE + RIGHT)
    
    return 0
end

return { run = run }