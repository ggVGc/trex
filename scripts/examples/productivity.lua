-- Productivity Enhancement Script
-- This script adds various productivity features

-- Text expansion system
local expansions = {
    ["@date"] = function() return os.date("%Y-%m-%d") end,
    ["@time"] = function() return os.date("%H:%M:%S") end,
    ["@datetime"] = function() return os.date("%Y-%m-%d %H:%M:%S") end,
    ["@user"] = function() return os.getenv("USER") or "user" end,
    ["@home"] = function() return os.getenv("HOME") or "/home/user" end,
    ["@pwd"] = function() 
        local pwd = xterm.utils.system("pwd")
        return pwd or ""
    end
}

-- Simple text expansion
local buffer = ""
xterm.hooks.register("char_pre", function(char, attrs)
    if char >= 32 and char <= 126 then  -- Printable ASCII
        buffer = buffer .. string.char(char)
        
        -- Keep buffer reasonable size
        if #buffer > 50 then
            buffer = string.sub(buffer, -25)
        end
        
        -- Check for expansions
        for trigger, func in pairs(expansions) do
            if string.find(buffer, trigger .. "$") then
                -- Remove the trigger from terminal (simulate backspace)
                for i = 1, #trigger do
                    xterm.terminal.write("\b \b")  -- backspace, space, backspace
                end
                
                -- Insert the expansion
                local expansion = func()
                xterm.terminal.write(expansion)
                
                buffer = ""
                return false  -- Don't process the original character
            end
        end
    else
        -- Reset buffer on non-printable characters
        buffer = ""
    end
    
    return false
end)

-- Quick commands with Ctrl+Alt combinations
xterm.hooks.register("key_press", function(keysym, state)
    -- Ctrl+Alt pressed (state & 12 == 12)
    if (state & 12) == 12 then
        if keysym == 100 then  -- 'd' - insert date
            xterm.terminal.write(os.date("%Y-%m-%d"))
            return true
        elseif keysym == 116 then  -- 't' - insert time
            xterm.terminal.write(os.date("%H:%M:%S"))
            return true
        elseif keysym == 117 then  -- 'u' - insert username
            xterm.terminal.write(os.getenv("USER") or "user")
            return true
        elseif keysym == 99 then   -- 'c' - clear screen
            xterm.terminal.clear()
            return true
        end
    end
    
    return false
end)

-- Status line update hook
local last_update = 0
xterm.hooks.register("status_update", function()
    local now = os.time()
    if now - last_update >= 5 then  -- Update every 5 seconds
        last_update = now
        local load = xterm.utils.system("uptime | awk '{print $NF}'") or "?"
        return "Load: " .. load .. " | " .. os.date("%H:%M")
    end
    return nil
end)

-- Help function
function show_help()
    xterm.terminal.write("\n=== Productivity Features ===\n")
    xterm.terminal.write("Text Expansions:\n")
    xterm.terminal.write("  @date     -> current date\n")
    xterm.terminal.write("  @time     -> current time\n")
    xterm.terminal.write("  @datetime -> current date and time\n")
    xterm.terminal.write("  @user     -> username\n")
    xterm.terminal.write("  @home     -> home directory\n")
    xterm.terminal.write("  @pwd      -> current directory\n")
    xterm.terminal.write("\nKey Combinations (Ctrl+Alt+):\n")
    xterm.terminal.write("  d -> insert date\n")
    xterm.terminal.write("  t -> insert time\n")
    xterm.terminal.write("  u -> insert username\n")
    xterm.terminal.write("  c -> clear screen\n")
    xterm.terminal.write("\n")
end

print("Productivity script loaded. Type 'show_help()' for available features")