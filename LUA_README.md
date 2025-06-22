# XTerm Lua Scripting Implementation

This implementation adds comprehensive Lua scripting support to XTerm, allowing users to customize behavior, automate tasks, and extend functionality through Lua scripts.

## Build Instructions

1. **Prerequisites**: Install Lua development libraries
   ```bash
   # Ubuntu/Debian
   sudo apt-get install liblua5.3-dev
   
   # RHEL/CentOS/Fedora
   sudo yum install lua-devel
   # or
   sudo dnf install lua-devel
   
   # macOS with Homebrew
   brew install lua
   ```

2. **Configure with Lua support**:
   ```bash
   ./configure --enable-lua-scripting
   ```

3. **Build**:
   ```bash
   make
   ```

## File Structure

### Core Implementation Files
- `lua_api.h` - Main header with API declarations
- `lua_api.c` - Core Lua state management and initialization  
- `lua_hooks.c` - Hook system for event handling
- `lua_config.c` - Terminal control and configuration APIs
- `lua_utils.c` - Utility functions and event system

### Integration Points
- `main.c` - Lua initialization and cleanup
- `charproc.c` - Character processing hooks
- `input.c` - Keyboard input hooks
- `util.c` - Character writing hooks

### Build System
- `configure.in` - Autotools configuration for Lua detection
- `Makefile.in` - Build rules for Lua source files

## Lua API Reference

### xterm.terminal
- `write(text)` - Write text to terminal
- `clear()` - Clear screen
- `set_title(title)` - Set window title
- `bell()` - Ring terminal bell
- `goto(row, col)` - Move cursor
- `save_cursor()` - Save cursor position
- `restore_cursor()` - Restore cursor position

### xterm.config
- `get(key)` - Get configuration value
- `set(key, value)` - Set configuration value
- `reset()` - Reset configuration

### xterm.hooks
- `register(hook_name, function)` - Register event hook
- `unregister(hook_name)` - Unregister hook
- `clear()` - Clear all hooks

### Hook Types
- `"char_pre"` - Before character processing
- `"char_post"` - After character processing
- `"key_press"` - Key press events
- `"key_release"` - Key release events
- `"mouse_press"` - Mouse button press
- `"mouse_release"` - Mouse button release
- `"mouse_motion"` - Mouse movement
- `"focus_in"` - Window focus gained
- `"focus_out"` - Window focus lost
- `"resize"` - Terminal resize
- `"startup"` - Terminal startup
- `"shutdown"` - Terminal shutdown

### xterm.events
- `on(event_name, function)` - Register event handler
- `off(event_name)` - Unregister event handler
- `emit(event_name, ...)` - Emit custom event

### xterm.utils
- `log(message)` - Log debug message
- `notify(message)` - Show notification
- `timestamp()` - Get current timestamp
- `file_exists(filename)` - Check if file exists
- `system(command)` - Execute system command

### xterm.menu
- `add_item(menu_name, item_name)` - Add menu item
- `remove_item(menu_name, item_name)` - Remove menu item
- `show()` - Show menu

## Usage Examples

### Basic Setup
Create `~/.xterm/scripts/init.lua`:
```lua
-- Custom key binding
xterm.hooks.register("key_press", function(keysym, state)
    if keysym == 116 and (state & 4) ~= 0 then  -- Ctrl+T
        xterm.terminal.write(os.date())
        return true  -- Event handled
    end
    return false  -- Pass through
end)

-- Startup message
xterm.events.on("startup", function()
    xterm.terminal.write("Welcome to XTerm with Lua!\n")
end)
```

### Advanced Example - Text Expansion
```lua
local expansions = {
    ["@date"] = function() return os.date("%Y-%m-%d") end,
    ["@time"] = function() return os.date("%H:%M:%S") end
}

local buffer = ""
xterm.hooks.register("char_pre", function(char, attrs)
    if char >= 32 and char <= 126 then
        buffer = buffer .. string.char(char)
        for trigger, func in pairs(expansions) do
            if string.find(buffer, trigger .. "$") then
                -- Simulate backspaces to remove trigger
                for i = 1, #trigger do
                    xterm.terminal.write("\b \b")
                end
                xterm.terminal.write(func())
                buffer = ""
                return false
            end
        end
    end
    return false
end)
```

## Command Line Options

- `--enable-lua-scripting` - Configure flag to enable Lua support
- `-lua-script <file>` - Load specific Lua script (planned)
- `-lua-disable` - Disable Lua even if compiled in (planned)
- `-lua-debug` - Enable Lua debugging output (planned)

## Script Locations

1. System-wide: `/usr/share/xterm/scripts/`
2. User scripts: `~/.xterm/scripts/`
3. Default init script: `~/.xterm/scripts/init.lua`

## Security Considerations

The Lua environment is sandboxed with the following restrictions:
- No `os.execute()` or `io.popen()` for arbitrary command execution
- Limited file system access
- No network access
- Script size limits (1MB maximum)
- Execution timeouts to prevent infinite loops

Safe alternatives are provided:
- `xterm.utils.system()` for controlled command execution
- `xterm.utils.file_exists()` for file checking

## Error Handling

- Lua errors are logged to stderr
- Scripts continue running after recoverable errors
- Scripting is automatically disabled after 10 consecutive errors
- Script reloading doesn't require restarting xterm

## Performance

- Minimal overhead when Lua is disabled at compile time
- Hook calls are optimized for frequently used events
- Script state is persistent across terminal operations
- Automatic script reloading when files change

## Example Scripts

See the `scripts/examples/` directory for:
- `colors.lua` - Dynamic color theme management
- `logger.lua` - Comprehensive session logging
- `productivity.lua` - Text expansion and shortcuts

## Debugging

Enable debug output:
```bash
xterm -lua-debug 2> lua_debug.log
```

Common debug messages:
- Script loading and execution
- Hook registration and calls
- Error conditions and recovery
- Performance metrics

## Future Enhancements

Planned features:
- Menu system integration
- Custom widgets and overlays
- Network scripting capabilities
- Plugin system for script sharing
- Integration with terminal multiplexers

## Contributing

To add new API functions:
1. Add function declaration to `lua_api.h`
2. Implement in appropriate source file
3. Register in the corresponding `luaopen_xterm_*` function
4. Add documentation and examples
5. Update test suite

## Compatibility

- Lua 5.1, 5.2, 5.3, and 5.4 supported
- Requires XTerm 400+ with this patch applied
- POSIX-compliant systems
- X11 environment required

This implementation provides a solid foundation for XTerm customization while maintaining the stability and security expected from a system terminal emulator.