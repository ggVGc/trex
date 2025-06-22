/* $XTermId: lua_api.h,v 1.1 2025/06/21 00:00:00 claude Exp $ */

/*
 * Copyright 2025 by Claude (Anthropic)
 *
 *                         All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef included_lua_api_h
#define included_lua_api_h

#include <xterm.h>

#ifdef OPT_LUA_SCRIPTING

#ifdef LUA_INCLUDE_PREFIX
#include LUA_INCLUDE_PREFIX "lua.h"
#include LUA_INCLUDE_PREFIX "lualib.h"
#include LUA_INCLUDE_PREFIX "lauxlib.h"
#else
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#endif

#define LUA_SCRIPT_DIR_DEFAULT "trex_runtime"
#define LUA_INIT_SCRIPT "trex.lua"
#define LUA_MAX_SCRIPT_SIZE (1024 * 1024)  /* 1MB max script size */

typedef enum {
    LUA_HOOK_CHAR_PRE,
    LUA_HOOK_CHAR_POST,
    LUA_HOOK_KEY_PRESS,
    LUA_HOOK_KEY_RELEASE,
    LUA_HOOK_MOUSE_PRESS,
    LUA_HOOK_MOUSE_RELEASE,
    LUA_HOOK_MOUSE_MOTION,
    LUA_HOOK_FOCUS_IN,
    LUA_HOOK_FOCUS_OUT,
    LUA_HOOK_RESIZE,
    LUA_HOOK_STARTUP,
    LUA_HOOK_SHUTDOWN,
    LUA_HOOK_MENU_ACTION,
    LUA_HOOK_STATUS_UPDATE,
    LUA_HOOK_COMMAND_MODE,
    LUA_HOOK_COUNT
} LuaHookType;

typedef struct {
    lua_State *L;
    int initialized;
    int error_count;
    char *script_dir;
    char *init_script;
    Boolean enabled;
    Boolean debug;
    time_t last_reload;
    Boolean command_mode;
    char *command_buffer;
    size_t command_length;
    size_t command_capacity;
} LuaContext;

typedef struct LuaHook {
    int hook_type;
    int ref;
    struct LuaHook *next;
} LuaHook;

extern LuaContext *lua_ctx;

/* Core Lua API functions */
int lua_xterm_init(void);
void lua_xterm_cleanup(void);
int lua_xterm_load_script(const char *filename);
int lua_xterm_reload_scripts(void);
void lua_xterm_set_enabled(Boolean enabled);
Boolean lua_xterm_is_enabled(void);

/* Hook management */
int lua_xterm_register_hook(LuaHookType type, int ref);
void lua_xterm_unregister_hook(LuaHookType type, int ref);
Boolean lua_xterm_call_hook(LuaHookType type, ...);
void lua_xterm_clear_hooks(void);

/* Error handling */
void lua_xterm_error(const char *format, ...);
void lua_xterm_debug(const char *format, ...);
int lua_xterm_safe_call(lua_State *L, int nargs, int nresults);

/* Utility functions */
char *lua_xterm_get_script_path(const char *filename);
char *lua_xterm_get_cwd_script_path(const char *filename);
char *lua_xterm_get_home_script_path(const char *filename);
int lua_xterm_file_exists(const char *filename);
char *lua_xterm_get_home_dir(void);
char *lua_xterm_get_cwd(void);

/* Library registration functions */
int luaopen_xterm_terminal(lua_State *L);
int luaopen_xterm_config(lua_State *L);
int luaopen_xterm_menu(lua_State *L);
int luaopen_xterm_events(lua_State *L);
int luaopen_xterm_utils(lua_State *L);
int luaopen_xterm_hooks(lua_State *L);

/* Terminal manipulation from Lua */
int lua_terminal_write(lua_State *L);
int lua_terminal_message(lua_State *L);
int lua_terminal_get_text(lua_State *L);
int lua_terminal_clear(lua_State *L);
int lua_terminal_set_title(lua_State *L);
int lua_terminal_bell(lua_State *L);
int lua_terminal_goto(lua_State *L);
int lua_terminal_save_cursor(lua_State *L);
int lua_terminal_restore_cursor(lua_State *L);

/* Configuration access from Lua */
int lua_config_get(lua_State *L);
int lua_config_set(lua_State *L);
int lua_config_reset(lua_State *L);

/* Menu manipulation from Lua */
int lua_menu_add_item(lua_State *L);
int lua_menu_remove_item(lua_State *L);
int lua_menu_show(lua_State *L);

/* Utility functions for Lua */
int lua_utils_log(lua_State *L);
int lua_utils_notify(lua_State *L);
int lua_utils_timestamp(lua_State *L);
int lua_utils_file_exists(lua_State *L);
int lua_utils_system(lua_State *L);

/* Hook registration from Lua */
int lua_hooks_register(lua_State *L);
int lua_hooks_unregister(lua_State *L);
int lua_hooks_clear(lua_State *L);

/* Event handling from Lua */
int lua_events_on(lua_State *L);
int lua_events_off(lua_State *L);
int lua_events_emit(lua_State *L);

/* Resource management */
void lua_xterm_set_script_dir(const char *dir);
void lua_xterm_set_init_script(const char *script);
void lua_xterm_set_debug(Boolean debug);

/* Integration with xterm's main loop */
void lua_xterm_process_events(void);
void lua_xterm_check_reload(void);

/* Command mode functions */
Boolean lua_xterm_is_command_mode(void);
void lua_xterm_enter_command_mode(void);
void lua_xterm_exit_command_mode(void);
void lua_xterm_command_mode_input(int ch);
void lua_xterm_command_mode_backspace(void);
void lua_xterm_command_mode_execute(void);
void lua_xterm_command_mode_clear(void);
const char *lua_xterm_get_command_buffer(void);
void lua_xterm_command_mode_display(void);

#endif /* OPT_LUA_SCRIPTING */

#endif /* included_lua_api_h */