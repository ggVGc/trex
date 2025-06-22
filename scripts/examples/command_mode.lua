-- Command mode example for xterm lua scripting
-- This script demonstrates how to activate and use the lua command mode

-- Register a key binding to enter command mode (Ctrl+:)
xterm.hooks.register("key_press", function(keysym, state)
    -- Check for Ctrl+: (colon)
    if keysym == 58 and (state & 4) ~= 0 then  -- 58 is XK_colon, 4 is ControlMask
        xterm.utils.log("Entering Lua command mode")
        xterm.utils.enter_command_mode()
        return true  -- Consume the event
    end
    return false
end)

-- Register command mode hooks
xterm.hooks.register("command_mode", function(action, data)
    if action == "enter" then
        -- Display command mode prompt
        xterm.terminal.save_cursor()
        xterm.terminal.goto(24, 1)  -- Bottom line
        xterm.terminal.write("\027[7m:")  -- Reverse video
        xterm.terminal.write(" ")
        xterm.terminal.write("\027[0m")  -- Normal video
    elseif action == "exit" then
        -- Clear command line
        xterm.terminal.goto(24, 1)
        xterm.terminal.write("\027[K")  -- Clear to end of line
        xterm.terminal.restore_cursor()
    elseif action == "display" then
        -- Update command display
        xterm.terminal.save_cursor()
        xterm.terminal.goto(24, 1)
        xterm.terminal.write("\027[7m:")  -- Reverse video
        xterm.terminal.write(data or "")
        xterm.terminal.write(" ")
        xterm.terminal.write("\027[0m")  -- Normal video
        xterm.terminal.write("\027[K")  -- Clear rest of line
        xterm.terminal.restore_cursor()
    end
end)

-- Example commands that can be executed in command mode:
-- print("Hello from Lua!")
-- xterm.terminal.bell()
-- xterm.terminal.set_title("New Title")
-- xterm.config.set("background", "#000000")

xterm.utils.log("Command mode example loaded. Press Ctrl+: to enter command mode")