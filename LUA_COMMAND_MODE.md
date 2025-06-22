# Lua Command Mode for XTerm

This document describes the new Lua command mode feature added to XTerm's Lua scripting API.

## Overview

The Lua command mode allows users to enter and execute Lua commands directly in the terminal, similar to Vim's command mode. When activated, users can type Lua code that will be executed in the context of the XTerm Lua environment.

## Usage

### Entering Command Mode

By default, you can enter command mode by pressing `Ctrl+:` (this can be customized in your Lua scripts).

### Command Mode Keys

- **Enter**: Execute the current command
- **Escape**: Exit command mode without executing
- **Backspace/Delete**: Delete the previous character
- **Any printable character**: Add to the command buffer

### Example Commands

Once in command mode, you can execute any valid Lua code:

```lua
-- Print a message
print("Hello from Lua!")

-- Ring the terminal bell
xterm.terminal.bell()

-- Change the terminal title
xterm.terminal.set_title("New Title")

-- Change background color
xterm.config.set("background", "#000000")

-- Access any Lua function or variable
xterm.utils.log("Command executed")
```

## API Functions

### From Lua Scripts

```lua
-- Enter command mode programmatically
xterm.utils.enter_command_mode()

-- Exit command mode
xterm.utils.exit_command_mode()

-- Check if in command mode
local in_cmd_mode = xterm.utils.is_command_mode()
```

### Hooks

The command mode system provides hooks for customization:

```lua
xterm.hooks.register("command_mode", function(action, data)
    if action == "enter" then
        -- Called when entering command mode
    elseif action == "exit" then
        -- Called when exiting command mode
    elseif action == "display" then
        -- Called when the command buffer changes
        -- data contains the current command text
    end
end)
```

## Example Configuration

See `scripts/examples/command_mode.lua` for a complete example that:
- Sets up Ctrl+: to enter command mode
- Displays the command prompt at the bottom of the screen
- Shows the command as you type

## Implementation Details

The command mode is implemented in:
- `lua_api.h/c`: Core command mode functions
- `input.c`: Keyboard input handling when in command mode
- `lua_utils.c`: Lua-accessible functions for command mode control

The command buffer dynamically grows as needed and is cleared when exiting command mode.