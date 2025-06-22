-- Simple command mode example for xterm lua scripting
-- This demonstrates a basic command mode without visual display

-- Variable to store command feedback
local last_command = ""

-- Register a key binding to enter command mode (Ctrl+L for "Lua")
xterm.hooks.register("key_press", function(keysym, state)
    -- Check for Ctrl+L (l)
    if keysym == 108 and (state & 4) ~= 0 then  -- 108 is XK_l, 4 is ControlMask
        xterm.utils.log("Entering Lua command mode")
        xterm.utils.enter_command_mode()
        xterm.terminal.bell()  -- Audio feedback
        return true  -- Consume the event
    end
    return false
end)

-- Register command mode hooks
xterm.hooks.register("command_mode", function(action, data)
    if action == "enter" then
        -- Command mode entered
        xterm.utils.log("Command mode: ready for input")
    elseif action == "exit" then
        -- Command mode exited
        xterm.utils.log("Command mode: exited")
        if last_command ~= "" then
            xterm.terminal.write("\n[Lua command mode exited]\n")
        end
    elseif action == "display" then
        -- Update command display (data contains current command)
        last_command = data or ""
    end
end)

-- Example of executing a command when it's entered
local original_execute = lua_xterm_command_mode_execute
if original_execute then
    lua_xterm_command_mode_execute = function()
        if last_command ~= "" then
            xterm.terminal.write("\n[Executing: " .. last_command .. "]\n")
        end
        original_execute()
    end
end

xterm.utils.log("Simple command mode loaded. Press Ctrl+L to enter command mode")