#ifndef INCLUDES_H_XAHWLRG4
#define INCLUDES_H_XAHWLRG4

#include <version.h>
#include <xterm.h>

#ifdef OPT_LUA_SCRIPTING
#include <lua_api.h>
#endif

#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xmu/Converters.h>

#if OPT_INPUT_METHOD

#if defined(HAVE_LIB_XAW)
#include <X11/Xaw/XawImP.h>
#elif defined(HAVE_LIB_XAW3D)
#include <X11/Xaw3d/XawImP.h>
#elif defined(HAVE_LIB_XAW3DXFT)
#include <X11/Xaw3dxft/XawImP.h>
#elif defined(HAVE_LIB_NEXTAW)
#include <X11/neXtaw/XawImP.h>
#elif defined(HAVE_LIB_XAWPLUS)
#include <X11/XawPlus/XawImP.h>
#endif

#endif

#if OPT_WIDE_CHARS
#include <xutf8.h>
#include <wcwidth.h>
#include <precompose.h>
#ifdef HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif
#endif

#if USE_DOUBLE_BUFFER
#include <X11/extensions/Xdbe.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#if defined(HAVE_SCHED_YIELD)
#include <sched.h>
#endif

#include <VTparse.h>
#include <data.h>
#include <error.h>
#include <menu.h>
#include <main.h>
#include <fontutils.h>
#include <charclass.h>
#include <xstrings.h>
#include <graphics.h>
#include <graphics_sixel.h>

#ifdef NO_LEAKS
#include <xtermcap.h>
#endif

#endif /* end of include guard: INCLUDES_H_XAHWLRG4 */
