# Lua Scripting Integration Plan for XTerm

## Overview

This document outlines a comprehensive plan to integrate Lua scripting capabilities into XTerm, allowing users to customize behavior, automate tasks, and extend functionality through Lua scripts.

## Project Analysis

XTerm is a mature X11 terminal emulator written in C, using the X Toolkit Intrinsics (Xt) and Athena Widget Set (Xaw). Key components identified:

- **charproc.c**: Main character processing and VT102 emulation
- **input.c**: Keyboard and mouse input handling
- **menu.c**: Context menu system
- **main.c**: Application initialization and main loop
- **screen.c**: Terminal screen management
- **util.c**: Utility functions

## Architecture Design

### 1. Lua Integration Points

#### Primary Integration Points:
- **Character Processing Hooks**: Pre/post character processing callbacks
- **Input Event Hooks**: Keyboard and mouse event interception
- **Menu System**: Custom menu items and actions
- **Terminal Events**: Window resize, focus, escape sequence handling
- **Configuration**: Runtime configuration through Lua scripts

#### Secondary Integration Points:
- **Status Line**: Custom status information
- **Color Schemes**: Dynamic color management
- **Font Management**: Runtime font switching
- **Scrollback**: Custom scrollback processing

### 2. Core Lua Subsystem

#### File Structure:
```
lua/
├── lua_api.c          # Main Lua C API bindings
├── lua_api.h          # Header declarations
├── lua_hooks.c        # Hook management system
├── lua_config.c       # Configuration through Lua
├── lua_utils.c        # Utility functions
└── scripts/
    ├── init.lua       # Default initialization script
    ├── examples/      # Example scripts
    └── user/          # User custom scripts
```

#### Core Components:

1. **Lua State Management**
   - Initialize Lua interpreter on startup
   - Manage script loading and execution
   - Handle error reporting and recovery
   - Script reloading without restart

2. **Hook System**
   - Pre/post character processing hooks
   - Input event hooks (key press, mouse events)
   - Terminal state change hooks
   - Menu action hooks

3. **API Bindings**
   - Terminal manipulation functions
   - Configuration access
   - X11 integration (limited)
   - File system operations (sandboxed)

## Implementation Plan

### Phase 1: Basic Infrastructure (Week 1-2)

#### 1.1 Build System Integration
- Add Lua detection to configure.in
- Add conditional compilation flags
- Update Makefile.in for Lua sources

#### 1.2 Core Lua Integration
- Create basic Lua state initialization
- Add Lua source files to project
- Implement basic error handling
- Add compile-time option (--enable-lua)

#### 1.3 Basic API Framework
```c
// lua_api.h
#ifdef OPT_LUA_SCRIPTING
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct {
    lua_State *L;
    int initialized;
    char *script_dir;
    char *user_script;
} LuaContext;

extern LuaContext *lua_ctx;

int lua_xterm_init(void);
void lua_xterm_cleanup(void);
int lua_xterm_load_script(const char *filename);
void lua_xterm_call_hook(const char *hook_name, ...);
#endif
```

### Phase 2: Hook System (Week 3-4)

#### 2.1 Character Processing Hooks
- Pre-character processing hook in charproc.c
- Post-character processing hook
- Escape sequence interception

#### 2.2 Input Event Hooks
- Keyboard event hooks in input.c
- Mouse event hooks
- Special key combination handling

#### 2.3 Hook Registration System
```lua
-- Example Lua hook registration
xterm.hooks.register("char_pre", function(char, attrs)
    -- Process character before display
    return char, attrs  -- Return modified or original
end)

xterm.hooks.register("key_press", function(keysym, state)
    if keysym == "F12" then
        xterm.menu.show_custom()
        return true  -- Event handled
    end
    return false  -- Pass through
end)
```

### Phase 3: API Development (Week 5-6)

#### 3.1 Terminal Control API
```lua
-- Terminal manipulation
xterm.terminal.write("Hello from Lua!")
xterm.terminal.clear()
xterm.terminal.set_title("Custom Title")
xterm.terminal.bell()

-- Screen manipulation
xterm.screen.scroll_up(5)
xterm.screen.goto(row, col)
xterm.screen.save_cursor()
xterm.screen.restore_cursor()
```

#### 3.2 Configuration API
```lua
-- Configuration access
xterm.config.set("background", "black")
xterm.config.set("foreground", "white")
local font = xterm.config.get("font")

-- Resource management
xterm.resources.set("*VT100.geometry", "132x50")
```

#### 3.3 Menu System API
```lua
-- Custom menu items
xterm.menu.add_item("Scripts", {
    {"Reload Config", function() xterm.lua.reload() end},
    {"Toggle Logging", toggle_logging},
    {"---", nil},  -- Separator
    {"Exit", function() xterm.quit() end}
})
```

### Phase 4: Advanced Features (Week 7-8)

#### 4.1 Script Management
- Script auto-loading from ~/.xterm/scripts/
- Script reloading without restart
- Script dependency management
- Error recovery and logging

#### 4.2 Event System
```lua
-- Event handling
xterm.events.on("resize", function(width, height)
    print("Terminal resized to " .. width .. "x" .. height)
end)

xterm.events.on("focus_in", function()
    xterm.terminal.set_cursor_color("red")
end)

xterm.events.on("focus_out", function()
    xterm.terminal.set_cursor_color("default")
end)
```

#### 4.3 Utility Functions
```lua
-- Utility functions
xterm.utils.log("Debug message")
xterm.utils.notify("Notification text")
local time = xterm.utils.timestamp()
local file_exists = xterm.utils.file_exists("/path/to/file")
```

### Phase 5: Integration and Testing (Week 9-10)

#### 5.1 Integration Points
- Modify charproc.c to call Lua hooks
- Update input.c for keyboard/mouse hooks
- Enhance menu.c for custom menu items
- Add configuration loading in main.c

#### 5.2 Example Scripts
```lua
-- ~/.xterm/scripts/init.lua
-- Custom initialization script

-- Set up custom key bindings  
xterm.hooks.register("key_press", function(keysym, state)
    if state.ctrl and keysym == "t" then
        xterm.terminal.write(os.date())
        return true
    end
    return false
end)

-- Custom color scheme based on time
local function update_colors()
    local hour = tonumber(os.date("%H"))
    if hour >= 18 or hour < 6 then
        -- Night mode
        xterm.config.set("background", "#001122")
        xterm.config.set("foreground", "#ccddee")
    else
        -- Day mode  
        xterm.config.set("background", "#ffffff")
        xterm.config.set("foreground", "#000000")
    end
end

xterm.events.on("startup", update_colors)
xterm.events.on("focus_in", update_colors)

-- Custom status line
xterm.hooks.register("status_update", function()
    local load = xterm.utils.system("uptime | awk '{print $10}'")
    return "Load: " .. load
end)
```

## Build System Integration

### Configure Script Changes
```bash
# configure.in additions
AC_ARG_ENABLE(lua-scripting,
    [  --enable-lua-scripting  enable Lua scripting support],
    [enable_lua=$enableval],
    [enable_lua=no])

if test "$enable_lua" = "yes"; then
    AC_CHECK_LIB(lua, lua_newstate, 
        [LIBS="$LIBS -llua"; AC_DEFINE(OPT_LUA_SCRIPTING,1)],
        [AC_MSG_ERROR([Lua library not found])])
    AC_CHECK_HEADERS([lua.h lualib.h lauxlib.h])
fi
```

### Makefile Changes
```makefile
# Conditional compilation
LUASRCS = lua_api.c lua_hooks.c lua_config.c lua_utils.c
LUAOBJS = lua_api.o lua_hooks.o lua_config.o lua_utils.o

# Add to main object list if enabled
OBJS = ... $(LUAOBJS)
```

## Security Considerations

1. **Sandbox Limitations**
   - Restrict file system access to specific directories
   - Disable dangerous Lua functions (os.execute, io.popen)
   - Limit memory usage and execution time

2. **Script Validation**
   - Syntax checking before execution
   - Runtime error handling and recovery
   - Safe script reloading mechanisms

3. **User Permissions**
   - Scripts run with xterm process permissions
   - No privilege escalation through scripts
   - Clear documentation of security implications

## Configuration Options

### Command Line Options
```bash
xterm -lua-script ~/.xterm/custom.lua
xterm -lua-disable  # Disable Lua even if compiled in
xterm -lua-debug    # Enable Lua debugging output
```

### X Resources
```
XTerm*luaScript: ~/.xterm/init.lua
XTerm*luaEnable: true
XTerm*luaDebug: false
```

## Documentation Plan

1. **User Documentation**
   - Lua scripting tutorial
   - API reference manual
   - Example script collection
   - Migration guide for existing configurations

2. **Developer Documentation**
   - C API integration guide
   - Hook system documentation
   - Building with Lua support
   - Contributing new API functions

## Testing Strategy

1. **Unit Tests**
   - Lua state management
   - Hook system functionality
   - API binding correctness
   - Error handling

2. **Integration Tests**
   - Script loading and execution
   - Event handling
   - Configuration changes
   - Memory leak detection

3. **Example Scripts**
   - Basic functionality demonstration
   - Common use case implementations
   - Performance testing scripts
   - Regression test suite

## Migration Path

For existing xterm users:
1. Lua support is optional (compile-time flag)
2. Zero impact when disabled
3. Gradual migration of X resources to Lua
4. Compatibility layer for common configurations

## Timeline Summary

- **Weeks 1-2**: Basic infrastructure and build system
- **Weeks 3-4**: Hook system implementation
- **Weeks 5-6**: Core API development
- **Weeks 7-8**: Advanced features and script management
- **Weeks 9-10**: Integration, testing, and documentation

## Success Metrics

1. Successful compilation with and without Lua support
2. Zero performance impact when Lua is disabled
3. Minimal performance impact with simple scripts
4. Comprehensive API coverage for common use cases
5. Stable script execution with proper error handling
6. Complete documentation and examples

This plan provides a solid foundation for adding Lua scripting capabilities to XTerm while maintaining the stability and performance characteristics that users expect.