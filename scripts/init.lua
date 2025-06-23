-- XTerm Lua Scripting - Default Initialization Script
-- This script demonstrates the Lua scripting capabilities of xterm

print("User init.lua loaded")

function log(arg)
  xterm.utils.log(arg)
  print(arg)
end

-- Example 1: Custom key bindings
xterm.hooks.register("key_press", function(keysym, state)
    -- Ctrl+T: Enter Lua command mode
    if keysym == 116 and (state & 4) ~= 0 then  -- 't' key with Ctrl
        log("Entering Lua command mode")
        xterm.utils.enter_command_mode()
        return true  -- Event handled, don't pass through
    end
    
    -- F12: Show a custom message
    if keysym == 65483 then  -- F12 key
        xterm.terminal.message("\nHello from Lua scripting!\n")
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
        log("Processed " .. char_count .. " characters")
    end
end)

-- Example 3: Terminal events
--[[
xterm.events.on("startup", function()
    log("Terminal started with Lua scripting enabled")
    xterm.terminal.set_title("XTerm with Lua")
end)
--]]

-- Example 4: Simple utility function
function insert_date()
    local date = os.date("%Y-%m-%d")
    xterm.terminal.write(date)
end

-- Example 5: Configuration monitoring
local original_title = xterm.config.get("title") or "xterm"

-- Log some information about the environment
log("Lua scripting version loaded")
log("Original title: " .. original_title)

-- Command mode display is now handled in C code
-- The hooks are still available for additional customization if needed

-- Print a welcome message
xterm.terminal.message("--- XTerm Lua Scripting Active ---")
xterm.terminal.message("Press Ctrl+T to enter Lua command mode, F12 for hello message")
xterm.terminal.message("In command mode: type Lua code, press Enter to execute, Escape to cancel\n")
