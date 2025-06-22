-- Minimal test script for command mode

-- Test entering command mode
print("Testing command mode...")

-- Check if we can enter command mode
xterm.utils.enter_command_mode()
print("Command mode entered: " .. tostring(xterm.utils.is_command_mode()))

-- Exit command mode
xterm.utils.exit_command_mode()
print("Command mode exited: " .. tostring(xterm.utils.is_command_mode()))

-- Register a simple key binding
xterm.hooks.register("key_press", function(keysym, state)
    -- F12 key (XK_F12 = 0xffc9)
    if keysym == 0xffc9 then
        print("F12 pressed - entering command mode")
        xterm.utils.enter_command_mode()
        return true
    end
    return false
end)

print("Command mode test loaded. Press F12 to enter command mode.")