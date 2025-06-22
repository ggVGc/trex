-- XTerm Lua Scripting - Default Initialization Script
-- This script demonstrates the Lua scripting capabilities of xterm

print("XTerm Lua scripting initialized!")

-- Example 1: Custom key bindings
xterm.hooks.register("key_press", function(keysym, state)
    -- Ctrl+T: Enter Lua command mode
    if keysym == 116 and (state & 4) ~= 0 then  -- 't' key with Ctrl
        xterm.utils.log("Entering Lua command mode")
        xterm.utils.enter_command_mode()
        return true  -- Event handled, don't pass through
    end
    
    -- F12: Show a custom message
    if keysym == 65483 then  -- F12 key
        xterm.terminal.write("\nHello from Lua scripting!\n")
        xterm.terminal.bell()
        return true
    end
    
    return false  -- Event not handled, pass through
end)

-- Example 2: Character processing hook
local char_count = 0
xterm.hooks.register("char_post", function(char, attrs)
    char_count = char_count + 1
    -- Every 100 characters, log a message
    if char_count % 100 == 0 then
        xterm.utils.log("Processed " .. char_count .. " characters")
    end
end)

-- Example 3: Terminal events
xterm.events.on("startup", function()
    xterm.utils.log("Terminal started with Lua scripting enabled")
    xterm.terminal.set_title("XTerm with Lua")
end)

-- Example 4: Simple utility function
function insert_date()
    local date = os.date("%Y-%m-%d")
    xterm.terminal.write(date)
end

-- Example 5: Configuration monitoring
local original_title = xterm.config.get("title") or "xterm"

-- Log some information about the environment
xterm.utils.log("Lua scripting version loaded")
xterm.utils.log("Original title: " .. original_title)

-- Register command mode display hooks
xterm.hooks.register("command_mode", function(action, data)
    if action == "enter" then
        -- Show prompt on new line
        xterm.terminal.write("\n\027[7mLua> \027[0m")
    elseif action == "exit" then
        -- Clear the line if empty command
        if (data or "") == "" then
            xterm.terminal.write("\r\027[K")  -- Carriage return and clear line
        else
            xterm.terminal.write("\n")  -- New line after command
        end
    elseif action == "display" then
        -- Update display with current command buffer
        xterm.terminal.write("\r\027[K")  -- Carriage return and clear line
        xterm.terminal.write("\027[7mLua> \027[0m" .. (data or ""))
    end
end)

-- Print a welcome message
xterm.terminal.write("\n--- XTerm Lua Scripting Active ---\n")
xterm.terminal.write("Press Ctrl+T to enter Lua command mode, F12 for hello message\n")
xterm.terminal.write("In command mode: type Lua code, press Enter to execute, Escape to cancel\n\n")