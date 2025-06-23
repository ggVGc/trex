/* $XTermId: lua_plugin.h,v 1.1 2025/06/23 00:00:00 claude Exp $ */

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

#ifndef included_lua_plugin_h
#define included_lua_plugin_h

#include <xterm.h>
#include <extension_api.h>

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

#define LUA_SCRIPT_DIR_DEFAULT "runtime"
#define LUA_INIT_SCRIPT "trex.lua"
#define LUA_MAX_SCRIPT_SIZE (1024 * 1024)  /* 1MB max script size */

typedef struct LuaHook {
    ExtensionHookType hook_type;
    int ref;
    struct LuaHook *next;
} LuaHook;

typedef struct {
    lua_State *L;
    char *script_dir;
    char *init_script;
    time_t last_reload;
    Boolean command_mode;
    char *command_buffer;
    size_t command_length;
    size_t command_capacity;
    LuaHook *hook_lists[EXT_HOOK_COUNT];
} LuaPluginData;

/* Plugin interface functions */
int lua_plugin_init(ExtensionPlugin *plugin, const ExtensionAPI *api);
void lua_plugin_cleanup(ExtensionPlugin *plugin);
int lua_plugin_reload(ExtensionPlugin *plugin);
Boolean lua_plugin_call_hook(ExtensionPlugin *plugin, ExtensionHookType type, ...);

/* Lua state management */
int lua_plugin_create_state(LuaPluginData *data, const ExtensionAPI *api);
void lua_plugin_destroy_state(LuaPluginData *data);
int lua_plugin_load_script(LuaPluginData *data, const ExtensionAPI *api, const char *filename);
int lua_plugin_reload_scripts(LuaPluginData *data, const ExtensionAPI *api);

/* Lua API registration */
int luaopen_xterm_terminal_plugin(lua_State *L, const ExtensionAPI *api);
int luaopen_xterm_config_plugin(lua_State *L, const ExtensionAPI *api);
int luaopen_xterm_menu_plugin(lua_State *L, const ExtensionAPI *api);
int luaopen_xterm_events_plugin(lua_State *L, const ExtensionAPI *api);
int luaopen_xterm_utils_plugin(lua_State *L, const ExtensionAPI *api);
int luaopen_xterm_hooks_plugin(lua_State *L, const ExtensionAPI *api);

/* Hook management */
int lua_plugin_register_hook(LuaPluginData *data, ExtensionHookType type, int ref);
void lua_plugin_unregister_hook(LuaPluginData *data, ExtensionHookType type, int ref);
void lua_plugin_clear_hooks(LuaPluginData *data);

/* Command mode functions */
Boolean lua_plugin_is_command_mode(LuaPluginData *data);
void lua_plugin_enter_command_mode(LuaPluginData *data, const ExtensionAPI *api);
void lua_plugin_exit_command_mode(LuaPluginData *data, const ExtensionAPI *api);
void lua_plugin_command_mode_input(LuaPluginData *data, const ExtensionAPI *api, int ch);
void lua_plugin_command_mode_execute(LuaPluginData *data, const ExtensionAPI *api);
const char *lua_plugin_get_command_buffer(LuaPluginData *data);

/* Utility functions */
char *lua_plugin_get_script_path(LuaPluginData *data, const ExtensionAPI *api, const char *filename);
void lua_plugin_setup_trex_paths(lua_State *L, const ExtensionAPI *api);
void lua_plugin_sandbox(lua_State *L);
int lua_plugin_safe_call(lua_State *L, int nargs, int nresults);
int lua_plugin_panic(lua_State *L);

/* Factory function */
ExtensionPlugin *create_lua_plugin(void);

#endif /* OPT_LUA_SCRIPTING */

#endif /* included_lua_plugin_h */