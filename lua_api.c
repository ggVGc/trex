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
#include <ptyx.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

LuaContext *lua_ctx = NULL;

static void lua_xterm_sandbox(lua_State *L);
static int lua_xterm_panic(lua_State *L);
static void lua_xterm_draw_command_line(void);
static void lua_xterm_setup_trex_paths(lua_State *L);

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

    /* Set up trex runtime paths before sandbox */
    lua_xterm_setup_trex_paths(lua_ctx->L);
    
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
    
    /* Set up trex runtime paths before sandbox */
    lua_xterm_setup_trex_paths(lua_ctx->L);
    
    lua_xterm_sandbox(lua_ctx->L);

    /* Re-register libraries */
    luaopen_xterm_terminal(lua_ctx->L);
    luaopen_xterm_config(lua_ctx->L);
    luaopen_xterm_menu(lua_ctx->L);
    luaopen_xterm_events(lua_ctx->L);
    luaopen_xterm_utils(lua_ctx->L);
    luaopen_xterm_hooks(lua_ctx->L);

    /* Load trex_init module using standard require */
    lua_getglobal(lua_ctx->L, "require");
    if (lua_isfunction(lua_ctx->L, -1)) {
        lua_pushstring(lua_ctx->L, "trex_init");
        result = lua_xterm_safe_call(lua_ctx->L, 1, 0);
        if (result != LUA_OK) {
            lua_xterm_error("Failed to load trex_init module: %s", lua_tostring(lua_ctx->L, -1));
            lua_pop(lua_ctx->L, 1);
            return 0;
        }
        lua_ctx->last_reload = time(NULL);
        return 1;
    } else {
        lua_pop(lua_ctx->L, 1);
        lua_xterm_error("require function not available");
        return 0;
    }
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
    char *cwd_path;
    char *home_path;
    
    if (filename == NULL) {
        return NULL;
    }

    /* If filename is absolute, use it as-is */
    if (filename[0] == '/') {
        return x_strdup(filename);
    }

    /* First try current working directory */
    cwd_path = lua_xterm_get_cwd_script_path(filename);
    if (cwd_path != NULL && lua_xterm_file_exists(cwd_path)) {
        return cwd_path;
    }
    if (cwd_path != NULL) {
        free(cwd_path);
    }

    /* Fall back to home directory */
    home_path = lua_xterm_get_home_script_path(filename);
    return home_path;
}

char *
lua_xterm_get_cwd_script_path(const char *filename)
{
    char *cwd;
    char *script_path;
    size_t path_len;

    if (filename == NULL) {
        return NULL;
    }

    cwd = lua_xterm_get_cwd();
    if (cwd == NULL) {
        return NULL;
    }

    path_len = strlen(cwd) + strlen(lua_ctx->script_dir) + strlen(filename) + 3;
    script_path = (char *) malloc(path_len);
    if (script_path == NULL) {
        free(cwd);
        return NULL;
    }

    snprintf(script_path, path_len, "%s/%s/%s", cwd, lua_ctx->script_dir, filename);
    free(cwd);

    return script_path;
}

char *
lua_xterm_get_home_script_path(const char *filename)
{
    char *home_dir;
    char *script_path;
    size_t path_len;

    if (filename == NULL) {
        return NULL;
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

char *
lua_xterm_get_cwd(void)
{
    char *cwd;
    char buffer[4096];
    
    cwd = getcwd(buffer, sizeof(buffer));
    if (cwd != NULL) {
        return x_strdup(cwd);
    }
    return NULL;
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
    /* No sandbox restrictions - full Lua environment available */
    /* os, io, dofile, loadfile, and all standard libraries are accessible */
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
    
    /* Display the command line */
    lua_xterm_draw_command_line();
    
    /* Call command mode hook if registered */
    lua_xterm_call_hook(LUA_HOOK_COMMAND_MODE, "s", "enter");
    
    lua_xterm_debug("Entered Lua command mode");
}

void
lua_xterm_exit_command_mode(void)
{
    extern XtermWidget term;
    XtermWidget xw = term;
    TScreen *screen;
    LineData *ld;
    int col;
    
    if (!lua_ctx) {
        return;
    }
    
    /* Clear the command line before exiting */
    if (xw && lua_ctx->command_mode) {
        screen = TScreenOf(xw);
        if (screen && screen->max_row >= 0) {
            ld = getLineData(screen, screen->max_row);
            if (ld) {
                /* Clear the last line */
                for (col = 0; col <= screen->max_col; col++) {
                    ld->charData[col] = (IChar)' ';
                    ld->attribs[col] = 0;
                }
                /* Refresh the line */
                ScrnRefresh(xw, screen->max_row, 0, 1, screen->max_col + 1, True);
            }
        }
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

static void
lua_xterm_draw_command_line(void)
{
    extern XtermWidget term;
    XtermWidget xw = term;
    TScreen *screen;
    char prompt[] = "Lua> ";
    int prompt_len = 5; /* Length of "Lua> " */
    int i, col;
    LineData *ld;
    
    if (!xw || !lua_ctx || !lua_ctx->command_mode) {
        return;
    }
    
    screen = TScreenOf(xw);
    if (!screen || screen->max_row < 0) {
        return;
    }
    
    lua_xterm_debug("Drawing command line, max_row=%d, max_col=%d", 
                    screen->max_row, screen->max_col);
    
    /* Get the last line of the screen buffer */
    ld = getLineData(screen, screen->max_row);
    if (!ld) {
        lua_xterm_debug("Failed to get line data for row %d", screen->max_row);
        return;
    }
    
    lua_xterm_debug("Got line data, clearing line");
    
    /* Clear the line */
    for (col = 0; col <= screen->max_col; col++) {
        ld->charData[col] = (IChar)' ';
        ld->attribs[col] = 0;
    }
    
    /* Write prompt in reverse video (except the space) */
    for (i = 0; i < prompt_len - 1; i++) {
        ld->charData[i] = (IChar)prompt[i];
        ld->attribs[i] = INVERSE;
    }
    /* Write the space without inverse video */
    ld->charData[prompt_len - 1] = (IChar)' ';
    ld->attribs[prompt_len - 1] = 0;
    
    lua_xterm_debug("Wrote prompt, command_length=%zu", 
                    lua_ctx->command_length);
    
    /* Write command buffer */
    col = prompt_len;
    if (lua_ctx->command_buffer && lua_ctx->command_length > 0) {
        int chars_to_write = (int)lua_ctx->command_length;
        if (col + chars_to_write > screen->max_col) {
            chars_to_write = screen->max_col - col;
        }
        
        for (i = 0; i < chars_to_write; i++) {
            ld->charData[col + i] = (IChar)lua_ctx->command_buffer[i];
            ld->attribs[col + i] = 0;
        }
        col += chars_to_write;
        
        lua_xterm_debug("Wrote %d chars: '%.*s'", 
                        chars_to_write, chars_to_write, lua_ctx->command_buffer);
    }
    
    /* Add cursor indicator */
    if (col <= screen->max_col) {
        ld->charData[col] = (IChar)'_';
        ld->attribs[col] = BLINK;
    }
    
    lua_xterm_debug("Refreshing screen");
    
    /* Mark line as dirty for refresh */
    ScrnRefresh(xw, screen->max_row, 0, 1, screen->max_col + 1, True);
}

void
lua_xterm_command_mode_display(void)
{
    if (!lua_ctx || !lua_ctx->command_mode) {
        return;
    }
    
    /* Draw the command line */
    lua_xterm_draw_command_line();
    
    /* Call display hook if registered */
    lua_xterm_call_hook(LUA_HOOK_COMMAND_MODE, "ss", "display", 
                        lua_ctx->command_buffer ? lua_ctx->command_buffer : "");
}


/* Set up trex runtime paths and package configuration */
static void
lua_xterm_setup_trex_paths(lua_State *L)
{
    char *cwd = NULL;
    char *trex_path = NULL;
    size_t path_len;
    
    /* Get current working directory */
    cwd = lua_xterm_get_cwd();
    if (cwd == NULL) {
        lua_xterm_debug("Failed to get current working directory for trex paths");
        return;
    }

    /* Create paths including current working directory and runtime subdirectories */
    const char *patterns[] = {
        "/runtime/?.lua"
    };
    size_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    
    /* Calculate total length needed */
    path_len = 1; /* for null terminator */
    for (size_t i = 0; i < num_patterns; i++) {
        path_len += strlen(cwd) + strlen(patterns[i]);
        if (i > 0) path_len += 1; /* for semicolon */
    }
    
    trex_path = (char *) malloc(path_len);
    if (trex_path == NULL) {
        free(cwd);
        lua_xterm_debug("Failed to allocate memory for trex path");
        return;
    }

    /* Build the complete path string */
    trex_path[0] = '\0';
    for (size_t i = 0; i < num_patterns; i++) {
        if (i > 0) strcat(trex_path, ";");
        strcat(trex_path, cwd);
        strcat(trex_path, patterns[i]);
    }
    free(cwd);

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
    
    lua_xterm_debug("Set up trex runtime paths");
}

#endif /* OPT_LUA_SCRIPTING */
