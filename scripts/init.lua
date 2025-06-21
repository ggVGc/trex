-- XTerm Lua Scripting - Default Initialization Script
-- This script demonstrates the Lua scripting capabilities of xterm

print("XTerm Lua scripting initialized!")

-- Example 1: Custom key bindings
xterm.hooks.register("key_press", function(keysym, state)
    -- Ctrl+T: Insert current timestamp
    if keysym == 116 and (state & 4) ~= 0 then  -- 't' key with Ctrl
        local timestamp = os.time()
        local timestr = os.date("%Y-%m-%d %H:%M:%S", timestamp)
        xterm.terminal.write(timestr)
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

-- Print a welcome message
xterm.terminal.write("\n--- XTerm Lua Scripting Active ---\n")
xterm.terminal.write("Press Ctrl+T for timestamp, F12 for hello message\n")
xterm.terminal.write("Type 'insert_date()' in the shell to see Lua integration\n\n")