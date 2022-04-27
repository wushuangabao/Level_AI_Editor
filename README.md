# Level_AI_Editor

> 这是一个游戏（关卡）脚本编辑器

## 操作

类似魔兽争霸3世界编辑器中的触发（Trigger）编辑器，用户可以添加变量、事件、条件和各种动作，进行可视化编辑。

## 配置

config目录中的配置文件，用于定义事件类型、变量类型、函数信息。

## 功能

以关卡为单位生成lua脚本，类似这样：
```
local levelTable = {}

-- 自定义动作
local function CustomAction_0(level)
    if (LevelAIHelper.AliveCount(level.flowController, 21) == 0 and LevelAIHelper.AliveCount(level.flowController, 1) == 0) then
    else
    end
end

local function Check_0(event, level)
    if (event.round == 20 or event.round == 1) then
        return 1
    else
        return 0
    end
end

local function Execute_0(event, level)
    LevelAIHelper.LevelFailed(level.flowController)
end

local function Check_1(event, level)
    if (LevelAIHelper.AliveCount(level.flowController, 1) == 0) then
        return 1
    else
        return 0
    end
end

local function Execute_1(event, level)
    LevelAIHelper.LevelFailed(level.flowController)
end

local function Check_2(event, level)
    if (LevelAIHelper.AliveCount(level.flowController, 21) == 0 and LevelAIHelper.AliveCount(level.flowController, 1) == 0) then
        return 1
    else
        return 0
    end
end

local function Execute_2(event, level)
    level.g_var_0 = LevelAIHelper.Add(level.flowController, event.dataId, event.reputationId)
    LevelAIHelper.LevelSuccess(level.flowController)
end

levelTable.EventFunc = {
    [E_LEVEL_TRIGGER_TYPE.BEFORE_ROUND_END] = {
        {   --关卡失败1
            check = Check_0,
            call = Execute_0,
            enable = 1
        },
    },
    [E_LEVEL_TRIGGER_TYPE.DEAD] = {
        {   --关卡失败2
            check = Check_1,
            call = Execute_1,
            enable = 1
        },
        {   --关卡胜利
            check = Check_2,
            call = Execute_2,
            enable = 1
        },
    },
}

function levelTable:init(flowController)
    self.flowController = flowController
    self.g_var_0 = 0    	-- 完成	number
    self.EventFunc[E_LEVEL_TRIGGER_TYPE.BEFORE_ROUND_END][1].enable = 1
    self.EventFunc[E_LEVEL_TRIGGER_TYPE.DEAD][1].enable = 1
    self.EventFunc[E_LEVEL_TRIGGER_TYPE.DEAD][2].enable = 1
end

function levelTable:Execute(event)
    local ret = false
    if self.EventFunc[event.id] ~= nil then
        for _, func in ipairs (self.EventFunc[event.id]) do
            if func and func.check and func.call and func.enable and func.enable == 1 and func.check(event, self) == 1 then
                func.call(event, self)
                ret = true
                -- break
            end
        end
    end
    return ret
end

return levelTable

```

## 开发环境

在 Windows 上使用 Qt5.10.0 开发，编译器用的 MinGW。