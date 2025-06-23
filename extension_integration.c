/* $XTermId: extension_integration.c,v 1.1 2025/06/23 00:00:00 claude Exp $ */

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

#include <extension_integration.h>

#ifdef OPT_LUA_SCRIPTING
#include <plugins/lua/lua_plugin.h>
#endif

#include <stdarg.h>

static Boolean system_enabled = True;
static ExtensionPlugin *lua_plugin = NULL;

int
extension_system_init(void)
{
    /* Initialize extension registry */
    if (!extension_registry_init()) {
        return 0;
    }

#ifdef OPT_LUA_SCRIPTING
    /* Create and register Lua plugin */
    lua_plugin = create_lua_plugin();
    if (lua_plugin != NULL) {
        if (extension_registry_register_plugin(lua_plugin)) {
            return 1;
        } else {
            /* Failed to register, cleanup */
            if (lua_plugin->plugin_cleanup) {
                lua_plugin->plugin_cleanup(lua_plugin);
            }
            free(lua_plugin);
            lua_plugin = NULL;
        }
    }
#endif

    return 1;
}

void
extension_system_cleanup(void)
{
#ifdef OPT_LUA_SCRIPTING
    lua_plugin = NULL; /* Registry will cleanup the plugin */
#endif

    extension_registry_cleanup();
}

Boolean
extension_system_call_hook(ExtensionHookType type, ...)
{
    va_list args;
    Boolean result;

    if (!system_enabled) {
        return False;
    }

    va_start(args, type);
    result = extension_call_hook(type, args);
    va_end(args);

    return result;
}

void
extension_system_process_events(void)
{
    if (system_enabled) {
        extension_process_events();
    }
}

Boolean
extension_system_is_command_mode(void)
{
#ifdef OPT_LUA_SCRIPTING
    if (lua_plugin != NULL && lua_plugin->plugin_data != NULL) {
        LuaPluginData *data = (LuaPluginData *) lua_plugin->plugin_data;
        return lua_plugin_is_command_mode(data);
    }
#endif
    return False;
}

void
extension_system_enter_command_mode(void)
{
#ifdef OPT_LUA_SCRIPTING
    if (lua_plugin != NULL && lua_plugin->plugin_data != NULL && lua_plugin->api != NULL) {
        LuaPluginData *data = (LuaPluginData *) lua_plugin->plugin_data;
        lua_plugin_enter_command_mode(data, lua_plugin->api);
    }
#endif
}

void
extension_system_exit_command_mode(void)
{
#ifdef OPT_LUA_SCRIPTING
    if (lua_plugin != NULL && lua_plugin->plugin_data != NULL && lua_plugin->api != NULL) {
        LuaPluginData *data = (LuaPluginData *) lua_plugin->plugin_data;
        lua_plugin_exit_command_mode(data, lua_plugin->api);
    }
#endif
}

void
extension_system_command_mode_input(int ch)
{
#ifdef OPT_LUA_SCRIPTING
    if (lua_plugin != NULL && lua_plugin->plugin_data != NULL && lua_plugin->api != NULL) {
        LuaPluginData *data = (LuaPluginData *) lua_plugin->plugin_data;
        lua_plugin_command_mode_input(data, lua_plugin->api, ch);
    }
#endif
}

void
extension_system_command_mode_execute(void)
{
#ifdef OPT_LUA_SCRIPTING
    if (lua_plugin != NULL && lua_plugin->plugin_data != NULL && lua_plugin->api != NULL) {
        LuaPluginData *data = (LuaPluginData *) lua_plugin->plugin_data;
        lua_plugin_command_mode_execute(data, lua_plugin->api);
    }
#endif
}

void
extension_system_command_mode_backspace(void)
{
#ifdef OPT_LUA_SCRIPTING
    if (lua_plugin != NULL && lua_plugin->plugin_data != NULL) {
        LuaPluginData *data = (LuaPluginData *) lua_plugin->plugin_data;
        if (data->command_mode && data->command_buffer && data->command_length > 0) {
            data->command_length--;
            data->command_buffer[data->command_length] = '\0';
        }
    }
#endif
}

Boolean
extension_system_is_enabled(void)
{
    return system_enabled;
}

void
extension_system_set_enabled(Boolean enabled)
{
    system_enabled = enabled;
}