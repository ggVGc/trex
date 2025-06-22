-- Enable command mode display
-- This script adds visual display to the Lua command mode

-- Variable to track if we're in command mode
local in_command_mode = false
local command_buffer = ""

-- Register command mode hooks for visual display
xterm.hooks.register("command_mode", function(action, data)
    if action == "enter" then
        in_command_mode = true
        command_buffer = ""
        -- Show prompt on new line
        xterm.terminal.write("\n\027[7mLua> \027[0m")
        
    elseif action == "exit" then
        in_command_mode = false
        -- Clear the line if empty command
        if command_buffer == "" then
            xterm.terminal.write("\r\027[K")  -- Carriage return and clear line
        else
            xterm.terminal.write("\n")  -- New line after command
        end
        command_buffer = ""
        
    elseif action == "display" then
        -- Update display with current command buffer
        command_buffer = data or ""
        -- Clear current line and redraw
        xterm.terminal.write("\r\027[K")  -- Carriage return and clear line
        xterm.terminal.write("\027[7mLua> \027[0m" .. command_buffer)
    end
end)

-- Also register Ctrl+L as an alternative to enter command mode
xterm.hooks.register("key_press", function(keysym, state)
    -- Ctrl+L (108 is 'l')
    if keysym == 108 and (state & 4) ~= 0 then
        xterm.utils.log("Entering Lua command mode via Ctrl+L")
        xterm.utils.enter_command_mode()
        return true
    end
    return false
end)

print("Command mode display enabled!")
print("Press Ctrl+T or Ctrl+L to enter Lua command mode")
print("You will see a 'Lua> ' prompt where you can type Lua commands")