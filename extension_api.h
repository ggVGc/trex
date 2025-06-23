/* $XTermId: extension_api.h,v 1.1 2025/06/23 00:00:00 claude Exp $ */

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

#ifndef included_extension_api_h
#define included_extension_api_h

#include <xterm.h>

typedef enum {
    EXT_HOOK_CHAR_PRE,
    EXT_HOOK_CHAR_POST,
    EXT_HOOK_KEY_PRESS,
    EXT_HOOK_KEY_RELEASE,
    EXT_HOOK_MOUSE_PRESS,
    EXT_HOOK_MOUSE_RELEASE,
    EXT_HOOK_MOUSE_MOTION,
    EXT_HOOK_FOCUS_IN,
    EXT_HOOK_FOCUS_OUT,
    EXT_HOOK_RESIZE,
    EXT_HOOK_STARTUP,
    EXT_HOOK_SHUTDOWN,
    EXT_HOOK_MENU_ACTION,
    EXT_HOOK_STATUS_UPDATE,
    EXT_HOOK_COMMAND_MODE,
    EXT_HOOK_COUNT
} ExtensionHookType;

typedef struct ExtensionPlugin ExtensionPlugin;

typedef struct {
    /* Plugin lifecycle */
    int (*init)(ExtensionPlugin *plugin);
    void (*cleanup)(ExtensionPlugin *plugin);
    int (*reload)(ExtensionPlugin *plugin);
    
    /* Hook registration */
    int (*register_hook)(ExtensionPlugin *plugin, ExtensionHookType type, void *handler_data);
    void (*unregister_hook)(ExtensionPlugin *plugin, ExtensionHookType type, void *handler_data);
    Boolean (*call_hook)(ExtensionPlugin *plugin, ExtensionHookType type, ...);
    
    /* Terminal API */
    void (*terminal_write)(const char *text, size_t len);
    void (*terminal_message)(const char *text);
    char *(*terminal_get_text)(void);
    void (*terminal_clear)(void);
    void (*terminal_set_title)(const char *title);
    void (*terminal_bell)(void);
    void (*terminal_goto)(int row, int col);
    void (*terminal_save_cursor)(void);
    void (*terminal_restore_cursor)(void);
    
    /* Configuration API */
    char *(*config_get)(const char *key);
    void (*config_set)(const char *key, const char *value);
    void (*config_reset)(void);
    
    /* Utility API */
    void (*log_message)(const char *format, ...);
    void (*debug_message)(const char *format, ...);
    void (*error_message)(const char *format, ...);
    int (*file_exists)(const char *filename);
    char *(*get_home_dir)(void);
    char *(*get_cwd)(void);
    
    /* Command mode API */
    Boolean (*is_command_mode)(void);
    void (*enter_command_mode)(void);
    void (*exit_command_mode)(void);
    void (*command_mode_input)(int ch);
    void (*command_mode_execute)(void);
    const char *(*get_command_buffer)(void);
    
    /* Plugin management */
    void (*set_enabled)(ExtensionPlugin *plugin, Boolean enabled);
    Boolean (*is_enabled)(ExtensionPlugin *plugin);
    void (*set_debug)(ExtensionPlugin *plugin, Boolean debug);
} ExtensionAPI;

struct ExtensionPlugin {
    /* Plugin information */
    char *name;
    char *version;
    char *description;
    char *author;
    
    /* Plugin state */
    Boolean initialized;
    Boolean enabled;
    Boolean debug;
    int error_count;
    void *plugin_data;
    
    /* API access */
    const ExtensionAPI *api;
    
    /* Plugin implementation */
    int (*plugin_init)(ExtensionPlugin *plugin, const ExtensionAPI *api);
    void (*plugin_cleanup)(ExtensionPlugin *plugin);
    int (*plugin_reload)(ExtensionPlugin *plugin);
    Boolean (*plugin_call_hook)(ExtensionPlugin *plugin, ExtensionHookType type, ...);
};

typedef struct ExtensionRegistry {
    ExtensionPlugin **plugins;
    int plugin_count;
    int plugin_capacity;
    Boolean initialized;
} ExtensionRegistry;

/* Extension registry management */
extern ExtensionRegistry *ext_registry;

/* Core extension API functions */
int extension_registry_init(void);
void extension_registry_cleanup(void);
int extension_registry_register_plugin(ExtensionPlugin *plugin);
void extension_registry_unregister_plugin(const char *name);
ExtensionPlugin *extension_registry_find_plugin(const char *name);
void extension_registry_enable_plugin(const char *name, Boolean enabled);
void extension_registry_reload_plugins(void);

/* Hook management */
Boolean extension_call_hook(ExtensionHookType type, ...);
void extension_clear_hooks(void);

/* Plugin loading */
int extension_load_plugin_dir(const char *dir_path);
int extension_load_plugin(const char *plugin_path);

/* Utility functions */
const char *extension_hook_type_name(ExtensionHookType type);
ExtensionHookType extension_hook_name_to_type(const char *name);

/* Extension process events */
void extension_process_events(void);
void extension_check_reload(void);

#endif /* included_extension_api_h */