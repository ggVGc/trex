/* $XTermId: lua_plugin.c,v 1.1 2025/06/23 00:00:00 claude Exp $ */

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
#include <xstrings.h>

#ifdef OPT_LUA_SCRIPTING

#include <sys/stat.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

/* Store API reference for Lua C functions */
static const ExtensionAPI *g_api = NULL;

ExtensionPlugin *
create_lua_plugin(void)
{
    ExtensionPlugin *plugin = (ExtensionPlugin *) calloc(1, sizeof(ExtensionPlugin));
    if (plugin == NULL) {
        return NULL;
    }

    plugin->name = x_strdup("lua");
    plugin->version = x_strdup("1.0.0");
    plugin->description = x_strdup("Lua scripting plugin for xterm");
    plugin->author = x_strdup("Claude (Anthropic)");
    
    plugin->initialized = False;
    plugin->enabled = True;
    plugin->debug = False;
    plugin->error_count = 0;
    plugin->plugin_data = NULL;
    
    plugin->plugin_init = lua_plugin_init;
    plugin->plugin_cleanup = lua_plugin_cleanup;
    plugin->plugin_reload = lua_plugin_reload;
    plugin->plugin_call_hook = lua_plugin_call_hook;

    return plugin;
}

int
lua_plugin_init(ExtensionPlugin *plugin, const ExtensionAPI *api)
{
    LuaPluginData *data;

    if (plugin == NULL || api == NULL) {
        return 0;
    }

    g_api = api;

    data = (LuaPluginData *) calloc(1, sizeof(LuaPluginData));
    if (data == NULL) {
        api->error_message("Failed to allocate Lua plugin data");
        return 0;
    }

    data->script_dir = x_strdup(LUA_SCRIPT_DIR_DEFAULT);
    data->init_script = x_strdup(LUA_INIT_SCRIPT);
    data->last_reload = time(NULL);
    data->command_mode = False;
    data->command_buffer = NULL;
    data->command_length = 0;
    data->command_capacity = 0;

    /* Initialize hook lists */
    for (int i = 0; i < EXT_HOOK_COUNT; i++) {
        data->hook_lists[i] = NULL;
    }

    plugin->plugin_data = data;

    /* Create Lua state */
    if (!lua_plugin_create_state(data, api)) {
        api->error_message("Failed to create Lua state");
        lua_plugin_cleanup(plugin);
        return 0;
    }

    /* Load initial scripts */
    lua_plugin_reload_scripts(data, api);

    api->debug_message("Lua plugin initialized");
    return 1;
}

void
lua_plugin_cleanup(ExtensionPlugin *plugin)
{
    LuaPluginData *data;

    if (plugin == NULL || plugin->plugin_data == NULL) {
        return;
    }

    data = (LuaPluginData *) plugin->plugin_data;

    if (g_api) {
        g_api->debug_message("Cleaning up Lua plugin");
    }

    lua_plugin_destroy_state(data);

    if (data->script_dir) {
        free(data->script_dir);
        data->script_dir = NULL;
    }

    if (data->init_script) {
        free(data->init_script);
        data->init_script = NULL;
    }

    if (data->command_buffer) {
        free(data->command_buffer);
        data->command_buffer = NULL;
    }

    free(data);
    plugin->plugin_data = NULL;
    g_api = NULL;
}

int
lua_plugin_reload(ExtensionPlugin *plugin)
{
    LuaPluginData *data;

    if (plugin == NULL || plugin->plugin_data == NULL) {
        return 0;
    }

    data = (LuaPluginData *) plugin->plugin_data;

    if (g_api) {
        g_api->debug_message("Reloading Lua plugin");
    }

    return lua_plugin_reload_scripts(data, g_api);
}

Boolean
lua_plugin_call_hook(ExtensionPlugin *plugin, ExtensionHookType type, ...)
{
    LuaPluginData *data;
    LuaHook *hook;
    va_list args;
    Boolean handled = False;

    if (plugin == NULL || plugin->plugin_data == NULL || !plugin->enabled) {
        return False;
    }

    if (type < 0 || type >= EXT_HOOK_COUNT) {
        return False;
    }

    data = (LuaPluginData *) plugin->plugin_data;
    hook = data->hook_lists[type];

    if (hook == NULL) {
        return False;
    }

    va_start(args, type);

    /* Call each registered hook for this type */
    while (hook != NULL) {
        lua_rawgeti(data->L, LUA_REGISTRYINDEX, hook->ref);
        
        if (!lua_isfunction(data->L, -1)) {
            lua_pop(data->L, 1);
            hook = hook->next;
            continue;  
        }

        int argc = 0;
        va_list args_copy;
        va_copy(args_copy, args);

        /* Push arguments based on hook type */
        switch (type) {
        case EXT_HOOK_KEY_PRESS:
        case EXT_HOOK_KEY_RELEASE:
            {
                int keysym = va_arg(args_copy, int);
                int state = va_arg(args_copy, int);
                lua_pushinteger(data->L, keysym);
                lua_pushinteger(data->L, state);
                argc = 2;
                break;
            }
        case EXT_HOOK_COMMAND_MODE:
            {
                const char *mode = va_arg(args_copy, const char *);
                lua_pushstring(data->L, mode ? mode : "");
                argc = 1;
                break;
            }
        default:
            /* No arguments for other hook types */
            break;
        }

        va_end(args_copy);

        /* Call the Lua function */
        if (lua_plugin_safe_call(data->L, argc, 1) == LUA_OK) {
            /* Check return value for some hooks */
            if (type == EXT_HOOK_KEY_PRESS || type == EXT_HOOK_KEY_RELEASE) {
                if (lua_isboolean(data->L, -1) && lua_toboolean(data->L, -1)) {
                    handled = True;
                }
            }
            lua_pop(data->L, 1);
        }

        hook = hook->next;
    }

    va_end(args);
    return handled;
}

int
lua_plugin_create_state(LuaPluginData *data, const ExtensionAPI *api)
{
    data->L = luaL_newstate();
    if (data->L == NULL) {
        api->error_message("Failed to create Lua state");
        return 0;
    }

    /* Set panic function */
    lua_atpanic(data->L, lua_plugin_panic);

    /* Open standard libraries */
    luaL_openlibs(data->L);

    /* Set up trex runtime paths before sandbox */
    lua_plugin_setup_trex_paths(data->L, api);
    
    /* Apply sandbox restrictions */
    lua_plugin_sandbox(data->L);

    /* Register xterm-specific libraries */
    luaopen_xterm_terminal_plugin(data->L, api);
    luaopen_xterm_config_plugin(data->L, api);
    luaopen_xterm_menu_plugin(data->L, api);
    luaopen_xterm_events_plugin(data->L, api);
    luaopen_xterm_utils_plugin(data->L, api);
    luaopen_xterm_hooks_plugin(data->L, api);

    return 1;
}

void
lua_plugin_destroy_state(LuaPluginData *data)
{
    if (data->L) {
        lua_plugin_clear_hooks(data);
        lua_close(data->L);
        data->L = NULL;
    }
}

int
lua_plugin_reload_scripts(LuaPluginData *data, const ExtensionAPI *api)
{
    int result = 0;

    api->debug_message("Reloading Lua scripts");

    /* Clear existing hooks */
    lua_plugin_clear_hooks(data);

    /* Reset Lua state */
    lua_plugin_destroy_state(data);
    if (!lua_plugin_create_state(data, api)) {
        return 0;
    }

    /* Load trex_init module using standard require */
    lua_getglobal(data->L, "require");
    if (lua_isfunction(data->L, -1)) {
        lua_pushstring(data->L, "trex_init");
        result = lua_plugin_safe_call(data->L, 1, 0);
        if (result != LUA_OK) {
            api->error_message("Failed to load trex_init module: %s", lua_tostring(data->L, -1));
            lua_pop(data->L, 1);
            return 0;
        }
        data->last_reload = time(NULL);
        return 1;
    } else {
        lua_pop(data->L, 1);
        api->error_message("require function not available");
        return 0;
    }
}

void
lua_plugin_setup_trex_paths(lua_State *L, const ExtensionAPI *api)
{
    char *cwd = NULL;
    char *home_dir = NULL; 
    char *trex_path = NULL;
    size_t path_len;
    
    /* Get current working directory */
    cwd = api->get_cwd();
    if (cwd == NULL) {
        api->debug_message("Failed to get current working directory for trex paths");
        return;
    }

    /* Get home directory */
    home_dir = api->get_home_dir();
    if (home_dir == NULL) {
        api->debug_message("Failed to get home directory for trex paths");
        free(cwd);
        return;
    }

    /* Create paths including current working directory, runtime subdirectories, and home share directory */
    const char *patterns[] = {
        "/runtime/?.lua"
    };
    size_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    
    /* Calculate total length needed for both cwd and home paths */
    path_len = 1; /* for null terminator */
    for (size_t i = 0; i < num_patterns; i++) {
        path_len += strlen(cwd) + strlen(patterns[i]);
        path_len += strlen(home_dir) + strlen("/.local/share/trex") + strlen(patterns[i]);
        if (i > 0) path_len += 2; /* for semicolons */
    }
    path_len += 1; /* for semicolon between cwd and home paths */
    
    trex_path = (char *) malloc(path_len);
    if (trex_path == NULL) {
        free(cwd);
        free(home_dir);
        api->debug_message("Failed to allocate memory for trex path");
        return;
    }

    /* Build the complete path string - first current working directory, then home directory */
    trex_path[0] = '\0';
    for (size_t i = 0; i < num_patterns; i++) {
        if (i > 0) strcat(trex_path, ";");
        strcat(trex_path, cwd);
        strcat(trex_path, patterns[i]);
    }
    
    /* Add home directory paths */
    for (size_t i = 0; i < num_patterns; i++) {
        strcat(trex_path, ";");
        strcat(trex_path, home_dir);
        strcat(trex_path, "/.local/share/trex");
        strcat(trex_path, patterns[i]);
    }
    
    free(cwd);
    free(home_dir);

    /* Set up package.path to include runtime directory */
    lua_getglobal(L, "package");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "path");
        if (lua_isstring(L, -1)) {
            /* Prepend trex path to existing package.path */
            const char *existing_path = lua_tostring(L, -1);
            size_t full_path_len = strlen(trex_path) + strlen(";") + strlen(existing_path) + 1;
            char *full_path = (char *) malloc(full_path_len);
            if (full_path != NULL) {
                snprintf(full_path, full_path_len, "%s;%s", trex_path, existing_path);
                lua_pop(L, 1);  /* Pop old path */
                lua_pushstring(L, full_path);
                lua_setfield(L, -2, "path");
                free(full_path);
            }
        } else {
            /* No existing path, just set our path */
            lua_pop(L, 1);  /* Pop nil/invalid value */
            lua_pushstring(L, trex_path);
            lua_setfield(L, -2, "path");
        }
    }
    lua_pop(L, 1);  /* Pop package table */

    free(trex_path);
    
    api->debug_message("Set up trex runtime paths");
}

void
lua_plugin_sandbox(lua_State *L)
{
    /* No sandbox restrictions - full Lua environment available */
    /* os, io, dofile, loadfile, and all standard libraries are accessible */
}

int
lua_plugin_safe_call(lua_State *L, int nargs, int nresults)
{
    int result = lua_pcall(L, nargs, nresults, 0);
    if (result != LUA_OK && g_api) {
        g_api->error_message("Lua error: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    return result;
}

int
lua_plugin_panic(lua_State *L)
{
    if (g_api) {
        g_api->error_message("Lua panic: %s", lua_tostring(L, -1));
    }
    return 0;
}

int
lua_plugin_register_hook(LuaPluginData *data, ExtensionHookType type, int ref)
{
    LuaHook *hook;

    if (type < 0 || type >= EXT_HOOK_COUNT) {
        return 0;
    }

    hook = (LuaHook *) malloc(sizeof(LuaHook));
    if (hook == NULL) {
        return 0;
    }

    hook->hook_type = type;
    hook->ref = ref;
    hook->next = data->hook_lists[type];
    data->hook_lists[type] = hook;

    if (g_api) {
        g_api->debug_message("Registered Lua hook for %s", extension_hook_type_name(type));
    }
    return 1;
}

void
lua_plugin_unregister_hook(LuaPluginData *data, ExtensionHookType type, int ref)
{
    LuaHook **current, *hook;

    if (type < 0 || type >= EXT_HOOK_COUNT) {
        return;
    }

    current = &data->hook_lists[type];
    while (*current != NULL) {
        hook = *current;
        if (hook->ref == ref) {
            *current = hook->next;
            luaL_unref(data->L, LUA_REGISTRYINDEX, ref);
            free(hook);
            if (g_api) {
                g_api->debug_message("Unregistered Lua hook for %s", extension_hook_type_name(type));
            }
            return;
        }
        current = &hook->next;
    }
}

void
lua_plugin_clear_hooks(LuaPluginData *data)
{
    int i;
    LuaHook *hook, *next;

    if (data->L == NULL) {
        return;
    }

    for (i = 0; i < EXT_HOOK_COUNT; i++) {
        hook = data->hook_lists[i];
        while (hook != NULL) {
            next = hook->next;
            luaL_unref(data->L, LUA_REGISTRYINDEX, hook->ref);
            free(hook);
            hook = next;
        }
        data->hook_lists[i] = NULL;
    }

    if (g_api) {
        g_api->debug_message("Cleared all Lua hooks");
    }
}

/* Command mode implementation */
Boolean
lua_plugin_is_command_mode(LuaPluginData *data)
{
    return data->command_mode;
}

void
lua_plugin_enter_command_mode(LuaPluginData *data, const ExtensionAPI *api)
{
    data->command_mode = True;
    
    /* Clear command buffer */
    if (data->command_buffer) {
        data->command_length = 0;
        data->command_buffer[0] = '\0';
    }
    
    api->debug_message("Entered Lua command mode");
}

void
lua_plugin_exit_command_mode(LuaPluginData *data, const ExtensionAPI *api)
{
    data->command_mode = False;
    
    /* Clear command buffer */
    if (data->command_buffer) {
        data->command_length = 0;
        data->command_buffer[0] = '\0';
    }
    
    api->debug_message("Exited Lua command mode");
}

void
lua_plugin_command_mode_input(LuaPluginData *data, const ExtensionAPI *api, int ch)
{
    if (!data->command_mode) {
        return;
    }
    
    /* Ensure buffer exists */
    if (!data->command_buffer) {
        data->command_capacity = 256;
        data->command_buffer = (char *) malloc(data->command_capacity);
        if (!data->command_buffer) {
            api->error_message("Failed to allocate command buffer");
            return;
        }
        data->command_length = 0;
        data->command_buffer[0] = '\0';
    }
    
    /* Grow buffer if needed */
    if (data->command_length + 2 >= data->command_capacity) {
        size_t new_capacity = data->command_capacity * 2;
        char *new_buffer = (char *) realloc(data->command_buffer, new_capacity);
        if (!new_buffer) {
            api->error_message("Failed to grow command buffer");
            return;
        }
        data->command_buffer = new_buffer;
        data->command_capacity = new_capacity;
    }
    
    /* Add character to buffer */
    data->command_buffer[data->command_length++] = (char) ch;
    data->command_buffer[data->command_length] = '\0';
}

void
lua_plugin_command_mode_execute(LuaPluginData *data, const ExtensionAPI *api)
{
    if (!data->command_mode || !data->command_buffer || data->command_length == 0) {
        return;
    }
    
    api->debug_message("Executing Lua command: %s", data->command_buffer);
    
    /* Execute the command as Lua code */
    int status = luaL_loadstring(data->L, data->command_buffer);
    if (status == LUA_OK) {
        status = lua_plugin_safe_call(data->L, 0, 0);
    }
    
    if (status != LUA_OK) {
        const char *error = lua_tostring(data->L, -1);
        api->error_message("Command error: %s", error ? error : "unknown error");
        lua_pop(data->L, 1);
    }
}

const char *
lua_plugin_get_command_buffer(LuaPluginData *data)
{
    if (!data->command_buffer) {
        return "";
    }
    
    return data->command_buffer;
}

#endif /* OPT_LUA_SCRIPTING */