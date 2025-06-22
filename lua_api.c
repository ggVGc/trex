/* $XTermId: lua_api.c,v 1.1 2025/06/21 00:00:00 claude Exp $ */

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
#include <fontutils.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>

LuaContext *lua_ctx = NULL;

static void lua_xterm_sandbox(lua_State *L);
static int lua_xterm_panic(lua_State *L);

int
lua_xterm_init(void)
{
    if (lua_ctx != NULL) {
        return 1; /* Already initialized */
    }

    lua_ctx = (LuaContext *) calloc(1, sizeof(LuaContext));
    if (lua_ctx == NULL) {
        lua_xterm_error("Failed to allocate Lua context");
        return 0;
    }

    lua_ctx->L = luaL_newstate();
    if (lua_ctx->L == NULL) {
        lua_xterm_error("Failed to create Lua state");
        free(lua_ctx);
        lua_ctx = NULL;
        return 0;
    }

    /* Set panic function */
    lua_atpanic(lua_ctx->L, lua_xterm_panic);

    /* Open standard libraries */
    luaL_openlibs(lua_ctx->L);

    /* Apply sandbox restrictions */
    lua_xterm_sandbox(lua_ctx->L);

    /* Register xterm-specific libraries */
    luaopen_xterm_terminal(lua_ctx->L);
    luaopen_xterm_config(lua_ctx->L);
    luaopen_xterm_menu(lua_ctx->L);
    luaopen_xterm_events(lua_ctx->L);
    luaopen_xterm_utils(lua_ctx->L);
    luaopen_xterm_hooks(lua_ctx->L);

    /* Set default values */
    lua_ctx->enabled = True;
    lua_ctx->debug = False;
    lua_ctx->script_dir = x_strdup(LUA_SCRIPT_DIR_DEFAULT);
    lua_ctx->init_script = x_strdup(LUA_INIT_SCRIPT);
    lua_ctx->error_count = 0;
    lua_ctx->last_reload = time(NULL);
    lua_ctx->initialized = 1;
    lua_ctx->command_mode = False;
    lua_ctx->command_buffer = NULL;
    lua_ctx->command_length = 0;
    lua_ctx->command_capacity = 0;

    lua_xterm_debug("Lua scripting initialized");

    /* Load initial script if it exists */
    lua_xterm_reload_scripts();

    return 1;
}

void
lua_xterm_cleanup(void)
{
    if (lua_ctx == NULL || !lua_ctx->initialized) {
        return;
    }

    lua_xterm_debug("Cleaning up Lua scripting");

    if (lua_ctx->L) {
        lua_close(lua_ctx->L);
        lua_ctx->L = NULL;
    }

    if (lua_ctx->script_dir) {
        free(lua_ctx->script_dir);
        lua_ctx->script_dir = NULL;
    }

    if (lua_ctx->init_script) {
        free(lua_ctx->init_script);
        lua_ctx->init_script = NULL;
    }

    if (lua_ctx->command_buffer) {
        free(lua_ctx->command_buffer);
        lua_ctx->command_buffer = NULL;
    }

    lua_ctx->initialized = 0;
    free(lua_ctx);
    lua_ctx = NULL;
}

int
lua_xterm_load_script(const char *filename)
{
    FILE *fp;
    struct stat st;
    char *script_path;
    int result;

    if (lua_ctx == NULL || !lua_ctx->initialized || !lua_ctx->enabled) {
        return 0;
    }

    script_path = lua_xterm_get_script_path(filename);
    if (script_path == NULL) {
        return 0;
    }

    /* Check if file exists and get size */
    if (stat(script_path, &st) != 0) {
        lua_xterm_debug("Script file not found: %s", script_path);
        free(script_path);
        return 0;
    }

    /* Check file size */
    if (st.st_size > LUA_MAX_SCRIPT_SIZE) {
        lua_xterm_error("Script file too large: %s (%ld bytes)", 
                       script_path, (long)st.st_size);
        free(script_path);
        return 0;
    }

    /* Load and execute script */
    result = luaL_loadfile(lua_ctx->L, script_path);
    if (result != LUA_OK) {
        lua_xterm_error("Failed to load script %s: %s", 
                       script_path, lua_tostring(lua_ctx->L, -1));
        lua_pop(lua_ctx->L, 1);
        free(script_path);
        return 0;
    }

    result = lua_xterm_safe_call(lua_ctx->L, 0, 0);
    if (result != LUA_OK) {
        lua_xterm_error("Failed to execute script %s", script_path);
        free(script_path);
        return 0;
    }

    lua_xterm_debug("Loaded script: %s", script_path);
    free(script_path);
    return 1;
}

int
lua_xterm_reload_scripts(void)
{
    char *init_path;
    int result = 0;

    if (lua_ctx == NULL || !lua_ctx->initialized) {
        return 0;
    }

    lua_xterm_debug("Reloading Lua scripts");

    /* Clear existing hooks */
    lua_xterm_clear_hooks();

    /* Reset Lua state */
    lua_close(lua_ctx->L);
    lua_ctx->L = luaL_newstate();
    if (lua_ctx->L == NULL) {
        lua_xterm_error("Failed to recreate Lua state");
        return 0;
    }

    lua_atpanic(lua_ctx->L, lua_xterm_panic);
    luaL_openlibs(lua_ctx->L);
    lua_xterm_sandbox(lua_ctx->L);

    /* Re-register libraries */
    luaopen_xterm_terminal(lua_ctx->L);
    luaopen_xterm_config(lua_ctx->L);
    luaopen_xterm_menu(lua_ctx->L);
    luaopen_xterm_events(lua_ctx->L);
    luaopen_xterm_utils(lua_ctx->L);
    luaopen_xterm_hooks(lua_ctx->L);

    /* Load init script */
    result = lua_xterm_load_script(lua_ctx->init_script);
    lua_ctx->last_reload = time(NULL);

    return result;
}

void
lua_xterm_set_enabled(Boolean enabled)
{
    if (lua_ctx != NULL) {
        lua_ctx->enabled = enabled;
        lua_xterm_debug("Lua scripting %s", enabled ? "enabled" : "disabled");
    }
}

Boolean
lua_xterm_is_enabled(void)
{
    return (lua_ctx != NULL && lua_ctx->initialized && lua_ctx->enabled);
}

void
lua_xterm_error(const char *format, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    fprintf(stderr, "xterm-lua: ERROR: %s\n", buffer);

    if (lua_ctx != NULL) {
        lua_ctx->error_count++;
        /* Disable after too many errors */
        if (lua_ctx->error_count > 10) {
            lua_xterm_error("Too many Lua errors, disabling scripting");
            lua_ctx->enabled = False;
        }
    }
}

void
lua_xterm_debug(const char *format, ...)
{
    va_list args;
    char buffer[1024];

    if (lua_ctx == NULL || !lua_ctx->debug) {
        return;
    }

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    fprintf(stderr, "xterm-lua: DEBUG: %s\n", buffer);
}

int
lua_xterm_safe_call(lua_State *L, int nargs, int nresults)
{
    int result = lua_pcall(L, nargs, nresults, 0);
    if (result != LUA_OK) {
        lua_xterm_error("Lua error: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    return result;
}

char *
lua_xterm_get_script_path(const char *filename)
{
    char *home_dir;
    char *script_path;
    size_t path_len;

    if (filename == NULL) {
        return NULL;
    }

    /* If filename is absolute, use it as-is */
    if (filename[0] == '/') {
        return x_strdup(filename);
    }

    home_dir = lua_xterm_get_home_dir();
    if (home_dir == NULL) {
        return NULL;
    }

    path_len = strlen(home_dir) + strlen(lua_ctx->script_dir) + strlen(filename) + 3;
    script_path = (char *) malloc(path_len);
    if (script_path == NULL) {
        free(home_dir);
        return NULL;
    }

    snprintf(script_path, path_len, "%s/%s/%s", home_dir, lua_ctx->script_dir, filename);
    free(home_dir);

    return script_path;
}

int
lua_xterm_file_exists(const char *filename)
{
    struct stat st;
    return (stat(filename, &st) == 0);
}

char *
lua_xterm_get_home_dir(void)
{
    char *home = getenv("HOME");
    if (home != NULL) {
        return x_strdup(home);
    }
    return x_strdup("/tmp");
}

void
lua_xterm_set_script_dir(const char *dir)
{
    if (lua_ctx != NULL && dir != NULL) {
        if (lua_ctx->script_dir) {
            free(lua_ctx->script_dir);
        }
        lua_ctx->script_dir = x_strdup(dir);
    }
}

void
lua_xterm_set_init_script(const char *script)
{
    if (lua_ctx != NULL && script != NULL) {
        if (lua_ctx->init_script) {
            free(lua_ctx->init_script);
        }
        lua_ctx->init_script = x_strdup(script);
    }
}

void
lua_xterm_set_debug(Boolean debug)
{
    if (lua_ctx != NULL) {
        lua_ctx->debug = debug;
    }
}

void
lua_xterm_process_events(void)
{
    /* This function is called from the main event loop
     * to handle any pending Lua events or maintenance */
    if (!lua_xterm_is_enabled()) {
        return;
    }

    /* Check for script reload if needed */
    lua_xterm_check_reload();
}

void
lua_xterm_check_reload(void)
{
    static time_t last_check = 0;
    time_t now = time(NULL);
    char *init_path;
    struct stat st;

    /* Check only once per second */
    if (now - last_check < 1) {
        return;
    }
    last_check = now;

    if (!lua_xterm_is_enabled()) {
        return;
    }

    init_path = lua_xterm_get_script_path(lua_ctx->init_script);
    if (init_path != NULL && stat(init_path, &st) == 0) {
        if (st.st_mtime > lua_ctx->last_reload) {
            lua_xterm_debug("Script file changed, reloading");
            lua_xterm_reload_scripts();
        }
        free(init_path);
    }
}

static void
lua_xterm_sandbox(lua_State *L)
{
    /* Remove dangerous functions */
    lua_pushnil(L);
    lua_setglobal(L, "os");
    
    lua_pushnil(L);
    lua_setglobal(L, "io");
    
    lua_pushnil(L);
    lua_setglobal(L, "require");
    
    lua_pushnil(L);
    lua_setglobal(L, "dofile");
    
    lua_pushnil(L);
    lua_setglobal(L, "loadfile");
    
    /* Create a safe os table with limited functions */
    lua_newtable(L);
    
    lua_pushcfunction(L, lua_utils_timestamp);
    lua_setfield(L, -2, "time");
    
    lua_setglobal(L, "os");
}

static int
lua_xterm_panic(lua_State *L)
{
    lua_xterm_error("Lua panic: %s", lua_tostring(L, -1));
    return 0;
}

/* Command mode implementation */

Boolean
lua_xterm_is_command_mode(void)
{
    return (lua_ctx && lua_ctx->command_mode);
}

void
lua_xterm_enter_command_mode(void)
{
    if (!lua_ctx || !lua_ctx->enabled) {
        return;
    }
    
    lua_ctx->command_mode = True;
    lua_xterm_command_mode_clear();
    
    /* Call command mode hook if registered */
    lua_xterm_call_hook(LUA_HOOK_COMMAND_MODE, "s", "enter");
    
    lua_xterm_debug("Entered Lua command mode");
}

void
lua_xterm_exit_command_mode(void)
{
    if (!lua_ctx) {
        return;
    }
    
    lua_ctx->command_mode = False;
    lua_xterm_command_mode_clear();
    
    /* Call command mode hook if registered */
    lua_xterm_call_hook(LUA_HOOK_COMMAND_MODE, "s", "exit");
    
    lua_xterm_debug("Exited Lua command mode");
}

void
lua_xterm_command_mode_input(int ch)
{
    if (!lua_ctx || !lua_ctx->command_mode) {
        return;
    }
    
    /* Ensure buffer exists */
    if (!lua_ctx->command_buffer) {
        lua_ctx->command_capacity = 256;
        lua_ctx->command_buffer = (char *) malloc(lua_ctx->command_capacity);
        if (!lua_ctx->command_buffer) {
            lua_xterm_error("Failed to allocate command buffer");
            return;
        }
        lua_ctx->command_length = 0;
        lua_ctx->command_buffer[0] = '\0';
    }
    
    /* Grow buffer if needed */
    if (lua_ctx->command_length + 2 >= lua_ctx->command_capacity) {
        size_t new_capacity = lua_ctx->command_capacity * 2;
        char *new_buffer = (char *) realloc(lua_ctx->command_buffer, new_capacity);
        if (!new_buffer) {
            lua_xterm_error("Failed to grow command buffer");
            return;
        }
        lua_ctx->command_buffer = new_buffer;
        lua_ctx->command_capacity = new_capacity;
    }
    
    /* Add character to buffer */
    lua_ctx->command_buffer[lua_ctx->command_length++] = (char) ch;
    lua_ctx->command_buffer[lua_ctx->command_length] = '\0';
    
    /* Update display */
    lua_xterm_command_mode_display();
}

void
lua_xterm_command_mode_backspace(void)
{
    if (!lua_ctx || !lua_ctx->command_mode || !lua_ctx->command_buffer) {
        return;
    }
    
    if (lua_ctx->command_length > 0) {
        lua_ctx->command_length--;
        lua_ctx->command_buffer[lua_ctx->command_length] = '\0';
        lua_xterm_command_mode_display();
    }
}

void
lua_xterm_command_mode_execute(void)
{
    if (!lua_ctx || !lua_ctx->command_mode || !lua_ctx->command_buffer) {
        return;
    }
    
    if (lua_ctx->command_length == 0) {
        lua_xterm_exit_command_mode();
        return;
    }
    
    lua_xterm_debug("Executing Lua command: %s", lua_ctx->command_buffer);
    
    /* Execute the command as Lua code */
    int status = luaL_loadstring(lua_ctx->L, lua_ctx->command_buffer);
    if (status == LUA_OK) {
        status = lua_xterm_safe_call(lua_ctx->L, 0, 0);
    }
    
    if (status != LUA_OK) {
        const char *error = lua_tostring(lua_ctx->L, -1);
        lua_xterm_error("Command error: %s", error ? error : "unknown error");
        lua_pop(lua_ctx->L, 1);
    }
    
    /* Exit command mode after execution */
    lua_xterm_exit_command_mode();
}

void
lua_xterm_command_mode_clear(void)
{
    if (!lua_ctx || !lua_ctx->command_buffer) {
        return;
    }
    
    lua_ctx->command_length = 0;
    lua_ctx->command_buffer[0] = '\0';
}

const char *
lua_xterm_get_command_buffer(void)
{
    if (!lua_ctx || !lua_ctx->command_buffer) {
        return "";
    }
    
    return lua_ctx->command_buffer;
}

void
lua_xterm_command_mode_display(void)
{
    /* This function should update the terminal display with the current command */
    /* The actual implementation will depend on xterm's display system */
    if (!lua_ctx || !lua_ctx->command_mode) {
        return;
    }
    
    /* Call display hook if registered */
    lua_xterm_call_hook(LUA_HOOK_COMMAND_MODE, "ss", "display", 
                        lua_ctx->command_buffer ? lua_ctx->command_buffer : "");
}

#endif /* OPT_LUA_SCRIPTING */