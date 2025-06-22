/* $XTermId: lua_hooks.c,v 1.1 2025/06/21 00:00:00 claude Exp $ */

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
#include <stdarg.h>

static LuaHook *hook_lists[LUA_HOOK_COUNT] = { NULL };

static const char *hook_names[LUA_HOOK_COUNT] = {
    "char_pre",
    "char_post", 
    "key_press",
    "key_release",
    "mouse_press",
    "mouse_release",
    "mouse_motion",
    "focus_in",
    "focus_out",
    "resize",
    "startup",
    "shutdown",
    "menu_action",
    "status_update",
    "command_mode"
};

static LuaHookType lua_hook_name_to_type(const char *name);

int
lua_xterm_register_hook(LuaHookType type, int ref)
{
    LuaHook *hook;

    if (type < 0 || type >= LUA_HOOK_COUNT) {
        return 0;
    }

    hook = (LuaHook *) malloc(sizeof(LuaHook));
    if (hook == NULL) {
        return 0;
    }

    hook->hook_type = type;
    hook->ref = ref;
    hook->next = hook_lists[type];
    hook_lists[type] = hook;

    lua_xterm_debug("Registered hook for %s", hook_names[type]);
    return 1;
}

void
lua_xterm_unregister_hook(LuaHookType type, int ref)
{
    LuaHook **current, *hook;

    if (type < 0 || type >= LUA_HOOK_COUNT) {
        return;
    }

    current = &hook_lists[type];
    while (*current != NULL) {
        hook = *current;
        if (hook->ref == ref) {
            *current = hook->next;
            luaL_unref(lua_ctx->L, LUA_REGISTRYINDEX, ref);
            free(hook);
            lua_xterm_debug("Unregistered hook for %s", hook_names[type]);
            return;
        }
        current = &hook->next;
    }
}

Boolean
lua_xterm_call_hook(LuaHookType type, ...)
{
    LuaHook *hook;
    va_list args;
    int argc = 0;
    const char *str_arg;
    int int_arg;
    Boolean bool_arg;

    if (!lua_xterm_is_enabled() || type < 0 || type >= LUA_HOOK_COUNT) {
        return False;
    }

    hook = hook_lists[type];
    if (hook == NULL) {
        return False;
    }

    va_start(args, type);

    /* Call each registered hook */
    while (hook != NULL) {
        /* Push function onto stack */
        lua_rawgeti(lua_ctx->L, LUA_REGISTRYINDEX, hook->ref);
        
        if (!lua_isfunction(lua_ctx->L, -1)) {
            lua_pop(lua_ctx->L, 1);
            hook = hook->next;
            continue;  
        }

        argc = 0;
        va_list args_copy;
        va_copy(args_copy, args);

        /* Push arguments based on hook type */
        switch (type) {
        case LUA_HOOK_CHAR_PRE:
        case LUA_HOOK_CHAR_POST:
            int_arg = va_arg(args_copy, int);  /* character */
            int_arg = va_arg(args_copy, int);  /* attributes */
            lua_pushinteger(lua_ctx->L, int_arg);
            argc = 1;
            break;

        case LUA_HOOK_KEY_PRESS:
        case LUA_HOOK_KEY_RELEASE:
            {
                int keysym = va_arg(args_copy, int);  /* keysym */
                int state = va_arg(args_copy, int);   /* state */
                lua_pushinteger(lua_ctx->L, keysym);
                lua_pushinteger(lua_ctx->L, state);
                argc = 2;
                break;
            }

        case LUA_HOOK_MOUSE_PRESS:
        case LUA_HOOK_MOUSE_RELEASE:
        case LUA_HOOK_MOUSE_MOTION:
            int_arg = va_arg(args_copy, int);  /* button */
            int_arg = va_arg(args_copy, int);  /* x */
            int_arg = va_arg(args_copy, int);  /* y */
            lua_pushinteger(lua_ctx->L, int_arg);
            argc = 1;
            break;

        case LUA_HOOK_RESIZE:
            int_arg = va_arg(args_copy, int);  /* width */
            int_arg = va_arg(args_copy, int);  /* height */
            lua_pushinteger(lua_ctx->L, int_arg);
            argc = 1;
            break;

        case LUA_HOOK_MENU_ACTION:
            str_arg = va_arg(args_copy, const char *);  /* action */
            lua_pushstring(lua_ctx->L, str_arg ? str_arg : "");
            argc = 1;
            break;

        default:
            /* No arguments for other hook types */
            break;
        }

        va_end(args_copy);

        /* Call the Lua function */
        if (lua_xterm_safe_call(lua_ctx->L, argc, 1) == LUA_OK) {
            /* Check return value for some hooks */
            if (type == LUA_HOOK_KEY_PRESS || type == LUA_HOOK_KEY_RELEASE) {
                if (lua_isboolean(lua_ctx->L, -1) && lua_toboolean(lua_ctx->L, -1)) {
                    /* Hook handled the event, stop processing */
                    lua_pop(lua_ctx->L, 1);
                    va_end(args);
                    return True;
                }
            }
            lua_pop(lua_ctx->L, 1);
        }

        hook = hook->next;
    }

    va_end(args);
    return False;
}

void
lua_xterm_clear_hooks(void)
{
    int i;
    LuaHook *hook, *next;

    if (!lua_xterm_is_enabled()) {
        return;
    }

    for (i = 0; i < LUA_HOOK_COUNT; i++) {
        hook = hook_lists[i];
        while (hook != NULL) {
            next = hook->next;
            luaL_unref(lua_ctx->L, LUA_REGISTRYINDEX, hook->ref);
            free(hook);
            hook = next;
        }
        hook_lists[i] = NULL;
    }

    lua_xterm_debug("Cleared all hooks");
}

/* Lua API functions */

int
lua_hooks_register(lua_State *L)
{
    const char *hook_name;
    LuaHookType hook_type;
    int ref;

    if (lua_gettop(L) != 2) {
        return luaL_error(L, "Usage: xterm.hooks.register(hook_name, function)");
    }

    hook_name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    hook_type = lua_hook_name_to_type(hook_name);
    if (hook_type == -1) {
        return luaL_error(L, "Unknown hook type: %s", hook_name);
    }

    /* Create reference to function */
    lua_pushvalue(L, 2);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    if (!lua_xterm_register_hook(hook_type, ref)) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        return luaL_error(L, "Failed to register hook");
    }

    return 0;
}

int
lua_hooks_unregister(lua_State *L)
{
    const char *hook_name;
    LuaHookType hook_type;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.hooks.unregister(hook_name)");
    }

    hook_name = luaL_checkstring(L, 1);
    hook_type = lua_hook_name_to_type(hook_name);
    if (hook_type == -1) {
        return luaL_error(L, "Unknown hook type: %s", hook_name);
    }

    /* For now, clear all hooks of this type */
    /* TODO: Implement selective unregistration */
    LuaHook *hook, *next;
    hook = hook_lists[hook_type];
    while (hook != NULL) {
        next = hook->next;
        luaL_unref(L, LUA_REGISTRYINDEX, hook->ref);
        free(hook);
        hook = next;
    }
    hook_lists[hook_type] = NULL;

    return 0;
}

int
lua_hooks_clear(lua_State *L)
{
    lua_xterm_clear_hooks();
    return 0;
}

int
luaopen_xterm_hooks(lua_State *L)
{
    static const luaL_Reg hooks_funcs[] = {
        {"register", lua_hooks_register},
        {"unregister", lua_hooks_unregister},
        {"clear", lua_hooks_clear},
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

    /* Create hooks subtable */
    luaL_newlib(L, hooks_funcs);
    lua_setfield(L, -2, "hooks");

    lua_pop(L, 1);  /* Pop xterm table */
    return 0;
}

static LuaHookType
lua_hook_name_to_type(const char *name)
{
    int i;

    if (name == NULL) {
        return -1;
    }

    for (i = 0; i < LUA_HOOK_COUNT; i++) {
        if (strcmp(name, hook_names[i]) == 0) {
            return (LuaHookType) i;
        }
    }

    return -1;
}

#endif /* OPT_LUA_SCRIPTING */