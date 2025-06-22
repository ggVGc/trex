/* $XTermId: lua_config.c,v 1.1 2025/06/21 00:00:00 claude Exp $ */

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
#include <data.h>
#include <xstrings.h>

/* Terminal control functions */

int
lua_terminal_write(lua_State *L)
{
    const char *text;
    size_t len;
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.terminal.write(text)");
    }

    text = luaL_checklstring(L, 1, &len);
    if (text != NULL && len > 0) {
        /* Write text to terminal */
        v_write(screen->respond, (const Char *) text, (unsigned) len);
    }

    return 0;
}

int
lua_terminal_clear(lua_State *L)
{
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);
    
    /* Send clear screen sequence */
    v_write(screen->respond, (const Char *) "\033[2J\033[H", 7);
    
    return 0;
}

int
lua_terminal_set_title(lua_State *L)
{
    const char *title;
    XtermWidget xw = term;
    
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.terminal.set_title(title)");
    }

    title = luaL_checkstring(L, 1);
    if (title != NULL) {
        /* Use xterm's built-in ChangeTitle function */
        ChangeTitle(xw, (char *) title);
    }

    return 0;
}

int
lua_terminal_bell(lua_State *L)
{
    XtermWidget xw = term;
    Bell(xw, XkbBI_TerminalBell, 0);
    return 0;
}

int
lua_terminal_goto(lua_State *L)
{
    int row, col;
    char buffer[32];
    int len;
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);

    if (lua_gettop(L) != 2) {
        return luaL_error(L, "Usage: xterm.terminal.goto(row, col)");
    }

    row = (int) luaL_checkinteger(L, 1);
    col = (int) luaL_checkinteger(L, 2);

    len = snprintf(buffer, sizeof(buffer), "\033[%d;%dH", row, col);
    if (len > 0) {
        v_write(screen->respond, (const Char *) buffer, (unsigned) len);
    }

    return 0;
}

int
lua_terminal_save_cursor(lua_State *L)
{
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);
    v_write(screen->respond, (const Char *) "\0337", 2);
    return 0;
}

int
lua_terminal_restore_cursor(lua_State *L)
{
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);
    v_write(screen->respond, (const Char *) "\0338", 2);
    return 0;
}

/* Configuration functions */

int
lua_config_get(lua_State *L)
{
    const char *key;
    XtermWidget xw = term;

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Usage: xterm.config.get(key)");
    }

    key = luaL_checkstring(L, 1);
    if (key == NULL) {
        lua_pushnil(L);
        return 1;
    }

    /* Handle common configuration keys */
    if (strcmp(key, "background") == 0) {
        /* Use a simple placeholder for now */
        lua_pushstring(L, "black");
    } else if (strcmp(key, "foreground") == 0) {
        lua_pushstring(L, "white");
    } else if (strcmp(key, "font") == 0) {
        lua_pushstring(L, xw->misc.default_font.f_n);
    } else if (strcmp(key, "geometry") == 0) {
        char geom[64];
        snprintf(geom, sizeof(geom), "%dx%d", 
                TScreenOf(xw)->max_col, TScreenOf(xw)->max_row);
        lua_pushstring(L, geom);
    } else if (strcmp(key, "title") == 0) {
        /* Use a simple placeholder for now */
        lua_pushstring(L, "xterm");
    } else {
        /* Unknown key */
        lua_pushnil(L);
    }

    return 1;
}

int
lua_config_set(lua_State *L)
{
    const char *key, *value;
    XtermWidget xw = term;

    if (lua_gettop(L) != 2) {
        return luaL_error(L, "Usage: xterm.config.set(key, value)");
    }

    key = luaL_checkstring(L, 1);
    value = luaL_checkstring(L, 2);

    if (key == NULL || value == NULL) {
        return 0;
    }

    /* Handle common configuration keys */
    if (strcmp(key, "background") == 0) {
        /* This would require more complex color handling */
        lua_xterm_debug("Background color change requested: %s", value);
    } else if (strcmp(key, "foreground") == 0) {
        lua_xterm_debug("Foreground color change requested: %s", value);
    } else if (strcmp(key, "title") == 0) {
        /* Change window title */
        ChangeTitle(xw, value);
    } else {
        lua_xterm_debug("Unknown config key: %s", key);
    }

    return 0;
}

int
lua_config_reset(lua_State *L)
{
    lua_xterm_debug("Config reset requested");
    return 0;
}

/* Menu functions */

int
lua_menu_add_item(lua_State *L)
{
    const char *menu_name, *item_name;

    if (lua_gettop(L) != 2) {
        return luaL_error(L, "Usage: xterm.menu.add_item(menu_name, item_name)");
    }

    menu_name = luaL_checkstring(L, 1);
    item_name = luaL_checkstring(L, 2);

    lua_xterm_debug("Menu item add requested: %s -> %s", menu_name, item_name);
    
    return 0;
}

int
lua_menu_remove_item(lua_State *L)
{
    const char *menu_name, *item_name;

    if (lua_gettop(L) != 2) {
        return luaL_error(L, "Usage: xterm.menu.remove_item(menu_name, item_name)");
    }

    menu_name = luaL_checkstring(L, 1);
    item_name = luaL_checkstring(L, 2);

    lua_xterm_debug("Menu item remove requested: %s -> %s", menu_name, item_name);
    
    return 0;
}

int
lua_menu_show(lua_State *L)
{
    lua_xterm_debug("Menu show requested");
    return 0;
}

/* Library registration */

int
luaopen_xterm_terminal(lua_State *L)
{
    static const luaL_Reg terminal_funcs[] = {
        {"write", lua_terminal_write},
        {"clear", lua_terminal_clear},
        {"set_title", lua_terminal_set_title},
        {"bell", lua_terminal_bell},
        {"goto", lua_terminal_goto},
        {"save_cursor", lua_terminal_save_cursor},
        {"restore_cursor", lua_terminal_restore_cursor},
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

    /* Create terminal subtable */
    luaL_newlib(L, terminal_funcs);
    lua_setfield(L, -2, "terminal");

    lua_pop(L, 1);  /* Pop xterm table */
    return 0;
}

int
luaopen_xterm_config(lua_State *L)
{
    static const luaL_Reg config_funcs[] = {
        {"get", lua_config_get},
        {"set", lua_config_set},
        {"reset", lua_config_reset},
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

    /* Create config subtable */
    luaL_newlib(L, config_funcs);
    lua_setfield(L, -2, "config");

    lua_pop(L, 1);  /* Pop xterm table */
    return 0;
}

int
luaopen_xterm_menu(lua_State *L)
{
    static const luaL_Reg menu_funcs[] = {
        {"add_item", lua_menu_add_item},
        {"remove_item", lua_menu_remove_item},
        {"show", lua_menu_show},
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

    /* Create menu subtable */
    luaL_newlib(L, menu_funcs);
    lua_setfield(L, -2, "menu");

    lua_pop(L, 1);  /* Pop xterm table */
    return 0;
}

#endif /* OPT_LUA_SCRIPTING */