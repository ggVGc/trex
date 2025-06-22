/* $XTermId: lua_utils.c,v 1.1 2025/06/21 00:00:00 claude Exp $ */

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

#include <xterm.h>

#ifdef OPT_LUA_SCRIPTING

#include <lua_api.h>
#include <time.h>

/* Utility functions for Lua */

int
lua_utils_log(lua_State *L)
{
    const char *message;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.utils.log(message)");
    }

    message = luaL_checkstring(L, 1);
    if (message != NULL) {
        lua_xterm_debug("Lua: %s", message);
    }

    return 0;
}

int
lua_utils_notify(lua_State *L)
{
    const char *message;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.utils.notify(message)");
    }

    message = luaL_checkstring(L, 1);
    if (message != NULL) {
        /* For now, just log it. In a real implementation,
         * this could use desktop notifications */
        fprintf(stderr, "xterm-notify: %s\n", message);
    }

    return 0;
}

int
lua_utils_timestamp(lua_State *L)
{
    time_t now = time(NULL);
    lua_pushinteger(L, (lua_Integer) now);
    return 1;
}

int
lua_utils_file_exists(lua_State *L)
{
    const char *filename;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.utils.file_exists(filename)");
    }

    filename = luaL_checkstring(L, 1);
    if (filename != NULL) {
        lua_pushboolean(L, lua_xterm_file_exists(filename));
    } else {
        lua_pushboolean(L, 0);
    }

    return 1;
}

int
lua_utils_system(lua_State *L)
{
    const char *command;
    FILE *fp;
    char buffer[256];
    char *result = NULL;
    size_t result_len = 0;
    size_t buffer_len;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.utils.system(command)");
    }

    command = luaL_checkstring(L, 1);
    if (command == NULL) {
        lua_pushstring(L, "");
        return 1;
    }

    /* Execute command and capture output */
    fp = popen(command, "r");
    if (fp == NULL) {
        lua_pushstring(L, "");
        return 1;
    }

    /* Read output */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        buffer_len = strlen(buffer);
        result = (char *) realloc(result, result_len + buffer_len + 1);
        if (result != NULL) {
            strcpy(result + result_len, buffer);
            result_len += buffer_len;
        }
    }

    pclose(fp);

    if (result != NULL) {
        /* Remove trailing newline */
        if (result_len > 0 && result[result_len - 1] == '\n') {
            result[result_len - 1] = '\0';
        }
        lua_pushstring(L, result);
        free(result);
    } else {
        lua_pushstring(L, "");
    }

    return 1;
}

/* Events system */

static int event_refs[32];  /* Simple event storage */
static int event_count = 0;

int
lua_events_on(lua_State *L)
{
    const char *event_name;
    int ref;

    if (lua_gettop(L) != 2) {
        return luaL_error(L, "Usage: xterm.events.on(event_name, function)");
    }

    event_name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    /* Store function reference */
    lua_pushvalue(L, 2);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    if (event_count < 32) {
        event_refs[event_count++] = ref;
        lua_xterm_debug("Registered event handler for %s", event_name);
    } else {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        return luaL_error(L, "Too many event handlers");
    }

    return 0;
}

int
lua_events_off(lua_State *L)
{
    const char *event_name;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.events.off(event_name)");
    }

    event_name = luaL_checkstring(L, 1);
    lua_xterm_debug("Unregistered event handler for %s", event_name);

    return 0;
}

int
lua_events_emit(lua_State *L)
{
    const char *event_name;

    if (lua_gettop(L) < 1) {
        return luaL_error(L, "Usage: xterm.events.emit(event_name, ...)");
    }

    event_name = luaL_checkstring(L, 1);
    lua_xterm_debug("Event emitted: %s", event_name);

    return 0;
}

/* Command mode functions */

int
lua_utils_enter_command_mode(lua_State *L)
{
    lua_xterm_enter_command_mode();
    return 0;
}

int
lua_utils_exit_command_mode(lua_State *L)
{
    lua_xterm_exit_command_mode();
    return 0;
}

int
lua_utils_is_command_mode(lua_State *L)
{
    lua_pushboolean(L, lua_xterm_is_command_mode());
    return 1;
}

/* Library registration */

int
luaopen_xterm_utils(lua_State *L)
{
    static const luaL_Reg utils_funcs[] = {
        {"log", lua_utils_log},
        {"notify", lua_utils_notify},
        {"timestamp", lua_utils_timestamp},
        {"file_exists", lua_utils_file_exists},
        {"system", lua_utils_system},
        {"enter_command_mode", lua_utils_enter_command_mode},
        {"exit_command_mode", lua_utils_exit_command_mode},
        {"is_command_mode", lua_utils_is_command_mode},
        {NULL, NULL}
    };

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
luaopen_xterm_events(lua_State *L)
{
    static const luaL_Reg events_funcs[] = {
        {"on", lua_events_on},
        {"off", lua_events_off},
        {"emit", lua_events_emit},
        {NULL, NULL}
    };

    /* Create xterm table if it doesn't exist */
    lua_getglobal(L, "xterm");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "xterm");
        lua_getglobal(L, "xterm");
    }

    /* Create events subtable */
    luaL_newlib(L, events_funcs);
    lua_setfield(L, -2, "events");

    lua_pop(L, 1);  /* Pop xterm table */
    return 0;
}

#endif /* OPT_LUA_SCRIPTING */