-- Logging Example
-- This script demonstrates comprehensive logging capabilities

local log_file = nil
local log_enabled = false

-- Initialize logging
function start_logging(filename)
    filename = filename or "xterm_session.log"
    log_enabled = true
    xterm.utils.log("Logging started to: " .. filename)
    
    -- Note: In a real implementation, this would open a file
    -- For this demo, we'll just use the debug output
    xterm.terminal.write("Logging enabled (debug mode)\n")
end

function stop_logging()
    log_enabled = false
    xterm.utils.log("Logging stopped")
    xterm.terminal.write("Logging disabled\n")
end

-- Hook to log all character input
xterm.hooks.register("char_pre", function(char, attrs)
    if log_enabled and char > 32 and char < 127 then  -- Printable characters
        xterm.utils.log("CHAR: " .. string.char(char))
    end
end)

-- Hook to log key presses
xterm.hooks.register("key_press", function(keysym, state)
    if log_enabled then
        xterm.utils.log("KEY: keysym=" .. keysym .. " state=" .. state)
    end
end)

-- Quick toggle with F11
xterm.hooks.register("key_press", function(keysym, state)
    if keysym == 65482 then  -- F11
        if log_enabled then
            stop_logging()
        else
            start_logging()
        end
        return true
    end
    return false
end)

print("Logger script loaded. Use start_logging([filename]), stop_logging(), or press F11 to toggle")