# User Script Loading

The trex runtime automatically loads user scripts from `$HOME/.config/trex` on startup and adds this directory to package.path.

## User Configuration Directory

User scripts are located in: `~/.config/trex/`

### Main Configuration
- `~/.config/trex/init.lua` - Main user configuration (loaded first if it exists)

### Module Support
The `~/.config/trex/` directory is added to `package.path`, so you can:
- Create additional `.lua` files in `~/.config/trex/`
- Use `require('modulename')` to load them from your init.lua
- Organize code into modules using standard Lua patterns

## Trex API

User scripts have access to the global `trex` object with these functions:

### Core Modules
- `trex.utils` - Utility functions (logging, etc.)
- `trex.advanced` - Advanced functionality

### Terminal Functions
- `trex.message(text)` - Display modal message
- `trex.write(text)` - Write to terminal
- `trex.get_text()` - Get current terminal text

### System Functions
- `trex.reload_scripts()` - Reload all user scripts
- `trex.version` - Runtime version
- `trex.loaded_scripts` - List of loaded scripts

## Example User Configuration

### Main Configuration (`~/.config/trex/init.lua`)
```lua
-- ~/.config/trex/init.lua
print("Loading user configuration...")

-- Load trex API using normal require
local trex = require("trex")

trex.utils.log("User configuration loaded!")

-- Load additional user modules from ~/.config/trex/
local myutils = require("myutils")
local keybinds = require("keybinds")

-- Add custom functions
local user_functions = {
    hello = function()
        trex.message("Hello from user script!")
    end,
    
    timestamp = function()
        local date = os.date("%Y-%m-%d %H:%M:%S")
        trex.write(date)
    end,
    
    -- Use functions from user modules
    my_function = myutils.do_something,
    setup_keys = keybinds.setup
}

trex.utils.log("User configuration complete!")

return user_functions
```

### User Module (`~/.config/trex/myutils.lua`)
```lua
-- ~/.config/trex/myutils.lua
local myutils = {}

function myutils.do_something()
    trex.message("Function from user module!")
end

function myutils.format_timestamp()
    return os.date("%Y-%m-%d %H:%M:%S")
end

return myutils
```

## Error Handling

- Script loading errors are logged but don't stop startup
- Missing directories/files are silently ignored
- Each script runs in protected mode (pcall)
- Check terminal output for loading status and errors

## Script Development Tips

- Use `trex.utils.log()` for debugging output
- Always use `local trex = require("trex")` to access the API
- Use `trex.reload_scripts()` to test changes without restart
- Place commonly used functions in `init.lua`
- Create additional modules in `~/.config/trex/` and require them
- The system loads `runtime/trex_init.lua` on startup, which sets up user script loading