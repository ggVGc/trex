/* $XTermId: extension_api.c,v 1.1 2025/06/23 00:00:00 claude Exp $ */

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
#include <extension_api.h>
#include <data.h>
#include <xstrings.h>
#include <fontutils.h>
#include <ptyx.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

ExtensionRegistry *ext_registry = NULL;

static const char *hook_names[EXT_HOOK_COUNT] = {
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

/* Forward declarations */
static void ext_terminal_write(const char *text, size_t len);
static void ext_terminal_message(const char *text);
static char *ext_terminal_get_text(void);
static void ext_terminal_clear(void);
static void ext_terminal_set_title(const char *title);
static void ext_terminal_bell(void);
static void ext_terminal_goto(int row, int col);
static void ext_terminal_save_cursor(void);
static void ext_terminal_restore_cursor(void);
static char *ext_config_get(const char *key);
static void ext_config_set(const char *key, const char *value);
static void ext_config_reset(void);
static void ext_log_message(const char *format, ...);
static void ext_debug_message(const char *format, ...);
static void ext_error_message(const char *format, ...);
static int ext_file_exists(const char *filename);
static char *ext_get_home_dir(void);
static char *ext_get_cwd(void);
static Boolean ext_is_command_mode(void);
static void ext_enter_command_mode(void);
static void ext_exit_command_mode(void);
static void ext_command_mode_input(int ch);
static void ext_command_mode_execute(void);
static const char *ext_get_command_buffer(void);
static void ext_set_enabled(ExtensionPlugin *plugin, Boolean enabled);
static Boolean ext_is_enabled(ExtensionPlugin *plugin);
static void ext_set_debug(ExtensionPlugin *plugin, Boolean debug);

/* Extension API implementation */
static ExtensionAPI extension_api = {
    /* Terminal API */
    .terminal_write = ext_terminal_write,
    .terminal_message = ext_terminal_message,
    .terminal_get_text = ext_terminal_get_text,
    .terminal_clear = ext_terminal_clear,
    .terminal_set_title = ext_terminal_set_title,
    .terminal_bell = ext_terminal_bell,
    .terminal_goto = ext_terminal_goto,
    .terminal_save_cursor = ext_terminal_save_cursor,
    .terminal_restore_cursor = ext_terminal_restore_cursor,
    
    /* Configuration API */
    .config_get = ext_config_get,
    .config_set = ext_config_set,
    .config_reset = ext_config_reset,
    
    /* Utility API */
    .log_message = ext_log_message,
    .debug_message = ext_debug_message,
    .error_message = ext_error_message,
    .file_exists = ext_file_exists,
    .get_home_dir = ext_get_home_dir,
    .get_cwd = ext_get_cwd,
    
    /* Command mode API */
    .is_command_mode = ext_is_command_mode,
    .enter_command_mode = ext_enter_command_mode,
    .exit_command_mode = ext_exit_command_mode,
    .command_mode_input = ext_command_mode_input,
    .command_mode_execute = ext_command_mode_execute,
    .get_command_buffer = ext_get_command_buffer,
    
    /* Plugin management */
    .set_enabled = ext_set_enabled,
    .is_enabled = ext_is_enabled,
    .set_debug = ext_set_debug
};

int
extension_registry_init(void)
{
    if (ext_registry != NULL) {
        return 1; /* Already initialized */
    }

    ext_registry = (ExtensionRegistry *) calloc(1, sizeof(ExtensionRegistry));
    if (ext_registry == NULL) {
        ext_error_message("Failed to allocate extension registry");
        return 0;
    }

    ext_registry->plugin_capacity = 16;
    ext_registry->plugins = (ExtensionPlugin **) calloc(ext_registry->plugin_capacity, sizeof(ExtensionPlugin *));
    if (ext_registry->plugins == NULL) {
        free(ext_registry);
        ext_registry = NULL;
        ext_error_message("Failed to allocate plugin array");
        return 0;
    }

    ext_registry->plugin_count = 0;
    ext_registry->initialized = True;

    ext_debug_message("Extension registry initialized");
    return 1;
}

void
extension_registry_cleanup(void)
{
    int i;

    if (ext_registry == NULL || !ext_registry->initialized) {
        return;
    }

    ext_debug_message("Cleaning up extension registry");

    /* Cleanup all plugins */
    for (i = 0; i < ext_registry->plugin_count; i++) {
        if (ext_registry->plugins[i] != NULL) {
            ExtensionPlugin *plugin = ext_registry->plugins[i];
            
            if (plugin->plugin_cleanup) {
                plugin->plugin_cleanup(plugin);
            }
            
            if (plugin->name) free(plugin->name);
            if (plugin->version) free(plugin->version);
            if (plugin->description) free(plugin->description);
            if (plugin->author) free(plugin->author);
            
            free(plugin);
            ext_registry->plugins[i] = NULL;
        }
    }

    free(ext_registry->plugins);
    ext_registry->plugins = NULL;
    ext_registry->plugin_count = 0;
    ext_registry->initialized = False;
    
    free(ext_registry);
    ext_registry = NULL;
}

int
extension_registry_register_plugin(ExtensionPlugin *plugin)
{
    if (ext_registry == NULL || !ext_registry->initialized || plugin == NULL) {
        return 0;
    }

    /* Check if we need to grow the array */
    if (ext_registry->plugin_count >= ext_registry->plugin_capacity) {
        int new_capacity = ext_registry->plugin_capacity * 2;
        ExtensionPlugin **new_plugins = (ExtensionPlugin **) realloc(
            ext_registry->plugins, 
            new_capacity * sizeof(ExtensionPlugin *)
        );
        if (new_plugins == NULL) {
            ext_error_message("Failed to grow plugin array");
            return 0;
        }
        ext_registry->plugins = new_plugins;
        ext_registry->plugin_capacity = new_capacity;
    }

    /* Add plugin to registry */
    ext_registry->plugins[ext_registry->plugin_count] = plugin;
    ext_registry->plugin_count++;

    /* Set API reference */
    plugin->api = &extension_api;

    /* Initialize plugin */
    if (plugin->plugin_init) {
        if (plugin->plugin_init(plugin, &extension_api) != 0) {
            plugin->initialized = True;
            ext_debug_message("Initialized plugin: %s", plugin->name ? plugin->name : "unknown");
        } else {
            ext_error_message("Failed to initialize plugin: %s", plugin->name ? plugin->name : "unknown");
            return 0;
        }
    }

    return 1;
}

void
extension_registry_unregister_plugin(const char *name)
{
    int i, j;
    ExtensionPlugin *plugin;

    if (ext_registry == NULL || !ext_registry->initialized || name == NULL) {
        return;
    }

    for (i = 0; i < ext_registry->plugin_count; i++) {
        plugin = ext_registry->plugins[i];
        if (plugin != NULL && plugin->name != NULL && strcmp(plugin->name, name) == 0) {
            /* Cleanup plugin */
            if (plugin->plugin_cleanup) {
                plugin->plugin_cleanup(plugin);
            }
            
            if (plugin->name) free(plugin->name);
            if (plugin->version) free(plugin->version);
            if (plugin->description) free(plugin->description);
            if (plugin->author) free(plugin->author);
            
            free(plugin);

            /* Shift remaining plugins */
            for (j = i; j < ext_registry->plugin_count - 1; j++) {
                ext_registry->plugins[j] = ext_registry->plugins[j + 1];
            }
            ext_registry->plugin_count--;
            ext_registry->plugins[ext_registry->plugin_count] = NULL;

            ext_debug_message("Unregistered plugin: %s", name);
            return;
        }
    }
}

ExtensionPlugin *
extension_registry_find_plugin(const char *name)
{
    int i;

    if (ext_registry == NULL || !ext_registry->initialized || name == NULL) {
        return NULL;
    }

    for (i = 0; i < ext_registry->plugin_count; i++) {
        ExtensionPlugin *plugin = ext_registry->plugins[i];
        if (plugin != NULL && plugin->name != NULL && strcmp(plugin->name, name) == 0) {
            return plugin;
        }
    }

    return NULL;
}

void
extension_registry_enable_plugin(const char *name, Boolean enabled)
{
    ExtensionPlugin *plugin = extension_registry_find_plugin(name);
    if (plugin != NULL) {
        plugin->enabled = enabled;
        ext_debug_message("Plugin %s %s", name, enabled ? "enabled" : "disabled");
    }
}

void
extension_registry_reload_plugins(void)
{
    int i;

    if (ext_registry == NULL || !ext_registry->initialized) {
        return;
    }

    ext_debug_message("Reloading all plugins");

    for (i = 0; i < ext_registry->plugin_count; i++) {
        ExtensionPlugin *plugin = ext_registry->plugins[i];
        if (plugin != NULL && plugin->enabled && plugin->plugin_reload) {
            if (plugin->plugin_reload(plugin) != 0) {
                ext_debug_message("Reloaded plugin: %s", plugin->name ? plugin->name : "unknown");
            } else {
                ext_error_message("Failed to reload plugin: %s", plugin->name ? plugin->name : "unknown");
            }
        }
    }
}

Boolean
extension_call_hook(ExtensionHookType type, ...)
{
    int i;
    va_list args;
    Boolean handled = False;

    if (ext_registry == NULL || !ext_registry->initialized || type < 0 || type >= EXT_HOOK_COUNT) {
        return False;
    }

    va_start(args, type);

    for (i = 0; i < ext_registry->plugin_count; i++) {
        ExtensionPlugin *plugin = ext_registry->plugins[i];
        if (plugin != NULL && plugin->enabled && plugin->initialized && plugin->plugin_call_hook) {
            va_list args_copy;
            va_copy(args_copy, args);
            
            if (plugin->plugin_call_hook(plugin, type, args_copy)) {
                handled = True;
            }
            
            va_end(args_copy);
        }
    }

    va_end(args);
    return handled;
}

void
extension_clear_hooks(void)
{
    ext_debug_message("Clearing all extension hooks");
    /* Individual plugins handle their own hook cleanup */
}

const char *
extension_hook_type_name(ExtensionHookType type)
{
    if (type >= 0 && type < EXT_HOOK_COUNT) {
        return hook_names[type];
    }
    return "unknown";
}

ExtensionHookType
extension_hook_name_to_type(const char *name)
{
    int i;

    if (name == NULL) {
        return -1;
    }

    for (i = 0; i < EXT_HOOK_COUNT; i++) {
        if (strcmp(name, hook_names[i]) == 0) {
            return (ExtensionHookType) i;
        }
    }

    return -1;
}

void
extension_process_events(void)
{
    /* This function is called from the main event loop */
    if (ext_registry == NULL || !ext_registry->initialized) {
        return;
    }

    /* Process any pending extension events */
    /* For now, just check for reloads */
    extension_check_reload();
}

void
extension_check_reload(void)
{
    static time_t last_check = 0;
    time_t now = time(NULL);

    /* Check only once per second */
    if (now - last_check < 1) {
        return;
    }
    last_check = now;

    /* Individual plugins can implement their own reload checking */
}

/* Terminal API implementations */
static void
ext_terminal_write(const char *text, size_t len)
{
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);

    if (text != NULL && len > 0) {
        v_write(screen->respond, (const Char *) text, (unsigned) len);
    }
}

static void
ext_terminal_message(const char *text)
{
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);
    IChar *buffer;
    size_t len, i;

    if (text == NULL) return;
    
    len = strlen(text);
    if (len == 0) return;

    /* Move to beginning of line and then to next line */
    CarriageReturn(xw);
    xtermIndex(xw, 1);
    
    /* Convert text to IChar format for dotext */
    buffer = malloc(len * sizeof(IChar));
    if (buffer != NULL) {
        for (i = 0; i < len; i++) {
            buffer[i] = (IChar)(unsigned char)text[i];
        }
        
        /* Write message as terminal output (not shell input) */
        dotext(xw, screen->gsets[(int)(screen->curgl)], buffer, (Cardinal)len);
        
        free(buffer);
    }
    
    /* Move to next line for new shell prompt */
    CarriageReturn(xw);
    xtermIndex(xw, 1);
}

static char *
ext_terminal_get_text(void)
{
    /* Simplified implementation - return empty string for now */
    return x_strdup("");
}

static void
ext_terminal_clear(void)
{
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);
    
    v_write(screen->respond, (const Char *) "\033[2J\033[H", 7);
}

static void
ext_terminal_set_title(const char *title)
{
    XtermWidget xw = term;
    
    if (title != NULL) {
        ChangeTitle(xw, (char *) title);
    }
}

static void
ext_terminal_bell(void)
{
    XtermWidget xw = term;
    Bell(xw, XkbBI_TerminalBell, 0);
}

static void
ext_terminal_goto(int row, int col)
{
    char buffer[32];
    int len;
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);

    len = snprintf(buffer, sizeof(buffer), "\033[%d;%dH", row, col);
    if (len > 0) {
        v_write(screen->respond, (const Char *) buffer, (unsigned) len);
    }
}

static void
ext_terminal_save_cursor(void)
{
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);
    v_write(screen->respond, (const Char *) "\0337", 2);
}

static void
ext_terminal_restore_cursor(void)
{
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);
    v_write(screen->respond, (const Char *) "\0338", 2);
}

/* Configuration API implementations */
static char *
ext_config_get(const char *key)
{
    XtermWidget xw = term;

    if (key == NULL) {
        return NULL;
    }

    /* Handle common configuration keys */
    if (strcmp(key, "background") == 0) {
        return x_strdup("black");
    } else if (strcmp(key, "foreground") == 0) {
        return x_strdup("white");
    } else if (strcmp(key, "font") == 0) {
        return x_strdup(xw->misc.default_font.f_n);
    } else if (strcmp(key, "geometry") == 0) {
        char geom[64];
        snprintf(geom, sizeof(geom), "%dx%d", 
                TScreenOf(xw)->max_col, TScreenOf(xw)->max_row);
        return x_strdup(geom);
    } else if (strcmp(key, "title") == 0) {
        return x_strdup("xterm");
    }

    return NULL;
}

static void
ext_config_set(const char *key, const char *value)
{
    XtermWidget xw = term;

    if (key == NULL || value == NULL) {
        return;
    }

    if (strcmp(key, "title") == 0) {
        ChangeTitle(xw, (char *) value);
    } else {
        ext_debug_message("Config set: %s = %s", key, value);
    }
}

static void
ext_config_reset(void)
{
    ext_debug_message("Config reset requested");
}

/* Utility API implementations */
static void
ext_log_message(const char *format, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    fprintf(stderr, "xterm-ext: %s\n", buffer);
}

static void
ext_debug_message(const char *format, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    fprintf(stderr, "xterm-ext: DEBUG: %s\n", buffer);
}

static void
ext_error_message(const char *format, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    fprintf(stderr, "xterm-ext: ERROR: %s\n", buffer);
}

static int
ext_file_exists(const char *filename)
{
    struct stat st;
    return (stat(filename, &st) == 0);
}

static char *
ext_get_home_dir(void)
{
    char *home = getenv("HOME");
    if (home != NULL) {
        return x_strdup(home);
    }
    return x_strdup("/tmp");
}

static char *
ext_get_cwd(void)
{
    char buffer[4096];
    char *cwd = getcwd(buffer, sizeof(buffer));
    if (cwd != NULL) {
        return x_strdup(cwd);
    }
    return NULL;
}

/* Command mode stubs - will be implemented by plugins */
static Boolean
ext_is_command_mode(void)
{
    return False;
}

static void
ext_enter_command_mode(void)
{
    ext_debug_message("Command mode entry requested");
}

static void
ext_exit_command_mode(void)
{
    ext_debug_message("Command mode exit requested");
}

static void
ext_command_mode_input(int ch)
{
    ext_debug_message("Command mode input: %c", ch);
}

static void
ext_command_mode_execute(void)
{
    ext_debug_message("Command mode execute requested");
}

static const char *
ext_get_command_buffer(void)
{
    return "";
}

static void
ext_set_enabled(ExtensionPlugin *plugin, Boolean enabled)
{
    if (plugin != NULL) {
        plugin->enabled = enabled;
    }
}

static Boolean
ext_is_enabled(ExtensionPlugin *plugin)
{
    return (plugin != NULL && plugin->enabled);
}

static void
ext_set_debug(ExtensionPlugin *plugin, Boolean debug)
{
    if (plugin != NULL) {
        plugin->debug = debug;
    }
}