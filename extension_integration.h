/* $XTermId: extension_integration.h,v 1.1 2025/06/23 00:00:00 claude Exp $ */

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

#ifndef included_extension_integration_h
#define included_extension_integration_h

#include <xterm.h>
#include <extension_api.h>

/* Wrapper functions to maintain compatibility with existing code */

/* Initialization and cleanup */
int extension_system_init(void);
void extension_system_cleanup(void);

/* Hook management - these replace the lua_xterm_* functions */
Boolean extension_system_call_hook(ExtensionHookType type, ...);
void extension_system_process_events(void);

/* Command mode wrappers */
Boolean extension_system_is_command_mode(void);
void extension_system_enter_command_mode(void);
void extension_system_exit_command_mode(void);
void extension_system_command_mode_input(int ch);
void extension_system_command_mode_execute(void);
void extension_system_command_mode_backspace(void);

/* Status functions */
Boolean extension_system_is_enabled(void);
void extension_system_set_enabled(Boolean enabled);

/* Compatibility macros for existing code */
#define lua_xterm_init() extension_system_init()
#define lua_xterm_cleanup() extension_system_cleanup()
#define lua_xterm_call_hook(type, ...) extension_system_call_hook(type, ##__VA_ARGS__)
#define lua_xterm_process_events() extension_system_process_events()
#define lua_xterm_is_command_mode() extension_system_is_command_mode()
#define lua_xterm_enter_command_mode() extension_system_enter_command_mode()
#define lua_xterm_exit_command_mode() extension_system_exit_command_mode()
#define lua_xterm_command_mode_input(ch) extension_system_command_mode_input(ch)
#define lua_xterm_command_mode_execute() extension_system_command_mode_execute()
#define lua_xterm_command_mode_backspace() extension_system_command_mode_backspace()
#define lua_xterm_is_enabled() extension_system_is_enabled()
#define lua_xterm_set_enabled(enabled) extension_system_set_enabled(enabled)

/* Hook type compatibility */
#define LUA_HOOK_CHAR_PRE EXT_HOOK_CHAR_PRE
#define LUA_HOOK_CHAR_POST EXT_HOOK_CHAR_POST
#define LUA_HOOK_KEY_PRESS EXT_HOOK_KEY_PRESS
#define LUA_HOOK_KEY_RELEASE EXT_HOOK_KEY_RELEASE
#define LUA_HOOK_MOUSE_PRESS EXT_HOOK_MOUSE_PRESS
#define LUA_HOOK_MOUSE_RELEASE EXT_HOOK_MOUSE_RELEASE
#define LUA_HOOK_MOUSE_MOTION EXT_HOOK_MOUSE_MOTION
#define LUA_HOOK_FOCUS_IN EXT_HOOK_FOCUS_IN
#define LUA_HOOK_FOCUS_OUT EXT_HOOK_FOCUS_OUT
#define LUA_HOOK_RESIZE EXT_HOOK_RESIZE
#define LUA_HOOK_STARTUP EXT_HOOK_STARTUP
#define LUA_HOOK_SHUTDOWN EXT_HOOK_SHUTDOWN
#define LUA_HOOK_MENU_ACTION EXT_HOOK_MENU_ACTION
#define LUA_HOOK_STATUS_UPDATE EXT_HOOK_STATUS_UPDATE
#define LUA_HOOK_COMMAND_MODE EXT_HOOK_COMMAND_MODE

#endif /* included_extension_integration_h */