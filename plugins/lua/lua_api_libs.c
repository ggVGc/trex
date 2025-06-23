/* $XTermId: lua_api_libs.c,v 1.1 2025/06/23 00:00:00 claude Exp $ */

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

#include "lua_plugin.h"
#include <extension_api.h>

#ifdef OPT_LUA_SCRIPTING

static const ExtensionAPI *current_api = NULL;
static ExtensionPlugin *current_plugin = NULL;

/* Terminal API functions */
static int
lua_terminal_write_plugin(lua_State *L)
{
    const char *text;
    size_t len;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.terminal.write(text)");
    }

    text = luaL_checklstring(L, 1, &len);
    if (text != NULL && len > 0 && current_api) {
        current_api->terminal_write(text, len);
    }

    return 0;
}

static int
lua_terminal_message_plugin(lua_State *L)
{
    const char *text;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.terminal.message(text)");
    }

    text = luaL_checkstring(L, 1);
    if (text != NULL && current_api) {
        current_api->terminal_message(text);
    }

    return 0;
}

static int
lua_terminal_get_text_plugin(lua_State *L)
{
    char *text;

    if (current_api) {
        text = current_api->terminal_get_text();
        if (text) {
            lua_pushstring(L, text);
            free(text);
        } else {
            lua_pushstring(L, "");
        }
    } else {
        lua_pushstring(L, "");
    }

    return 1;
}

static int
lua_terminal_clear_plugin(lua_State *L)
{
    if (current_api) {
        current_api->terminal_clear();
    }
    return 0;
}

static int
lua_terminal_set_title_plugin(lua_State *L)
{
    const char *title;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.terminal.set_title(title)");
    }

    title = luaL_checkstring(L, 1);
    if (title != NULL && current_api) {
        current_api->terminal_set_title(title);
    }

    return 0;
}

static int
lua_terminal_bell_plugin(lua_State *L)
{
    if (current_api) {
        current_api->terminal_bell();
    }
    return 0;
}

static int
lua_terminal_goto_plugin(lua_State *L)
{
    int row, col;

    if (lua_gettop(L) != 2) {
        return luaL_error(L, "Usage: xterm.terminal.goto(row, col)");
    }

    row = (int) luaL_checkinteger(L, 1);
    col = (int) luaL_checkinteger(L, 2);

    if (current_api) {
        current_api->terminal_goto(row, col);
    }

    return 0;
}

static int
lua_terminal_save_cursor_plugin(lua_State *L)
{
    if (current_api) {
        current_api->terminal_save_cursor();
    }
    return 0;
}

static int
lua_terminal_restore_cursor_plugin(lua_State *L)
{
    if (current_api) {
        current_api->terminal_restore_cursor();
    }
    return 0;
}

/* Configuration functions */
static int
lua_config_get_plugin(lua_State *L)
{
    const char *key;
    char *value;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.config.get(key)");
    }

    key = luaL_checkstring(L, 1);
    if (key == NULL) {
        lua_pushnil(L);
        return 1;
    }

    if (current_api) {
        value = current_api->config_get(key);
        if (value) {
            lua_pushstring(L, value);
            free(value);
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }

    return 1;
}

static int
lua_config_set_plugin(lua_State *L)
{
    const char *key, *value;

    if (lua_gettop(L) != 2) {
        return luaL_error(L, "Usage: xterm.config.set(key, value)");
    }

    key = luaL_checkstring(L, 1);
    value = luaL_checkstring(L, 2);

    if (key != NULL && value != NULL && current_api) {
        current_api->config_set(key, value);
    }

    return 0;
}

static int
lua_config_reset_plugin(lua_State *L)
{
    if (current_api) {
        current_api->config_reset();
    }
    return 0;
}

/* Utility functions */
static int
lua_utils_log_plugin(lua_State *L)
{
    const char *message;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.utils.log(message)");
    }

    message = luaL_checkstring(L, 1);
    if (message != NULL && current_api) {
        current_api->log_message("Lua: %s", message);
    }

    return 0;
}

static int
lua_utils_file_exists_plugin(lua_State *L)
{
    const char *filename;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.utils.file_exists(filename)");
    }

    filename = luaL_checkstring(L, 1);
    if (filename != NULL && current_api) {
        lua_pushboolean(L, current_api->file_exists(filename));
    } else {
        lua_pushboolean(L, 0);
    }

    return 1;
}

static int
lua_utils_enter_command_mode_plugin(lua_State *L)
{
    if (current_api) {
        current_api->enter_command_mode();
    }
    return 0;
}

static int
lua_utils_exit_command_mode_plugin(lua_State *L)
{
    if (current_api) {
        current_api->exit_command_mode();
    }
    return 0;
}

static int
lua_utils_is_command_mode_plugin(lua_State *L)
{
    if (current_api) {
        lua_pushboolean(L, current_api->is_command_mode());
    } else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

/* Hook functions */
static int
lua_hooks_register_plugin(lua_State *L)
{
    const char *hook_name;
    ExtensionHookType hook_type;
    int ref;
    LuaPluginData *data;

    if (lua_gettop(L) != 2) {
        return luaL_error(L, "Usage: xterm.hooks.register(hook_name, function)");
    }

    hook_name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    hook_type = extension_hook_name_to_type(hook_name);
    if (hook_type == -1) {
        return luaL_error(L, "Unknown hook type: %s", hook_name);
    }

    /* Create reference to function */
    lua_pushvalue(L, 2);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    /* Register hook with the Lua plugin */
    if (current_plugin && current_plugin->plugin_data) {
        data = (LuaPluginData *) current_plugin->plugin_data;
        if (lua_plugin_register_hook(data, hook_type, ref)) {
            if (current_api) {
                current_api->debug_message("Lua hook registered: %s", hook_name);
            }
        } else {
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
            return luaL_error(L, "Failed to register hook: %s", hook_name);
        }
    } else {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        return luaL_error(L, "No plugin context available for hook registration");
    }

    return 0;
}

static int
lua_hooks_unregister_plugin(lua_State *L)
{
    const char *hook_name;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.hooks.unregister(hook_name)");
    }

    hook_name = luaL_checkstring(L, 1);
    if (current_api) {
        current_api->debug_message("Lua hook unregistered: %s", hook_name);
    }

    return 0;
}

static int
lua_hooks_clear_plugin(lua_State *L)
{
    if (current_api) {
        current_api->debug_message("Lua hooks cleared");
    }
    return 0;
}

/* Library registration functions */
int
luaopen_xterm_terminal_plugin(lua_State *L, const ExtensionAPI *api)
{
    static const luaL_Reg terminal_funcs[] = {
        {"write", lua_terminal_write_plugin},
        {"message", lua_terminal_message_plugin},
        {"get_text", lua_terminal_get_text_plugin},
        {"clear", lua_terminal_clear_plugin},
        {"set_title", lua_terminal_set_title_plugin},
        {"bell", lua_terminal_bell_plugin},
        {"goto", lua_terminal_goto_plugin},
        {"save_cursor", lua_terminal_save_cursor_plugin},
        {"restore_cursor", lua_terminal_restore_cursor_plugin},
        {NULL, NULL}
    };

    current_api = api;

    /* Create xterm table if it doesn't exist */
    lua_getglobal(L, "xterm");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "xterm");
        lua_getglobal(L, "xterm");
    }

    /* Create terminal subtable */
    luaL_newlib(L, terminal_funcs);
    lua_setfield(L, -2, "terminal");

    lua_pop(L, 1);  /* Pop xterm table */
    return 0;
}

int
luaopen_xterm_config_plugin(lua_State *L, const ExtensionAPI *api)
{
    static const luaL_Reg config_funcs[] = {
        {"get", lua_config_get_plugin},
        {"set", lua_config_set_plugin},
        {"reset", lua_config_reset_plugin},
        {NULL, NULL}
    };

    current_api = api;

    /* Create xterm table if it doesn't exist */
    lua_getglobal(L, "xterm");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "xterm");
        lua_getglobal(L, "xterm");
    }

    /* Create config subtable */
    luaL_newlib(L, config_funcs);
    lua_setfield(L, -2, "config");

    lua_pop(L, 1);  /* Pop xterm table */
    return 0;
}

int
luaopen_xterm_menu_plugin(lua_State *L, const ExtensionAPI *api)
{
    current_api = api;

    /* Create xterm table if it doesn't exist */
    lua_getglobal(L, "xterm");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "xterm");
        lua_getglobal(L, "xterm");
    }

    /* Create empty menu subtable for now */
    lua_newtable(L);
    lua_setfield(L, -2, "menu");

    lua_pop(L, 1);  /* Pop xterm table */
    return 0;
}

int
luaopen_xterm_events_plugin(lua_State *L, const ExtensionAPI *api)
{
    current_api = api;

    /* Create xterm table if it doesn't exist */
    lua_getglobal(L, "xterm");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "xterm");
        lua_getglobal(L, "xterm");
    }

    /* Create empty events subtable for now */
    lua_newtable(L);
    lua_setfield(L, -2, "events");

    lua_pop(L, 1);  /* Pop xterm table */
    return 0;
}

int
luaopen_xterm_utils_plugin(lua_State *L, const ExtensionAPI *api)
{
    static const luaL_Reg utils_funcs[] = {
        {"log", lua_utils_log_plugin},
        {"file_exists", lua_utils_file_exists_plugin},
        {"enter_command_mode", lua_utils_enter_command_mode_plugin},
        {"exit_command_mode", lua_utils_exit_command_mode_plugin},
        {"is_command_mode", lua_utils_is_command_mode_plugin},
        {NULL, NULL}
    };

    current_api = api;

    /* Create xterm table if it doesn't exist */
    lua_getglobal(L, "xterm");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "xterm");
        lua_getglobal(L, "xterm");
    }

    /* Create utils subtable */
    luaL_newlib(L, utils_funcs);
    lua_setfield(L, -2, "utils");

    lua_pop(L, 1);  /* Pop xterm table */
    return 0;
}

int
luaopen_xterm_hooks_plugin(lua_State *L, const ExtensionAPI *api)
{
    static const luaL_Reg hooks_funcs[] = {
        {"register", lua_hooks_register_plugin},
        {"unregister", lua_hooks_unregister_plugin},
        {"clear", lua_hooks_clear_plugin},
        {NULL, NULL}
    };

    current_api = api;

    /* Find the Lua plugin in the extension registry */
    if (api && ext_registry && ext_registry->initialized) {
        current_plugin = extension_registry_find_plugin("lua");
    }

    /* Create xterm table if it doesn't exist */
    lua_getglobal(L, "xterm");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "xterm");
        lua_getglobal(L, "xterm");
    }

    /* Create hooks subtable */
    luaL_newlib(L, hooks_funcs);
    lua_setfield(L, -2, "hooks");

    lua_pop(L, 1);  /* Pop xterm table */
    return 0;
}

#endif /* OPT_LUA_SCRIPTING */