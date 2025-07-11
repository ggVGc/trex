/* $XTermId: main.c,v 1.939 2025/06/08 23:04:16 tom Exp $ */

/*
 * Copyright 2002-2024,2025 by Thomas E. Dickey
 * Copyright 1987, 1988  X Consortium
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
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization.
 */

/*
 * Copyright 1987, 1988 by Digital Equipment Corporation, Maynard.
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *				 W A R N I N G
 *
 * If you think you know what all of this code is doing, you are
 * probably very mistaken.  There be serious and nasty dragons here.
 *
 * This client is *not* to be taken as an example of how to write X
 * Toolkit applications.  It is in need of a substantial rewrite,
 * ideally to create a generic tty widget with several different parsing
 * widgets so that you can plug 'em together any way you want.  Don't
 * hold your breath, though....
 */

/* main.c */

#define RES_OFFSET(field)	XtOffsetOf(XTERM_RESOURCE, field)

#include <xterm.h>
#include <version.h>
#include <graphics.h>

#ifdef OPT_LUA_SCRIPTING
#include <lua_api.h>
#endif

/* xterm uses these X Toolkit resource names, which are exported in array */
#undef XtNborderWidth
#undef XtNiconName
#undef XtNgeometry
#undef XtNreverseVideo
#undef XtNtitle

#define XtNborderWidth		"borderWidth"
#define XtNgeometry		"geometry"
#define XtNiconName		"iconName"
#define XtNreverseVideo		"reverseVideo"
#define XtNtitle		"title"

#if OPT_TOOLBAR

#if defined(HAVE_LIB_XAW)
#include <X11/Xaw/Form.h>
#elif defined(HAVE_LIB_XAW3D)
#include <X11/Xaw3d/Form.h>
#elif defined(HAVE_LIB_XAW3DXFT)
#include <X11/Xaw3dxft/Form.h>
#include <X11/Xaw3dxft/Xaw3dXft.h>
#elif defined(HAVE_LIB_NEXTAW)
#include <X11/neXtaw/Form.h>
#elif defined(HAVE_LIB_XAWPLUS)
#include <X11/XawPlus/Form.h>
#endif

#else

#if defined(HAVE_LIB_XAW3DXFT)
#include <X11/Xaw3dxft/Xaw3dXft.h>
#endif

#endif /* OPT_TOOLBAR */

#include <pwd.h>
#include <ctype.h>

#include <data.h>
#include <error.h>
#include <menu.h>
#include <main.h>
#include <xstrings.h>
#include <xtermcap.h>
#include <xterm_io.h>

#if OPT_WIDE_CHARS
#include <charclass.h>
#endif

#ifdef __osf__
#define USE_SYSV_SIGNALS
#define WTMP
#include <pty.h>		/* openpty() */
#endif

#ifdef __sgi
#include <grp.h>		/* initgroups() */
#endif

static GCC_NORETURN void hungtty(int);
static GCC_NORETURN void Syntax(char *);
static GCC_NORETURN void HsSysError(int);

#if defined(__SCO__) || defined(SVR4) || defined(_POSIX_VERSION) || ( defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 1) )
#define USE_POSIX_SIGNALS
#endif

#if defined(SYSV) && !defined(SVR4) && !defined(ISC22) && !defined(ISC30)
/* older SYSV systems cannot ignore SIGHUP.
   Shell hangs, or you get extra shells, or something like that */
#define USE_SYSV_SIGHUP
#endif

#if defined(sony) && defined(bsd43) && !defined(KANJI)
#define KANJI
#endif

#ifdef __linux__
#define USE_SYSV_PGRP
#define USE_SYSV_SIGNALS
#define WTMP
#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#endif

#ifdef __CYGWIN__
#define WTMP
#endif

#ifdef __SCO__
#ifndef _SVID3
#define _SVID3
#endif
#endif

#if defined(__GLIBC__) && !defined(__linux__)
#define USE_SYSV_PGRP
#define WTMP
#endif

#if defined(USE_TTY_GROUP) || defined(USE_UTMP_SETGID) || defined(HAVE_INITGROUPS)
#include <grp.h>
#endif

#ifndef TTY_GROUP_NAME
#define TTY_GROUP_NAME "tty"
#endif

#include <sys/stat.h>

#ifdef Lynx
#ifndef BSDLY
#define BSDLY	0
#endif
#ifndef VTDLY
#define VTDLY	0
#endif
#ifndef FFDLY
#define FFDLY	0
#endif
#endif

#ifdef SYSV			/* { */

#ifdef USE_USG_PTYS		/* AT&T SYSV has no ptyio.h */
#include <sys/stropts.h>	/* for I_PUSH */
#include <poll.h>		/* for POLLIN */
#endif /* USE_USG_PTYS */

#define USE_SYSV_SIGNALS
#define	USE_SYSV_PGRP

#if !defined(TIOCSWINSZ) || defined(__SCO__) || defined(__UNIXWARE__)
#define USE_SYSV_ENVVARS	/* COLUMNS/LINES vs. TERMCAP */
#endif

/*
 * now get system-specific includes
 */
#ifdef macII
#include <sys/ttychars.h>
#undef USE_SYSV_ENVVARS
#undef FIOCLEX
#undef FIONCLEX
#define setpgrp2 setpgrp
#include <sgtty.h>
#include <sys/resource.h>
#endif

#ifdef __hpux
#include <sys/ptyio.h>
#endif /* __hpux */

#ifdef __osf__
#undef  USE_SYSV_PGRP
#define setpgrp setpgid
#endif

#ifdef __sgi
#include <sys/sysmacros.h>
#endif /* __sgi */

#ifdef sun
#include <sys/strredir.h>
#endif

#else /* } !SYSV { */ /* BSD systems */

#ifdef __QNX__

#ifndef __QNXNTO__
#define ttyslot() 1
#else
#define USE_SYSV_PGRP
extern __inline__
int
ttyslot(void)
{
    return 1;			/* yuk */
}
#endif

#else

#if defined(__INTERIX) || defined(__APPLE__)
#define setpgrp setpgid
#endif

#ifndef __linux__
#ifndef USE_POSIX_TERMIOS
#ifndef USE_ANY_SYSV_TERMIO
#include <sgtty.h>
#endif
#endif /* USE_POSIX_TERMIOS */
#ifdef Lynx
#include <resource.h>
#else
#include <sys/resource.h>
#endif
#endif /* !__linux__ */

#endif /* __QNX__ */

#endif /* } !SYSV */

/* Xpoll.h and <sys/param.h> on glibc 2.1 systems have colliding NBBY's */
#if defined(__GLIBC__) && ((__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1)))
#ifndef NOFILE
#define NOFILE OPEN_MAX
#endif
#elif !(defined(WIN32) || defined(Lynx) || defined(__GNU__))
#include <sys/param.h>		/* for NOFILE */
#endif

#if defined(BSD) && (BSD >= 199103)
#define WTMP
#endif

#include <stdio.h>
#include <math.h>

#ifdef __hpux
#include <sys/utsname.h>
#endif /* __hpux */

#if defined(apollo) && (OSMAJORVERSION == 10) && (OSMINORVERSION < 4)
#define ttyslot() 1
#endif /* apollo */

#if defined(UTMPX_FOR_UTMP)
#define UTMP_STR utmpx
#else
#define UTMP_STR utmp
#endif

#if defined(USE_UTEMPTER)
#include <utempter.h>
#if 1
#define UTEMPTER_ADD(pty,hostname,master_fd) utempter_add_record(master_fd, hostname)
#define UTEMPTER_DEL()                       utempter_remove_added_record ()
#else
#define UTEMPTER_ADD(pty,hostname,master_fd) addToUtmp(pty, hostname, master_fd)
#define UTEMPTER_DEL()                       removeFromUtmp()
#endif
#endif

#if defined(I_FIND) && defined(I_PUSH)
#define PUSH_FAILS(fd,name) ioctl(fd, I_FIND, name) == 0 \
			 && ioctl(fd, I_PUSH, name) < 0
#else
#define PUSH_FAILS(fd,name) ioctl(fd, I_PUSH, name) < 0
#endif

#if defined(UTMPX_FOR_UTMP)

#include <utmpx.h>

#define call_endutent  endutxent
#define call_getutid   getutxid
#define call_pututline pututxline
#define call_setutent  setutxent
#define call_updwtmp   updwtmpx

#elif defined(HAVE_UTMP)

#include <utmp.h>

#if defined(_CRAY) && (OSMAJORVERSION < 8)
extern struct utmp *getutid __((struct utmp * _Id));
#endif

#define call_endutent  endutent
#define call_getutid   getutid
#define call_pututline pututline
#define call_setutent  setutent
#define call_updwtmp   updwtmp

#endif

#if defined(USE_LASTLOG) && defined(HAVE_LASTLOG_H)
#include <lastlog.h>		/* caution: glibc includes utmp.h here */
#endif

#ifndef USE_LASTLOGX
#if defined(_NETBSD_SOURCE) && defined(_PATH_LASTLOGX)
#define USE_LASTLOGX 1
#endif
#endif

#ifdef  PUCC_PTYD
#include <local/openpty.h>
#endif /* PUCC_PTYD */

#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#include <util.h>		/* openpty() */
#endif

#if defined(__FreeBSD__) || defined(__DragonFly__)
#include <libutil.h>		/* openpty() */
#endif

#if !defined(UTMP_FILENAME)
#if defined(UTMP_FILE)
#define UTMP_FILENAME UTMP_FILE
#elif defined(_PATH_UTMP)
#define UTMP_FILENAME _PATH_UTMP
#else
#define UTMP_FILENAME "/etc/utmp"
#endif
#endif

#ifndef LASTLOG_FILENAME
#ifdef _PATH_LASTLOG
#define LASTLOG_FILENAME _PATH_LASTLOG
#else
#define LASTLOG_FILENAME "/usr/adm/lastlog"	/* only on BSD systems */
#endif
#endif

#if !defined(WTMP_FILENAME)
#if defined(WTMP_FILE)
#define WTMP_FILENAME WTMP_FILE
#elif defined(_PATH_WTMP)
#define WTMP_FILENAME _PATH_WTMP
#elif defined(SYSV)
#define WTMP_FILENAME "/etc/wtmp"
#else
#define WTMP_FILENAME "/usr/adm/wtmp"
#endif
#endif

#include <signal.h>

#if defined(__SCO__) || (defined(ISC) && !defined(_POSIX_VERSION))
#undef SIGTSTP			/* defined, but not the BSD way */
#endif

#ifdef SIGTSTP
#include <sys/wait.h>
#endif

#if defined(__SCO__) || defined(__UNIXWARE__)
#undef ECHOKE
#undef ECHOCTL
#endif

#if defined(HAVE_SYS_TTYDEFAULTS_H) && !defined(CEOF)
#include <sys/ttydefaults.h>
#endif

#ifdef X_NOT_POSIX
extern long lseek();
#if defined(USG) || defined(SVR4)
extern unsigned sleep();
#else
extern void sleep();
#endif
extern char *ttyname();
#endif

#if defined(SYSV) && defined(DECL_PTSNAME)
extern char *ptsname(int);
#endif

static void reapchild(int /* n */ );
static int spawnXTerm(XtermWidget	/* xw */
		      ,unsigned /* line_speed */ );
static void remove_termcap_entry(char *, const char *);
#ifdef USE_PTY_SEARCH
static int pty_search(int * /* pty */ );
#endif

static int get_pty(int *pty, char *from);
static void resize_termcap(XtermWidget xw);
static void set_owner(char *device, unsigned uid, unsigned gid, unsigned mode);

static Bool added_utmp_entry = False;

#ifdef HAVE_POSIX_SAVED_IDS
static uid_t save_euid;
static gid_t save_egid;
#endif

static uid_t save_ruid;
static gid_t save_rgid;

#if defined(USE_UTMP_SETGID)
static int really_get_pty(int *pty, char *from);
#endif

#if defined(USE_SYSV_UTMP) && !defined(USE_UTEMPTER)
static Bool xterm_exiting = False;
#endif

static char *explicit_shname = NULL;

/*
** Ordinarily it should be okay to omit the assignment in the following
** statement. Apparently the c89 compiler on AIX 4.1.3 has a bug, or does
** it? Without the assignment though the compiler will init command_to_exec
** to 0xffffffff instead of NULL; and subsequent usage, e.g. in spawnXTerm() to
** SEGV.
*/
static char **command_to_exec = NULL;

#if OPT_LUIT_PROG
static char **command_to_exec_with_luit = NULL;
static unsigned command_length_with_luit = 0;
#endif

/* choose a nice default value for speed - if we make it too low, users who
 * mistakenly use $TERM set to vt100 will get padding delays.  Setting it to a
 * higher value is not useful since legacy applications (termcap) that care
 * about padding generally store the code in a short, which does not have
 * enough bits for the extended values.
 */
#ifdef B38400			/* everyone should define this */
#define VAL_LINE_SPEED B38400
#else /* ...but xterm's used this for a long time */
#define VAL_LINE_SPEED B9600
#endif

/*
 * Allow use of system default characters if defined and reasonable.
 * These are based on the BSD ttydefaults.h
 */
#ifndef CBRK
#define CBRK     0xff		/* was 0 */
#endif
#ifndef CDISCARD
#define CDISCARD CONTROL('O')
#endif
#ifndef CDSUSP
#define CDSUSP   CONTROL('Y')
#endif
#ifndef CEOF
#define CEOF     CONTROL('D')
#endif
#ifndef CEOL
#define CEOL	 0xff		/* was 0 */
#endif
#ifndef CERASE
#define CERASE   0177
#endif
#ifndef CERASE2
#define	CERASE2  CONTROL('H')
#endif
#ifndef CFLUSH
#define CFLUSH   CONTROL('O')
#endif
#ifndef CINTR
#define CINTR    CONTROL('C')
#endif
#ifndef CKILL
#define CKILL	 CONTROL('U')	/* was '@' */
#endif
#ifndef CLNEXT
#define CLNEXT   CONTROL('V')
#endif
#ifndef CNUL
#define CNUL     0
#endif
#ifndef CQUIT
#define CQUIT    CONTROL('\\')
#endif
#ifndef CRPRNT
#define CRPRNT   CONTROL('R')
#endif
#ifndef CREPRINT
#define CREPRINT CRPRNT
#endif
#ifndef CSTART
#define CSTART   CONTROL('Q')
#endif
#ifndef CSTATUS
#define	CSTATUS  CONTROL('T')
#endif
#ifndef CSTOP
#define CSTOP    CONTROL('S')
#endif
#ifndef CSUSP
#define CSUSP    CONTROL('Z')
#endif
#ifndef CSWTCH
#define CSWTCH   0
#endif
#ifndef CWERASE
#define CWERASE  CONTROL('W')
#endif

#ifdef TERMIO_STRUCT
/* The following structures are initialized in main() in order
** to eliminate any assumptions about the internal order of their
** contents.
*/
static TERMIO_STRUCT d_tio;

#ifndef ONLCR
#define ONLCR 0
#endif

#ifndef OPOST
#define OPOST 0
#endif

#define D_TIO_FLAGS (OPOST | ONLCR)

#ifdef HAS_LTCHARS
static struct ltchars d_ltc;
#endif /* HAS_LTCHARS */

#ifdef TIOCLSET
static unsigned int d_lmode;
#endif /* TIOCLSET */

#else /* !TERMIO_STRUCT */

#define D_SG_FLAGS (EVENP | ODDP | ECHO | CRMOD)

static struct sgttyb d_sg =
{
    0, 0, 0177, CKILL, (D_SG_FLAGS | XTABS)
};
static struct tchars d_tc =
{
    CINTR, CQUIT, CSTART,
    CSTOP, CEOF, CBRK
};
static struct ltchars d_ltc =
{
    CSUSP, CDSUSP, CRPRNT,
    CFLUSH, CWERASE, CLNEXT
};
static int d_disipline = NTTYDISC;
static long int d_lmode = LCRTBS | LCRTERA | LCRTKIL | LCTLECH;
#ifdef sony
static long int d_jmode = KM_SYSSJIS | KM_ASCII;
static struct jtchars d_jtc =
{
    'J', 'B'
};
#endif /* sony */
#endif /* TERMIO_STRUCT */

/*
 * SYSV has the termio.c_cc[V] and ltchars; BSD has tchars and ltchars;
 * SVR4 has only termio.c_cc, but it includes everything from ltchars.
 * POSIX termios has termios.c_cc, which is similar to SVR4.
 */
#define TTYMODE(name) { name, sizeof(name)-1, 0, 0 }
static Boolean override_tty_modes = False;
/* *INDENT-OFF* */
static struct {
    const char *name;
    size_t len;
    int set;
    int value;
} ttyModes[] = {
    TTYMODE("intr"),		/* tchars.t_intrc ; VINTR */
#define XTTYMODE_intr	0
    TTYMODE("quit"),		/* tchars.t_quitc ; VQUIT */
#define XTTYMODE_quit	1
    TTYMODE("erase"),		/* sgttyb.sg_erase ; VERASE */
#define XTTYMODE_erase	2
    TTYMODE("kill"),		/* sgttyb.sg_kill ; VKILL */
#define XTTYMODE_kill	3
    TTYMODE("eof"),		/* tchars.t_eofc ; VEOF */
#define XTTYMODE_eof	4
    TTYMODE("eol"),		/* VEOL */
#define XTTYMODE_eol	5
    TTYMODE("swtch"),		/* VSWTCH */
#define XTTYMODE_swtch	6
    TTYMODE("start"),		/* tchars.t_startc ; VSTART */
#define XTTYMODE_start	7
    TTYMODE("stop"),		/* tchars.t_stopc ; VSTOP */
#define XTTYMODE_stop	8
    TTYMODE("brk"),		/* tchars.t_brkc */
#define XTTYMODE_brk	9
    TTYMODE("susp"),		/* ltchars.t_suspc ; VSUSP */
#define XTTYMODE_susp	10
    TTYMODE("dsusp"),		/* ltchars.t_dsuspc ; VDSUSP */
#define XTTYMODE_dsusp	11
    TTYMODE("rprnt"),		/* ltchars.t_rprntc ; VREPRINT */
#define XTTYMODE_rprnt	12
    TTYMODE("flush"),		/* ltchars.t_flushc ; VDISCARD */
#define XTTYMODE_flush	13
    TTYMODE("weras"),		/* ltchars.t_werasc ; VWERASE */
#define XTTYMODE_weras	14
    TTYMODE("lnext"),		/* ltchars.t_lnextc ; VLNEXT */
#define XTTYMODE_lnext	15
    TTYMODE("status"),		/* VSTATUS */
#define XTTYMODE_status	16
    TTYMODE("erase2"),		/* VERASE2 */
#define XTTYMODE_erase2	17
    TTYMODE("eol2"),		/* VEOL2 */
#define XTTYMODE_eol2	18
    TTYMODE("tabs"),		/* TAB0 */
#define XTTYMODE_tabs	19
    TTYMODE("-tabs"),		/* TAB3 */
#define XTTYMODE__tabs	20
};

#ifndef TAB0
#define TAB0 0
#endif

#ifndef TAB3
#if defined(OXTABS)
#define TAB3 OXTABS
#elif defined(XTABS)
#define TAB3 XTABS
#endif
#endif

#ifndef TABDLY
#define TABDLY (TAB0|TAB3)
#endif

#define isTtyMode(p,q) (ttyChars[p].myMode == q && ttyModes[q].set)

#define isTabMode(n) \
	(isTtyMode(n, XTTYMODE_tabs) || \
	 isTtyMode(n, XTTYMODE__tabs))

#define TMODE(ind,var) \
	if (ttyModes[ind].set) \
	    var = (cc_t) ttyModes[ind].value

#define validTtyChar(data, n) \
	    (ttyChars[n].sysMode >= 0 && \
	     ttyChars[n].sysMode < (int) XtNumber(data.c_cc))

static const struct {
    int sysMode;
    int myMode;
    int myDefault;
} ttyChars[] = {
#ifdef VINTR
    { VINTR,    XTTYMODE_intr,   CINTR },
#endif
#ifdef VQUIT
    { VQUIT,    XTTYMODE_quit,   CQUIT },
#endif
#ifdef VERASE
    { VERASE,   XTTYMODE_erase,  CERASE },
#endif
#ifdef VKILL
    { VKILL,    XTTYMODE_kill,   CKILL },
#endif
#ifdef VEOF
    { VEOF,     XTTYMODE_eof,    CEOF },
#endif
#ifdef VEOL
    { VEOL,     XTTYMODE_eol,    CEOL },
#endif
#ifdef VSWTCH
    { VSWTCH,   XTTYMODE_swtch,  CNUL },
#endif
#ifdef VSTART
    { VSTART,   XTTYMODE_start,  CSTART },
#endif
#ifdef VSTOP
    { VSTOP,    XTTYMODE_stop,   CSTOP },
#endif
#ifdef VSUSP
    { VSUSP,    XTTYMODE_susp,   CSUSP },
#endif
#ifdef VDSUSP
    { VDSUSP,   XTTYMODE_dsusp,  CDSUSP },
#endif
#ifdef VREPRINT
    { VREPRINT, XTTYMODE_rprnt,  CREPRINT },
#endif
#ifdef VDISCARD
    { VDISCARD, XTTYMODE_flush,  CDISCARD },
#endif
#ifdef VWERASE
    { VWERASE,  XTTYMODE_weras,  CWERASE },
#endif
#ifdef VLNEXT
    { VLNEXT,   XTTYMODE_lnext,  CLNEXT },
#endif
#ifdef VSTATUS
    { VSTATUS,  XTTYMODE_status, CSTATUS },
#endif
#ifdef VERASE2
    { VERASE2,  XTTYMODE_erase2, CERASE2 },
#endif
#ifdef VEOL2
    { VEOL2,    XTTYMODE_eol2,   CNUL },
#endif
    { -1,       XTTYMODE_tabs,   TAB0 },
    { -1,       XTTYMODE__tabs,  TAB3 },
};
/* *INDENT-ON* */

static int parse_tty_modes(char *s);

#ifndef USE_UTEMPTER
#ifdef USE_SYSV_UTMP
#if (defined(AIXV3) && (OSMAJORVERSION < 4)) && !(defined(getutid))
extern struct utmp *getutid();
#endif /* AIXV3 */

#else /* not USE_SYSV_UTMP */
static char etc_utmp[] = UTMP_FILENAME;
#endif /* USE_SYSV_UTMP */

#if defined(USE_LASTLOG) && defined(USE_STRUCT_LASTLOG)
static char etc_lastlog[] = LASTLOG_FILENAME;
#else
#undef USE_LASTLOG
#endif

#ifdef WTMP
static char etc_wtmp[] = WTMP_FILENAME;
#endif
#endif /* !USE_UTEMPTER */

/*
 * Some people with 4.3bsd /bin/login seem to like to use login -p -f user
 * to implement xterm -ls.  They can turn on USE_LOGIN_DASH_P and turn off
 * WTMP and USE_LASTLOG.
 */
#ifdef USE_LOGIN_DASH_P
#ifndef LOGIN_FILENAME
#define LOGIN_FILENAME "/bin/login"
#endif
static char bin_login[] = LOGIN_FILENAME;
#endif

static char noPassedPty[2];
static char *passedPty = noPassedPty;	/* name if pty if slave */

#if defined(TIOCCONS) || defined(SRIOCSREDIR)
static int Console;
#include <X11/Xmu/SysUtil.h>	/* XmuGetHostname */
#define MIT_CONSOLE_LEN	12
#define MIT_CONSOLE "MIT_CONSOLE_"
static char mit_console_name[255 + MIT_CONSOLE_LEN + 1] = MIT_CONSOLE;
static Atom mit_console;
#endif /* TIOCCONS */

#ifndef USE_SYSV_UTMP
static int tslot;
#endif /* USE_SYSV_UTMP */
static sigjmp_buf env;

#define SetUtmpHost(dst, screen) \
	{ \
	    char host[sizeof(dst) + 1]; \
	    strncpy(host, DisplayString(screen->display), sizeof(host) - 1); \
	    host[sizeof(dst)] = '\0'; \
	    TRACE(("DisplayString(%s)\n", host)); \
	    if (!resource.utmpDisplayId) { \
		char *endptr = strrchr(host, ':'); \
		if (endptr) { \
		    TRACE(("trimming display-id '%s'\n", host)); \
		    *endptr = '\0'; \
		} \
	    } \
	    copy_filled(dst, host, sizeof(dst)); \
	}

#ifdef HAVE_UTMP_UT_SYSLEN
#  define SetUtmpSysLen(utmp) 			   \
	{ \
	    utmp.ut_host[sizeof(utmp.ut_host)-1] = '\0'; \
	    utmp.ut_syslen = (short) ((int) strlen(utmp.ut_host) + 1); \
	}
#endif

/* used by VT (charproc.c) */

static XtResource application_resources[] =
{
    Sres(XtNiconGeometry, XtCIconGeometry, icon_geometry, NULL),
    Sres(XtNtitle, XtCTitle, title, NULL),
    Sres(XtNiconHint, XtCIconHint, icon_hint, NULL),
    Sres(XtNiconName, XtCIconName, icon_name, NULL),
    Sres(XtNtermName, XtCTermName, term_name, NULL),
    Sres(XtNttyModes, XtCTtyModes, tty_modes, NULL),
    Sres(XtNvalidShells, XtCValidShells, valid_shells, NULL),
    Bres(XtNhold, XtCHold, hold_screen, False),
    Bres(XtNutmpInhibit, XtCUtmpInhibit, utmpInhibit, False),
    Bres(XtNutmpDisplayId, XtCUtmpDisplayId, utmpDisplayId, True),
    Bres(XtNmessages, XtCMessages, messages, True),
    Ires(XtNminBufSize, XtCMinBufSize, minBufSize, 4096),
    Ires(XtNmaxBufSize, XtCMaxBufSize, maxBufSize, 32768),
    Sres(XtNmenuLocale, XtCMenuLocale, menuLocale, DEF_MENU_LOCALE),
    Bres(XtNnotMapped, XtCNotMapped, notMapped, False),
    Sres(XtNomitTranslation, XtCOmitTranslation, omitTranslation, NULL),
    Sres(XtNkeyboardType, XtCKeyboardType, keyboardType, "unknown"),
#ifdef HAVE_LIB_XCURSOR
    Sres(XtNcursorTheme, XtCCursorTheme, cursorTheme, "none"),
#endif
#if OPT_PRINT_ON_EXIT
    Ires(XtNprintModeImmediate, XtCPrintModeImmediate, printModeNow, 0),
    Ires(XtNprintOptsImmediate, XtCPrintOptsImmediate, printOptsNow, 9),
    Sres(XtNprintFileImmediate, XtCPrintFileImmediate, printFileNow, NULL),
    Ires(XtNprintModeOnXError, XtCPrintModeOnXError, printModeOnXError, 0),
    Ires(XtNprintOptsOnXError, XtCPrintOptsOnXError, printOptsOnXError, 9),
    Sres(XtNprintFileOnXError, XtCPrintFileOnXError, printFileOnXError, NULL),
#endif
#if OPT_SUNPC_KBD
    Bres(XtNsunKeyboard, XtCSunKeyboard, sunKeyboard, False),
#endif
#if OPT_HP_FUNC_KEYS
    Bres(XtNhpFunctionKeys, XtCHpFunctionKeys, hpFunctionKeys, False),
#endif
#if OPT_SCO_FUNC_KEYS
    Bres(XtNscoFunctionKeys, XtCScoFunctionKeys, scoFunctionKeys, False),
#endif
#if OPT_SUN_FUNC_KEYS
    Bres(XtNsunFunctionKeys, XtCSunFunctionKeys, sunFunctionKeys, False),
#endif
#if OPT_TCAP_FKEYS
    Bres(XtNtcapFunctionKeys, XtCTcapFunctionKeys, termcapKeys, False),
#endif
#if OPT_INITIAL_ERASE
    Bres(XtNptyInitialErase, XtCPtyInitialErase, ptyInitialErase, DEF_INITIAL_ERASE),
    Bres(XtNbackarrowKeyIsErase, XtCBackarrowKeyIsErase, backarrow_is_erase, DEF_BACKARO_ERASE),
#endif
    Bres(XtNuseInsertMode, XtCUseInsertMode, useInsertMode, False),
#if OPT_ZICONBEEP
    Ires(XtNzIconBeep, XtCZIconBeep, zIconBeep, 0),
    Sres(XtNzIconTitleFormat, XtCZIconTitleFormat, zIconFormat, "*** %s"),
#endif
#if OPT_PTY_HANDSHAKE
    Bres(XtNwaitForMap, XtCWaitForMap, wait_for_map, False),
    Bres(XtNptyHandshake, XtCPtyHandshake, ptyHandshake, True),
    Bres(XtNptySttySize, XtCPtySttySize, ptySttySize, DEF_PTY_STTY_SIZE),
#endif
#if OPT_REPORT_CCLASS
    Bres(XtNreportCClass, XtCReportCClass, reportCClass, False),
#endif
#if OPT_REPORT_COLORS
    Bres(XtNreportColors, XtCReportColors, reportColors, False),
#endif
#if OPT_REPORT_FONTS
    Bres(XtNreportFonts, XtCReportFonts, reportFonts, False),
#endif
#if OPT_REPORT_ICONS
    Bres(XtNreportIcons, XtCReportIcons, reportIcons, False),
#endif
#if OPT_XRES_QUERY
    Bres(XtNreportXRes, XtCReportXRes, reportXRes, False),
#endif
#if OPT_SAME_NAME
    Bres(XtNsameName, XtCSameName, sameName, True),
#endif
#if OPT_SESSION_MGT
    Bres(XtNsessionMgt, XtCSessionMgt, sessionMgt, True),
#endif
#if OPT_TOOLBAR
    Bres(XtNtoolBar, XtCToolBar, toolBar, True),
#endif
#if OPT_MAXIMIZE
    Bres(XtNmaximized, XtCMaximized, maximized, False),
    Sres(XtNfullscreen, XtCFullscreen, fullscreen_s, "off"),
#endif
#if USE_DOUBLE_BUFFER
    Bres(XtNbuffered, XtCBuffered, buffered, DEF_DOUBLE_BUFFER),
    Ires(XtNbufferedFPS, XtCBufferedFPS, buffered_fps, 40),
#endif
};

static String fallback_resources[] =
{
#if OPT_TOOLBAR
    "*toolBar: false",
#endif
    "*SimpleMenu*menuLabel.vertSpace: 100",
    "*SimpleMenu*HorizontalMargins: 16",
    "*SimpleMenu*Sme.height: 16",
    "*SimpleMenu*Cursor: left_ptr",
    "*mainMenu.Label:  Main Options (no app-defaults)",
    "*vtMenu.Label:  VT Options (no app-defaults)",
    "*fontMenu.Label:  VT Fonts (no app-defaults)",
#if OPT_TEK4014
    "*tekMenu.Label:  Tek Options (no app-defaults)",
#endif
    NULL
};

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XrmParseCommand is let loose. */
/* *INDENT-OFF* */
#define DATA(option,pattern,type,value) { (char *) option, (char *) pattern, type, (XPointer) value }
#define OPTS(option,pattern_type,value) { (char *) option, (char *) pattern_type, (XPointer) value }
#define UP_ARG(name) "."name, XrmoptionSepArg
#define MY_ARG(name) "*"name, XrmoptionSepArg
#define VT_ARG(name) "*vt100."name, XrmoptionSepArg
#define NO_ARG(name) "*"name, XrmoptionNoArg
static XrmOptionDescRec optionDescList[] = {
OPTS("-geometry",	VT_ARG(XtNgeometry),			NULL),
OPTS("-132",		NO_ARG(XtNc132),			"on"),
OPTS("+132",		NO_ARG(XtNc132),			"off"),
OPTS("-ah",		NO_ARG(XtNalwaysHighlight),		"on"),
OPTS("+ah",		NO_ARG(XtNalwaysHighlight),		"off"),
OPTS("-aw",		NO_ARG(XtNautoWrap),			"on"),
OPTS("+aw",		NO_ARG(XtNautoWrap),			"off"),
#ifndef NO_ACTIVE_ICON
OPTS("-ai",		NO_ARG(XtNactiveIcon),			"off"),
OPTS("+ai",		NO_ARG(XtNactiveIcon),			"on"),
#endif /* NO_ACTIVE_ICON */
OPTS("-b",		MY_ARG(XtNinternalBorder),		NULL),
OPTS("-barc",		NO_ARG(XtNcursorBar),			"on"),
OPTS("+barc",		NO_ARG(XtNcursorBar),			"off"),
OPTS("-bc",		NO_ARG(XtNcursorBlink),			"on"),
OPTS("+bc",		NO_ARG(XtNcursorBlink),			"off"),
OPTS("-bcf",		MY_ARG(XtNcursorOffTime),		NULL),
OPTS("-bcn",		MY_ARG(XtNcursorOnTime),		NULL),
OPTS("-bdc",		NO_ARG(XtNcolorBDMode),			"off"),
OPTS("+bdc",		NO_ARG(XtNcolorBDMode),			"on"),
OPTS("-cb",		NO_ARG(XtNcutToBeginningOfLine),	"off"),
OPTS("+cb",		NO_ARG(XtNcutToBeginningOfLine),	"on"),
OPTS("-cc",		MY_ARG(XtNcharClass),			NULL),
OPTS("-cm",		NO_ARG(XtNcolorMode),			"off"),
OPTS("+cm",		NO_ARG(XtNcolorMode),			"on"),
OPTS("-cn",		NO_ARG(XtNcutNewline),			"off"),
OPTS("+cn",		NO_ARG(XtNcutNewline),			"on"),
OPTS("-cr",		MY_ARG(XtNcursorColor),			NULL),
OPTS("-cu",		NO_ARG(XtNcurses),			"on"),
OPTS("+cu",		NO_ARG(XtNcurses),			"off"),
OPTS("-dc",		NO_ARG(XtNdynamicColors),		"off"),
OPTS("+dc",		NO_ARG(XtNdynamicColors),		"on"),
OPTS("-fb",		MY_ARG(XtNboldFont),			NULL),
OPTS("-fbb",		NO_ARG(XtNfreeBoldBox),			"off"),
OPTS("+fbb",		NO_ARG(XtNfreeBoldBox),			"on"),
OPTS("-fbx",		NO_ARG(XtNforceBoxChars),		"off"),
OPTS("+fbx",		NO_ARG(XtNforceBoxChars),		"on"),
OPTS("-fc",		MY_ARG(XtNinitialFont),			NULL),
#ifndef NO_ACTIVE_ICON
OPTS("-fi",		MY_ARG(XtNiconFont),			NULL),
#endif /* NO_ACTIVE_ICON */
#if OPT_RENDERFONT
OPTS("-fa",		MY_ARG(XtNfaceName),			NULL),
OPTS("-fd",		MY_ARG(XtNfaceNameDoublesize),		NULL),
OPTS("-fs",		MY_ARG(XtNfaceSize),			NULL),
#endif
#if OPT_WIDE_ATTRS && OPT_ISO_COLORS
OPTS("-itc",		NO_ARG(XtNcolorITMode),			"off"),
OPTS("+itc",		NO_ARG(XtNcolorITMode),			"on"),
#endif
#if OPT_WIDE_CHARS
OPTS("-fw",		MY_ARG(XtNwideFont),			NULL),
OPTS("-fwb",		MY_ARG(XtNwideBoldFont),		NULL),
#endif
#if OPT_INPUT_METHOD
OPTS("-fx",		MY_ARG(XtNximFont),			NULL),
#endif
#if OPT_HIGHLIGHT_COLOR
OPTS("-hc",		MY_ARG(XtNhighlightColor),		NULL),
OPTS("-hm",		NO_ARG(XtNhighlightColorMode),		"on"),
OPTS("+hm",		NO_ARG(XtNhighlightColorMode),		"off"),
OPTS("-selfg",		MY_ARG(XtNhighlightTextColor),		NULL),
OPTS("-selbg",		MY_ARG(XtNhighlightColor),		NULL),
#endif
#if OPT_HP_FUNC_KEYS
OPTS("-hf",		NO_ARG(XtNhpFunctionKeys),		"on"),
OPTS("+hf",		NO_ARG(XtNhpFunctionKeys),		"off"),
#endif
OPTS("-hold",		NO_ARG(XtNhold),			"on"),
OPTS("+hold",		NO_ARG(XtNhold),			"off"),
#if OPT_INITIAL_ERASE
OPTS("-ie",		NO_ARG(XtNptyInitialErase),		"on"),
OPTS("+ie",		NO_ARG(XtNptyInitialErase),		"off"),
#endif
OPTS("-j",		NO_ARG(XtNjumpScroll),			"on"),
OPTS("+j",		NO_ARG(XtNjumpScroll),			"off"),
OPTS("-jf",		NO_ARG(XtNfastScroll),			"on"),
OPTS("+jf",		NO_ARG(XtNfastScroll),			"off"),
#if OPT_C1_PRINT
OPTS("-k8",		NO_ARG(XtNallowC1Printable),		"on"),
OPTS("+k8",		NO_ARG(XtNallowC1Printable),		"off"),
#endif
OPTS("-kt",		MY_ARG(XtNkeyboardType),		NULL),
/* parse logging options anyway for compatibility */
OPTS("-l",		NO_ARG(XtNlogging),			"on"),
OPTS("+l",		NO_ARG(XtNlogging),			"off"),
OPTS("-lf",		MY_ARG(XtNlogFile),			NULL),
OPTS("-ls",		NO_ARG(XtNloginShell),			"on"),
OPTS("+ls",		NO_ARG(XtNloginShell),			"off"),
OPTS("-mb",		NO_ARG(XtNmarginBell),			"on"),
OPTS("+mb",		NO_ARG(XtNmarginBell),			"off"),
OPTS("-mc",		MY_ARG(XtNmultiClickTime),		NULL),
OPTS("-mesg",		NO_ARG(XtNmessages),			"off"),
OPTS("+mesg",		NO_ARG(XtNmessages),			"on"),
OPTS("-ms",		MY_ARG(XtNpointerColor),		NULL),
OPTS("-nb",		MY_ARG(XtNnMarginBell),			NULL),
OPTS("-nomap",		NO_ARG(XtNnotMapped),			"on"),
OPTS("+nomap",		NO_ARG(XtNnotMapped),			"off"),
OPTS("-nul",		NO_ARG(XtNunderLine),			"off"),
OPTS("+nul",		NO_ARG(XtNunderLine),			"on"),
OPTS("-pc",		NO_ARG(XtNboldColors),			"on"),
OPTS("+pc",		NO_ARG(XtNboldColors),			"off"),
OPTS("-pf",		MY_ARG(XtNpointerFont),			NULL),
OPTS("-rw",		NO_ARG(XtNreverseWrap),			"on"),
OPTS("+rw",		NO_ARG(XtNreverseWrap),			"off"),
OPTS("-s",		NO_ARG(XtNmultiScroll),			"on"),
OPTS("+s",		NO_ARG(XtNmultiScroll),			"off"),
OPTS("-sb",		NO_ARG(XtNscrollBar),			"on"),
OPTS("+sb",		NO_ARG(XtNscrollBar),			"off"),
#if OPT_REPORT_CCLASS
OPTS("-report-charclass", NO_ARG(XtNreportCClass),		"on"),
#endif
#if OPT_REPORT_COLORS
OPTS("-report-colors",	NO_ARG(XtNreportColors),		"on"),
#endif
#if OPT_REPORT_ICONS
OPTS("-report-icons",	NO_ARG(XtNreportIcons),			"on"),
#endif
#if OPT_REPORT_FONTS
OPTS("-report-fonts",	NO_ARG(XtNreportFonts),			"on"),
#endif
#if OPT_XRES_QUERY
OPTS("-report-xres",	NO_ARG(XtNreportXRes),			"on"),
#endif
#ifdef SCROLLBAR_RIGHT
OPTS("-leftbar",	NO_ARG(XtNrightScrollBar),		"off"),
OPTS("-rightbar",	NO_ARG(XtNrightScrollBar),		"on"),
#endif
OPTS("-rvc",		NO_ARG(XtNcolorRVMode),			"off"),
OPTS("+rvc",		NO_ARG(XtNcolorRVMode),			"on"),
OPTS("-sf",		NO_ARG(XtNsunFunctionKeys),		"on"),
OPTS("+sf",		NO_ARG(XtNsunFunctionKeys),		"off"),
OPTS("-sh",		MY_ARG(XtNscaleHeight),			NULL),
OPTS("-si",		NO_ARG(XtNscrollTtyOutput),		"off"),
OPTS("+si",		NO_ARG(XtNscrollTtyOutput),		"on"),
OPTS("-sk",		NO_ARG(XtNscrollKey),			"on"),
OPTS("+sk",		NO_ARG(XtNscrollKey),			"off"),
OPTS("-sl",		MY_ARG(XtNsaveLines),			NULL),
#if OPT_SUNPC_KBD
OPTS("-sp",		NO_ARG(XtNsunKeyboard),			"on"),
OPTS("+sp",		NO_ARG(XtNsunKeyboard),			"off"),
#endif
#if OPT_TEK4014
OPTS("-t",		NO_ARG(XtNtekStartup),			"on"),
OPTS("+t",		NO_ARG(XtNtekStartup),			"off"),
#endif
OPTS("-ti",		MY_ARG(XtNdecTerminalID),		NULL),
OPTS("-tm",		MY_ARG(XtNttyModes),			NULL),
OPTS("-tn",		MY_ARG(XtNtermName),			NULL),
#if OPT_WIDE_CHARS
OPTS("-u8",		NO_ARG(XtNutf8),			"2"),
OPTS("+u8",		NO_ARG(XtNutf8),			"0"),
#endif
#if OPT_LUIT_PROG
OPTS("-lc",		NO_ARG(XtNlocale),			"on"),
OPTS("+lc",		NO_ARG(XtNlocale),			"off"),
OPTS("-lcc",		MY_ARG(XtNlocaleFilter),		NULL),
OPTS("-en",		MY_ARG(XtNlocale),			NULL),
#endif
OPTS("-uc",		NO_ARG(XtNcursorUnderLine),		"on"),
OPTS("+uc",		NO_ARG(XtNcursorUnderLine),		"off"),
OPTS("-ulc",		NO_ARG(XtNcolorULMode),			"off"),
OPTS("+ulc",		NO_ARG(XtNcolorULMode),			"on"),
OPTS("-ulit",		NO_ARG(XtNitalicULMode),		"off"),
OPTS("+ulit",		NO_ARG(XtNitalicULMode),		"on"),
OPTS("-ut",		NO_ARG(XtNutmpInhibit),			"on"),
OPTS("+ut",		NO_ARG(XtNutmpInhibit),			"off"),
OPTS("-im",		NO_ARG(XtNuseInsertMode),		"on"),
OPTS("+im",		NO_ARG(XtNuseInsertMode),		"off"),
OPTS("-vb",		NO_ARG(XtNvisualBell),			"on"),
OPTS("+vb",		NO_ARG(XtNvisualBell),			"off"),
OPTS("-pob",		NO_ARG(XtNpopOnBell),			"on"),
OPTS("+pob",		NO_ARG(XtNpopOnBell),			"off"),
#if OPT_WIDE_CHARS
OPTS("-wc",		NO_ARG(XtNwideChars),			"on"),
OPTS("+wc",		NO_ARG(XtNwideChars),			"off"),
OPTS("-mk_width",	NO_ARG(XtNmkWidth),			"on"),
OPTS("+mk_width",	NO_ARG(XtNmkWidth),			"off"),
OPTS("-cjk_width",	NO_ARG(XtNcjkWidth),			"on"),
OPTS("+cjk_width",	NO_ARG(XtNcjkWidth),			"off"),
#endif
OPTS("-wf",		NO_ARG(XtNwaitForMap),			"on"),
OPTS("+wf",		NO_ARG(XtNwaitForMap),			"off"),
#if OPT_ZICONBEEP
OPTS("-ziconbeep",	MY_ARG(XtNzIconBeep),			NULL),
#endif
#if OPT_SAME_NAME
OPTS("-samename",	NO_ARG(XtNsameName),			"on"),
OPTS("+samename",	NO_ARG(XtNsameName),			"off"),
#endif
#if OPT_SESSION_MGT
OPTS("-sm",		NO_ARG(XtNsessionMgt),			"on"),
OPTS("+sm",		NO_ARG(XtNsessionMgt),			"off"),
#endif
#if OPT_TOOLBAR
OPTS("-tb",		NO_ARG(XtNtoolBar),			"on"),
OPTS("+tb",		NO_ARG(XtNtoolBar),			"off"),
#endif
#if OPT_MAXIMIZE
OPTS("-maximized",	NO_ARG(XtNmaximized),			"on"),
OPTS("+maximized",	NO_ARG(XtNmaximized),			"off"),
OPTS("-fullscreen",	NO_ARG(XtNfullscreen),			"on"),
OPTS("+fullscreen",	NO_ARG(XtNfullscreen),			"off"),
#endif
/* options that we process ourselves */
DATA("-help",		NULL,		XrmoptionSkipNArgs,	NULL),
DATA("-version",	NULL,		XrmoptionSkipNArgs,	NULL),
DATA("-baudrate",	NULL,		XrmoptionSkipArg,	NULL),
DATA("-class",		NULL,		XrmoptionSkipArg,	NULL),
DATA("-e",		NULL,		XrmoptionSkipLine,	NULL),
DATA("-into",		NULL,		XrmoptionSkipArg,	NULL),
/* bogus old compatibility stuff for which there are
   standard XtOpenApplication options now */
DATA("%",		"*tekGeometry",	XrmoptionStickyArg,	NULL),
DATA("#",		".iconGeometry", XrmoptionStickyArg,	NULL),
OPTS("-T",		UP_ARG(XtNtitle),			NULL),
OPTS("-n",		MY_ARG(XtNiconName),			NULL),
OPTS("-r",		NO_ARG(XtNreverseVideo),		"on"),
OPTS("+r",		NO_ARG(XtNreverseVideo),		"off"),
OPTS("-rv",		NO_ARG(XtNreverseVideo),		"on"),
OPTS("+rv",		NO_ARG(XtNreverseVideo),		"off"),
OPTS("-w",		UP_ARG(XtNborderWidth),			NULL),
#undef DATA
};

static OptionHelp xtermOptions[] = {
{ "-version",              "print the version number" },
{ "-help",                 "print out this message" },
{ "-display displayname",  "X server to contact" },
{ "-geometry geom",        "size (in characters) and position" },
{ "-/+rv",                 "turn on/off reverse video" },
{ "-bg color",             "background color" },
{ "-fg color",             "foreground color" },
{ "-bd color",             "border color" },
{ "-bw number",            "border width in pixels" },
{ "-fn fontname",          "normal text font" },
{ "-fb fontname",          "bold text font" },
{ "-fc fontmenu",          "start with named fontmenu choice" },
{ "-/+fbb",                "turn on/off normal/bold font comparison inhibit"},
{ "-/+fbx",                "turn off/on linedrawing characters"},
#if OPT_RENDERFONT
{ "-fa pattern",           "FreeType font-selection pattern" },
{ "-fd pattern",           "FreeType Doublesize font-selection pattern" },
{ "-fs size",              "FreeType font-size" },
#endif
#if OPT_WIDE_CHARS
{ "-fw fontname",          "doublewidth text font" },
{ "-fwb fontname",         "doublewidth bold text font" },
#endif
#if OPT_INPUT_METHOD
{ "-fx fontname",          "XIM fontset" },
#endif
{ "-iconic",               "start iconic" },
{ "-name string",          "client instance, icon, and title strings" },
{ "-baudrate rate",        "set line-speed (default 38400)" },
{ "-class string",         "class string (XTerm)" },
{ "-title string",         "title string" },
{ "-xrm resourcestring",   "additional resource specifications" },
{ "-/+132",                "turn on/off 80/132 column switching" },
{ "-/+ah",                 "turn on/off always highlight" },
#ifndef NO_ACTIVE_ICON
{ "-/+ai",                 "turn off/on active icon" },
{ "-fi fontname",          "icon font for active icon" },
#endif /* NO_ACTIVE_ICON */
{ "-b number",             "internal border in pixels" },
{ "-/+bc",                 "turn on/off text cursor blinking" },
{ "-bcf milliseconds",     "time text cursor is off when blinking"},
{ "-bcn milliseconds",     "time text cursor is on when blinking"},
{ "-/+bdc",                "turn off/on display of bold as color"},
{ "-/+cb",                 "turn on/off cut-to-beginning-of-line inhibit" },
{ "-cc classrange",        "specify additional character classes" },
{ "-/+cm",                 "turn off/on ANSI color mode" },
{ "-/+cn",                 "turn on/off cut newline inhibit" },
{ "-pf fontname",          "cursor font for text area pointer" },
{ "-cr color",             "text cursor color" },
{ "-/+cu",                 "turn on/off curses emulation" },
{ "-/+dc",                 "turn off/on dynamic color selection" },
#if OPT_HIGHLIGHT_COLOR
{ "-/+hm",                 "turn on/off selection-color override" },
{ "-selbg color",          "selection background color" },
{ "-selfg color",          "selection foreground color" },
/* -hc is deprecated, not shown in help message */
#endif
#if OPT_HP_FUNC_KEYS
{ "-/+hf",                 "turn on/off HP Function Key escape codes" },
#endif
{ "-/+hold",               "turn on/off logic that retains window after exit" },
#if OPT_INITIAL_ERASE
{ "-/+ie",                 "turn on/off initialization of 'erase' from pty" },
#endif
{ "-/+im",                 "use insert mode for TERMCAP" },
{ "-/+j",                  "turn on/off jump scroll" },
#if OPT_C1_PRINT
{ "-/+k8",                 "turn on/off C1-printable classification"},
#endif
{ "-kt keyboardtype",      "set keyboard type:" KEYBOARD_TYPES },
#ifdef ALLOWLOGGING
{ "-/+l",                  "turn on/off logging" },
{ "-lf filename",          "logging filename (use '-' for standard out)" },
#else
{ "-/+l",                  "turn on/off logging (not supported)" },
{ "-lf filename",          "logging filename (not supported)" },
#endif
{ "-/+ls",                 "turn on/off login shell" },
{ "-/+mb",                 "turn on/off margin bell" },
{ "-mc milliseconds",      "multiclick time in milliseconds" },
{ "-/+mesg",               "forbid/allow messages" },
{ "-ms color",             "pointer color" },
{ "-nb number",            "margin bell in characters from right end" },
{ "-/+nomap",              "turn off/on initial mapping of window" },
{ "-/+nul",                "turn off/on display of underlining" },
{ "-/+aw",                 "turn on/off auto wraparound" },
{ "-/+pc",                 "turn on/off PC-style bold colors" },
{ "-/+rw",                 "turn on/off reverse wraparound" },
{ "-/+s",                  "turn on/off multiscroll" },
{ "-/+sb",                 "turn on/off scrollbar" },
#if OPT_REPORT_CCLASS
{"-report-charclass",      "report \"charClass\" after initialization"},
#endif
#if OPT_REPORT_COLORS
{ "-report-colors",        "report colors as they are allocated" },
#endif
#if OPT_REPORT_FONTS
{ "-report-fonts",         "report fonts as loaded to stdout" },
#endif
#if OPT_REPORT_ICONS
{ "-report-icons",         "report title/icon updates" },
#endif
#if OPT_XRES_QUERY
{ "-report-xres",          "report X resources for VT100 widget" },
#endif
#ifdef SCROLLBAR_RIGHT
{ "-rightbar",             "force scrollbar right (default left)" },
{ "-leftbar",              "force scrollbar left" },
#endif
{ "-/+rvc",                "turn off/on display of reverse as color" },
{ "-/+sf",                 "turn on/off Sun Function Key escape codes" },
{ "-sh number",            "scale line-height values by the given number" },
{ "-/+si",                 "turn on/off scroll-on-tty-output inhibit" },
{ "-/+sk",                 "turn on/off scroll-on-keypress" },
{ "-sl number",            "number of scrolled lines to save" },
#if OPT_SUNPC_KBD
{ "-/+sp",                 "turn on/off Sun/PC Function/Keypad mapping" },
#endif
#if OPT_TEK4014
{ "-/+t",                  "turn on/off Tek emulation window" },
#endif
#if OPT_TOOLBAR
{ "-/+tb",                 "turn on/off toolbar" },
#endif
{ "-ti termid",            "terminal identifier" },
{ "-tm string",            "terminal mode keywords and characters" },
{ "-tn name",              "TERM environment variable name" },
#if OPT_WIDE_CHARS
{ "-/+u8",                 "turn on/off UTF-8 mode (implies wide-characters)" },
#endif
#if OPT_LUIT_PROG
{ "-/+lc",                 "turn on/off locale mode using luit" },
{ "-lcc path",             "filename of locale converter (" DEFLOCALEFILTER ")" },
/* -en is deprecated, not shown in help message */
#endif
{ "-/+uc",                 "turn on/off underline cursor" },
{ "-/+ulc",                "turn off/on display of underline as color" },
{ "-/+ulit",               "turn off/on display of underline as italics" },
#ifdef HAVE_UTMP
{ "-/+ut",                 "turn on/off utmp support" },
#else
{ "-/+ut",                 "turn on/off utmp support (not available)" },
#endif
{ "-/+vb",                 "turn on/off visual bell" },
{ "-/+pob",                "turn on/off pop on bell" },
#if OPT_WIDE_ATTRS && OPT_ISO_COLORS
{ "-/+itc",                "turn off/on display of italic as color"},
#endif
#if OPT_WIDE_CHARS
{ "-/+wc",                 "turn on/off wide-character mode" },
{ "-/+mk_width",           "turn on/off simple width convention" },
{ "-/+cjk_width",          "turn on/off legacy CJK width convention" },
#endif
{ "-/+wf",                 "turn on/off wait for map before command exec" },
{ "-e command args ...",   "command to execute" },
#if OPT_TEK4014
{ "%geom",                 "Tek window geometry" },
#endif
{ "#geom",                 "icon window geometry" },
{ "-T string",             "title name for window" },
{ "-n string",             "icon name for window" },
#if defined(TIOCCONS) || defined(SRIOCSREDIR)
{ "-C",                    "intercept console messages" },
#else
{ "-C",                    "intercept console messages (not supported)" },
#endif
{ "-Sccn",                 "slave mode on \"ttycc\", file descriptor \"n\"" },
{ "-into windowId",        "use the window id given to -into as the parent window rather than the default root window" },
#if OPT_ZICONBEEP
{ "-ziconbeep percent",    "beep and flag icon of window having hidden output" },
#endif
#if OPT_SAME_NAME
{ "-/+samename",           "turn on/off the no-flicker option for title and icon name" },
#endif
#if OPT_SESSION_MGT
{ "-/+sm",                 "turn on/off the session-management support" },
#endif
#if OPT_MAXIMIZE
{"-/+maximized",           "turn on/off maximize on startup" },
{"-/+fullscreen",          "turn on/off fullscreen on startup" },
#endif
{ NULL, NULL }};
/* *INDENT-ON* */

static const char *const help_message[] =
{
    "Fonts should be fixed width and, if both normal and bold are specified, should",
    "have the same size.  If only a normal font is specified, it will be used for",
    "both normal and bold text (by doing overstriking).  The -e option, if given,",
    "must appear at the end of the command line, otherwise the user's default shell",
    "will be started.  Options that start with a plus sign (+) restore the default.",
    NULL
};

int
xtermDisabledChar(void)
{
    int value = -1;
#if defined(_POSIX_VDISABLE) && defined(HAVE_UNISTD_H)
    value = _POSIX_VDISABLE;
#endif
#if defined(_PC_VDISABLE)
    if (value == -1) {
	value = (int) fpathconf(0, _PC_VDISABLE);
	if (value == -1) {
	    if (errno == 0)
		value = 0377;
	}
    }
#elif defined(VDISABLE)
    if (value == -1)
	value = VDISABLE;
#endif
    return value;
}

/*
 * Retrieve (possibly allocating) an atom from the server.  Cache the result.
 */
Atom
CachedInternAtom(Display *display, const char *name)
{
    /*
     * Aside from a couple of rarely used atoms, the others are all known.
     */
    static const char *const known_atoms[] =
    {
	"FONT",
	"WM_CLASS",
	"WM_DELETE_WINDOW",
	"_NET_ACTIVE_WINDOW",
	"_NET_FRAME_EXTENTS",
	"_NET_FRAME_EXTENTS",
	"_NET_SUPPORTED",
	"_NET_SUPPORTING_WM_CHECK",
	"_NET_WM_ALLOWED_ACTIONS",
	"_NET_WM_ICON_NAME",
	"_NET_WM_NAME",
	"_NET_WM_PID",
	"_NET_WM_STATE",
	"_NET_WM_STATE_ADD",
	"_NET_WM_STATE_FULLSCREEN",
	"_NET_WM_STATE_HIDDEN",
	"_NET_WM_STATE_MAXIMIZED_HORZ",
	"_NET_WM_STATE_MAXIMIZED_VERT",
	"_NET_WM_STATE_REMOVE",
	"_WIN_SUPPORTING_WM_CHECK",
#if defined(HAVE_XKB_BELL_EXT)
	XkbBN_Info,
	XkbBN_MarginBell,
	XkbBN_MinorError,
	XkbBN_TerminalBell,
#endif
#if defined(HAVE_XKBQUERYEXTENSION)
	"Num Lock",
	"Caps Lock",
	"Scroll Lock",
#endif
    };

#define NumKnownAtoms  XtNumber(known_atoms)

    /*
     * The "+1" entry of the array is used for the occasional atom which is not
     * predefined.
     */
    static struct {
	Atom atom;
	const char *name;
    } AtomCache[NumKnownAtoms + 1];

    Boolean found = False;
    Atom result = None;
    Cardinal i;

    TRACE(("intern_atom \"%s\"\n", name));

    /*
     * This should never happen as it implies xterm is aware of multiple
     * displays.
     */
    if (display != XtDisplay(toplevel)) {
	SysError(ERROR_GET_ATOM);
    }

    if (AtomCache[0].name == NULL) {
	/* pre-load a number of atoms in one request to reduce latency */
	Atom atom_return[NumKnownAtoms];
	int code;

	TRACE(("initialising atom list\n"));
	code = XInternAtoms(display,
			    (char **) known_atoms,
			    (int) NumKnownAtoms,
			    False,
			    atom_return);
	/*
	 * result should be Success, but actually returns BadRequest.
	 * manpage says XInternAtoms can generate BadAlloc and BadValue errors.
	 */
	if (code > BadRequest)
	    SysError(ERROR_GET_ATOM);

	for (i = 0; i < NumKnownAtoms; ++i) {
	    AtomCache[i].name = known_atoms[i];
	    AtomCache[i].atom = atom_return[i];
	}
    }

    /* Linear search is probably OK here, due to the small number of atoms */
    for (i = 0; i < NumKnownAtoms; i++) {
	if (strcmp(name, AtomCache[i].name) == 0) {
	    found = True;
	    result = AtomCache[i].atom;
	    break;
	}
    }

    if (!found) {
	if (AtomCache[NumKnownAtoms].name == NULL
	    || strcmp(AtomCache[NumKnownAtoms].name, name)) {
	    char *actual = x_strdup(name);
	    free((void *) AtomCache[NumKnownAtoms].name);
	    result = XInternAtom(display, actual, False);
	    AtomCache[NumKnownAtoms].atom = result;
	    AtomCache[NumKnownAtoms].name = actual;
	    TRACE(("...allocated new atom\n"));
	} else {
	    result = AtomCache[NumKnownAtoms].atom;
	    TRACE(("...reused cached atom\n"));
	}
    }
    TRACE(("...intern_atom -> %ld\n", result));
    return result;
}

/*
 * Decode a key-definition.  This combines the termcap and ttyModes, for
 * comparison.  Note that octal escapes in ttyModes are done by the normal
 * resource translation.  Also, ttyModes allows '^-' as a synonym for disabled.
 */
static int
decode_keyvalue(char **ptr, int termcap)
{
    char *string = *ptr;
    int value = -1;

    TRACE(("decode_keyvalue '%s'\n", string));
    if (*string == '^') {
	switch (*++string) {
	case '?':
	    value = ANSI_DEL;
	    break;
	case '-':
	    if (!termcap) {
		value = xtermDisabledChar();
		break;
	    }
	    /* FALLTHRU */
	default:
	    value = CONTROL(*string);
	    break;
	}
	++string;
    } else if (termcap && (*string == '\\')) {
	char *s = (string + 1);
	char *d;
	int temp = (int) strtol(s, &d, 8);
	if (PartS2L(s, d) && temp > 0) {
	    value = temp;
	    string = d;
	}
    } else {
	value = CharOf(*string);
	++string;
    }
    *ptr = string;
    TRACE(("...decode_keyvalue %#x\n", value));
    return value;
}

static int
matchArg(XrmOptionDescRec * table, const char *param)
{
    int result = -1;
    int n;
    int ch;

    for (n = 0; (ch = table->option[n]) != '\0'; ++n) {
	if (param[n] == ch) {
	    result = n;
	} else {
	    if (param[n] != '\0')
		result = -1;
	    break;
	}
    }

    return result;
}

/* return the number of argv[] entries which constitute arguments of option */
static int
countArg(XrmOptionDescRec * item)
{
    int result = 0;

    switch (item->argKind) {
    case XrmoptionNoArg:
	/* FALLTHRU */
    case XrmoptionIsArg:
	/* FALLTHRU */
    case XrmoptionStickyArg:
	break;
    case XrmoptionSepArg:
	/* FALLTHRU */
    case XrmoptionResArg:
	/* FALLTHRU */
    case XrmoptionSkipArg:
	result = 1;
	break;
    case XrmoptionSkipLine:
	break;
    case XrmoptionSkipNArgs:
	result = (int) (long) (item->value);
	break;
    }
    return result;
}

#define isOption(string) (Boolean)((string)[0] == '-' || (string)[0] == '+')

/*
 * Parse the argument list, more/less as XtInitialize, etc., would do, so we
 * can find our own "-help" and "-version" options reliably.  Improve on just
 * doing that, by detecting ambiguous options (things that happen to match the
 * abbreviated option we are examining), and making it smart enough to handle
 * "-d" as an abbreviation for "-display".  Doing this requires checking the
 * standard table (something that the X libraries should do).
 */
static XrmOptionDescRec *
parseArg(int *num, char **argv, char **valuep)
{
    /* table adapted from XtInitialize, used here to improve abbreviations */
    /* *INDENT-OFF* */
#define DATA(option,kind) { (char *) option, NULL, kind, (XPointer) 0 }
    static XrmOptionDescRec opTable[] = {
	DATA("+synchronous",	   XrmoptionNoArg),
	DATA("-background",	   XrmoptionSepArg),
	DATA("-bd",		   XrmoptionSepArg),
	DATA("-bg",		   XrmoptionSepArg),
	DATA("-bordercolor",	   XrmoptionSepArg),
	DATA("-borderwidth",	   XrmoptionSepArg),
	DATA("-bw",		   XrmoptionSepArg),
	DATA("-display",	   XrmoptionSepArg),
	DATA("-fg",		   XrmoptionSepArg),
	DATA("-fn",		   XrmoptionSepArg),
	DATA("-font",		   XrmoptionSepArg),
	DATA("-foreground",	   XrmoptionSepArg),
	DATA("-iconic",		   XrmoptionNoArg),
	DATA("-name",		   XrmoptionSepArg),
	DATA("-reverse",	   XrmoptionNoArg),
	DATA("-selectionTimeout",  XrmoptionSepArg),
	DATA("-synchronous",	   XrmoptionNoArg),
	DATA("-title",		   XrmoptionSepArg),
	DATA("-xnllanguage",	   XrmoptionSepArg),
	DATA("-xrm",		   XrmoptionResArg),
	DATA("-xtsessionID",	   XrmoptionSepArg),
	/* These xterm options are processed after XtOpenApplication */
#if defined(TIOCCONS) || defined(SRIOCSREDIR)
	DATA("-C",		   XrmoptionNoArg),
#endif /* TIOCCONS */
	DATA("-S",		   XrmoptionStickyArg),
	DATA("-D",		   XrmoptionNoArg),
    };
#undef DATA
    /* *INDENT-ON* */
    XrmOptionDescRec *result = NULL;
    Cardinal inlist;
    Cardinal limit = XtNumber(optionDescList) + XtNumber(opTable);
    int atbest = -1;
    Boolean exact = False;
    int ambiguous1 = -1;
    int ambiguous2 = -1;
    char *option;

#define ITEM(n) ((Cardinal)(n) < XtNumber(optionDescList) \
		 ? &optionDescList[n] \
		 : &opTable[(Cardinal)(n) - XtNumber(optionDescList)])

    if ((option = argv[*num]) != NULL) {
	Boolean need_value;
	Boolean have_value = False;
	int best = -1;
	char *value;

	TRACE(("parseArg %s\n", option));
	if ((value = argv[(*num) + 1]) != NULL) {
	    have_value = (Boolean) !isOption(value);
	}
	for (inlist = 0; inlist < limit; ++inlist) {
	    XrmOptionDescRec *check = ITEM(inlist);
	    int test;

	    test = matchArg(check, option);
	    if (test < 0)
		continue;

	    /* check for exact match */
	    if ((test + 1) == (int) strlen(check->option)) {
		if (check->argKind == XrmoptionStickyArg) {
		    if (strlen(option) > strlen(check->option)) {
			exact = True;
			atbest = (int) inlist;
			break;
		    }
		} else if ((test + 1) == (int) strlen(option)) {
		    exact = True;
		    atbest = (int) inlist;
		    break;
		}
	    }

	    need_value = (Boolean) (test > 0 && countArg(check) > 0);

	    if (need_value && value != NULL) {
		;
	    } else if (need_value ^ have_value) {
		TRACE(("...skipping, need %d vs have %d\n", need_value, have_value));
		continue;
	    }

	    /* special-case for our own options - always allow abbreviation */
	    if (test > 0
		&& ITEM(inlist)->argKind >= XrmoptionSkipArg) {
		atbest = (int) inlist;
		if (ITEM(inlist)->argKind == XrmoptionSkipNArgs) {
		    /* in particular, silence a warning about ambiguity */
		    exact = 1;
		}
		break;
	    }
	    if (test > best) {
		best = test;
		atbest = (int) inlist;
	    } else if (test == best) {
		if (atbest >= 0) {
		    if (atbest > 0) {
			ambiguous1 = (int) inlist;
			ambiguous2 = (int) atbest;
		    }
		    atbest = -1;
		}
	    }
	}
    }

    *valuep = NULL;
    if (atbest >= 0) {
	result = ITEM(atbest);
	if (!exact) {
	    if (ambiguous1 >= 0 && ambiguous2 >= 0) {
		xtermWarning("ambiguous option \"%s\" vs \"%s\"\n",
			     ITEM(ambiguous1)->option,
			     ITEM(ambiguous2)->option);
	    } else if (strlen(option) > strlen(result->option)) {
		result = NULL;
	    }
	}
	if (result != NULL) {
	    TRACE(("...result %s\n", result->option));
	    /* expand abbreviations */
	    if (result->argKind != XrmoptionStickyArg) {
		if (strcmp(argv[*num], result->option)) {
		    argv[*num] = x_strdup(result->option);
		}
	    }

	    /* adjust (*num) to skip option value */
	    (*num) += countArg(result);
	    TRACE(("...next %s\n", NonNull(argv[*num])));
	    if (result->argKind == XrmoptionSkipArg) {
		*valuep = argv[*num];
		TRACE(("...parameter %s\n", NonNull(*valuep)));
	    }
	}
    }
#undef ITEM
    return result;
}

static void
Syntax(char *badOption)
{
    OptionHelp *opt;
    OptionHelp *list = sortedOpts(xtermOptions, optionDescList, XtNumber(optionDescList));
    int col;

    TRACE(("Syntax error at %s\n", badOption));
    xtermWarning("bad command line option \"%s\"\r\n\n", badOption);

    fprintf(stderr, "usage:  %s", ProgramName);
    col = 8 + (int) strlen(ProgramName);
    for (opt = list; opt->opt; opt++) {
	int len = 3 + (int) strlen(opt->opt);	/* space [ string ] */
	if (col + len > 79) {
	    fprintf(stderr, "\r\n   ");		/* 3 spaces */
	    col = 3;
	}
	fprintf(stderr, " [%s]", opt->opt);
	col += len;
    }

    fprintf(stderr, "\r\n\nType %s -help for a full description.\r\n\n",
	    ProgramName);
    exit(ERROR_MISC);
}

static void
Version(void)
{
    printf("%s\n", xtermVersion());
    fflush(stdout);
}

static void
Help(void)
{
    OptionHelp *opt;
    OptionHelp *list = sortedOpts(xtermOptions, optionDescList, XtNumber(optionDescList));
    const char *const *cpp;

    printf("%s usage:\n    %s [-options ...] [-e command args]\n\n",
	   xtermVersion(), ProgramName);
    printf("where options include:\n");
    for (opt = list; opt->opt; opt++) {
	printf("    %-28s %s\n", opt->opt, opt->desc);
    }

    putchar('\n');
    for (cpp = help_message; *cpp; cpp++)
	puts(*cpp);
    putchar('\n');
    fflush(stdout);
}

static void
NeedParam(XrmOptionDescRec * option_ptr, const char *option_val)
{
    if (IsEmpty(option_val)) {
	xtermWarning("option %s requires a value\n", option_ptr->option);
	exit(ERROR_MISC);
    }
}

#if defined(TIOCCONS) || defined(SRIOCSREDIR)
/* ARGSUSED */
static Boolean
ConvertConsoleSelection(Widget w GCC_UNUSED,
			Atom *selection GCC_UNUSED,
			Atom *target GCC_UNUSED,
			Atom *type GCC_UNUSED,
			XtPointer *value GCC_UNUSED,
			unsigned long *length GCC_UNUSED,
			int *format GCC_UNUSED)
{
    /* we don't save console output, so can't offer it */
    return False;
}
#endif /* TIOCCONS */

/*
 * DeleteWindow(): Action proc to implement ICCCM delete_window.
 */
/* ARGSUSED */
static void
DeleteWindow(Widget w,
	     XEvent *event GCC_UNUSED,
	     String *params GCC_UNUSED,
	     Cardinal *num_params GCC_UNUSED)
{
#if OPT_TEK4014
    if (w == toplevel) {
	if (TEK4014_SHOWN(term))
	    hide_vt_window();
	else
	    do_hangup(w, (XtPointer) 0, (XtPointer) 0);
    } else if (TScreenOf(term)->Vshow)
	hide_tek_window();
    else
#endif
	do_hangup(w, (XtPointer) 0, (XtPointer) 0);
}

/* ARGSUSED */
static void
xtermColorEvents(Widget w GCC_UNUSED,
		 XEvent *event,
		 String *params GCC_UNUSED,
		 Cardinal *num_params GCC_UNUSED)
{
    if (event->xclient.format == 8) {
#define BSIZE sizeof(event->xclient.data.b)
	/*
	 * The OSC must begin with a number (which is enabled in our table),
	 * not contain "?" (which would force a query/response), and have a
	 * single parameter after the number, separated by a ";".
	 */
	int code;
	char param;
	char *source = event->xclient.data.b;

	source[BSIZE - 1] = '\0';

	if (sscanf(source, "%d;%c", &code, &param) == 2
	    && code > 0
	    && code < OSC_NCOLORS
	    && TScreenOf(term)->color_events[code]
	    && strchr(source, '?') == NULL) {
	    TRACE(("xtermColorEvents code %d: %s\n", code, source));
	    do_osc(term, (Char *) source, strlen(source), 1 /* final */ );
	} else {
	    TRACE(("xtermColorEvents ignore %s\n", source));
	}
    } else {
	TRACE(("xtermColorEvents invalid format %d, expected 8\n",
	       event->xclient.format));
    }
}

/* ARGSUSED */
static void
KeyboardMapping(Widget w GCC_UNUSED,
		XEvent *event,
		String *params GCC_UNUSED,
		Cardinal *num_params GCC_UNUSED)
{
    switch (event->type) {
    case MappingNotify:
	XRefreshKeyboardMapping(&event->xmapping);
	break;
    }
}

static XtActionsRec actionProcs[] =
{
    {"DeleteWindow", DeleteWindow},
    {"ColorEvents", xtermColorEvents},
    {"KeyboardMapping", KeyboardMapping},
};

/*
 * Some platforms use names such as /dev/tty01, others /dev/pts/1.  Parse off
 * the "tty01" or "pts/1" portion, and return that for use as an identifier for
 * utmp.
 */
static char *
my_pty_name(char *device)
{
    size_t len = strlen(device);
    Bool name = False;

    while (len != 0) {
	int ch = device[len - 1];
	if (isdigit(ch)) {
	    len--;
	} else if (ch == '/') {
	    if (name)
		break;
	    len--;
	} else if (isalpha(CharOf(ch))) {
	    name = True;
	    len--;
	} else {
	    break;
	}
    }
    TRACE(("my_pty_name(%s) -> '%s'\n", device, device + len));
    return device + len;
}

/*
 * If the name contains a '/', it is a "pts/1" case.  Otherwise, return the
 * last few characters for a utmp identifier.
 */
static char *
my_pty_id(char *device)
{
    char *name = my_pty_name(device);
    char *leaf = x_basename(name);

    if (name == leaf) {		/* no '/' in the name */
	int len = (int) strlen(leaf);
	if (PTYCHARLEN < len)
	    leaf = leaf + (len - PTYCHARLEN);
    }
    TRACE(("my_pty_id  (%s) -> '%s'\n", NonNull(device), NonNull(leaf)));
    return leaf;
}

/*
 * Set the tty/pty identifier
 */
static void
set_pty_id(char *device, char *id)
{
    char *name = my_pty_name(device);
    char *leaf = x_basename(name);

    if (name == leaf) {
	strcpy(my_pty_id(device), id);
    } else {
	strcpy(leaf, id);
    }
    TRACE(("set_pty_id(%s) -> '%s'\n", id, device));
}

/*
 * The original -S option accepts two characters to identify the pty, and a
 * file-descriptor (assumed to be nonzero).  That is not general enough, so we
 * check first if the option contains a '/' to delimit the two fields, and if
 * not, fall-thru to the original logic.
 */
static Bool
ParseSccn(char *option)
{
    char *leaf = x_basename(option);
    Bool code = False;

    passedPty = x_strdup(option);
    if (leaf != option) {
	if (leaf - option > 0
	    && isdigit(CharOf(*leaf))
	    && sscanf(leaf, "%d", &am_slave) == 1) {
	    size_t len = (size_t) (leaf - option - 1);
	    /*
	     * If we have a slash, we only care about the part after the slash,
	     * which is a file-descriptor.  The part before the slash can be
	     * the /dev/pts/XXX value, but since we do not need to reopen it,
	     * it is useful mainly for display in a "ps -ef".
	     */
	    passedPty[len] = 0;
	    code = True;
	}
    } else {
	code = (sscanf(option, "%c%c%d",
		       passedPty, passedPty + 1, &am_slave) == 3);
	passedPty[2] = '\0';
    }
    TRACE(("ParseSccn(%s) = '%s' %d (%s)\n", option,
	   passedPty, am_slave, code ? "OK" : "ERR"));
    return code;
}

#if defined(USE_SYSV_UTMP) && !defined(USE_UTEMPTER)
/*
 * From "man utmp":
 * xterm and other terminal emulators directly create a USER_PROCESS record
 * and generate the ut_id by using the last two letters of /dev/ttyp%c or by
 * using p%d for /dev/pts/%d.  If they find a DEAD_PROCESS for this id, they
 * recycle it, otherwise they create a new entry.  If they can, they will mark
 * it as DEAD_PROCESS on exiting and it is advised that they null ut_line,
 * ut_time, ut_user and ut_host as well.
 *
 * Generally ut_id allows no more than 3 characters (plus null), even if the
 * pty implementation allows more than 3 digits.
 */
static char *
my_utmp_id(char *device)
{
    typedef struct UTMP_STR UTMP_STRUCT;
#define	UTIDSIZE	(sizeof(((UTMP_STRUCT *)NULL)->ut_id))
    static char result[UTIDSIZE + 1];

#if defined(__SCO__) || defined(__UNIXWARE__)
    /*
     * Legend does not support old-style pty's, has no related compatibility
     * issues, and can use the available space in ut_id differently from the
     * default convention.
     *
     * This scheme is intended to avoid conflicts both with other users of
     * utmpx as well as between multiple xterms.  First, Legend uses all of the
     * characters of ut_id, and adds no terminating NUL is required (the
     * default scheme may add a trailing NUL).  Second, all xterm entries will
     * start with the letter 'x' followed by three digits, which will be the
     * last three digits of the device name, regardless of the format of the
     * device name, with leading 0's added where necessary.  For instance, an
     * xterm on /dev/pts/3 will have a ut_id of x003; an xterm on /dev/pts123
     * will have a ut_id of x123.  Under the other convention, /dev/pts/3 would
     * have a ut_id of p3 and /dev/pts123 would have a ut_id of p123.
     */
    int len, n;

    len = strlen(device);
    n = UTIDSIZE;
    result[n] = '\0';
    while ((n > 0) && (len > 0) && isdigit(device[len - 1]))
	result[--n] = device[--len];
    while (n > 0)
	result[--n] = '0';
    result[0] = 'x';
#else
    char *name = my_pty_name(device);
    char *leaf = x_basename(name);
    size_t len = strlen(leaf);

    if ((UTIDSIZE - 1) < len)
	leaf = leaf + (len - (UTIDSIZE - 1));
    sprintf(result, "p%s", leaf);
#endif

    TRACE(("my_utmp_id (%s) -> '%s'\n", device, result));
    return result;
}
#endif /* USE_SYSV_UTMP */

#ifdef USE_POSIX_SIGNALS

typedef void (*sigfunc) (int);

/* make sure we sure we ignore SIGCHLD for the cases parent
   has just been stopped and not actually killed */

static sigfunc
posix_signal(int signo, sigfunc func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
#ifdef SA_RESTART
    act.sa_flags = SA_NOCLDSTOP | SA_RESTART;
#else
    act.sa_flags = SA_NOCLDSTOP;
#endif
    if (sigaction(signo, &act, &oact) < 0)
	return (SIG_ERR);
    return (oact.sa_handler);
}

#endif /* USE_POSIX_SIGNALS */

#if defined(DISABLE_SETUID) || defined(USE_UTMP_SETGID)
static void
disableSetUid(void)
{
    TRACE(("process %d disableSetUid\n", (int) getpid()));
    if (setuid(save_ruid) == -1) {
	xtermWarning("unable to reset uid\n");
	exit(ERROR_MISC);
    }
    TRACE_IDS;
}
#else
#define disableSetUid()		/* nothing */
#endif /* DISABLE_SETUID */

#if defined(DISABLE_SETGID) || defined(USE_UTMP_SETGID)
static void
disableSetGid(void)
{
    TRACE(("process %d disableSetGid\n", (int) getpid()));
    if (setegid(save_rgid) == -1) {
	xtermWarning("unable to reset effective gid\n");
	exit(ERROR_MISC);
    }
    TRACE_IDS;
}
#else
#define disableSetGid()		/* nothing */
#endif /* DISABLE_SETGID */

#if defined(HAVE_POSIX_SAVED_IDS)
#if (!defined(USE_UTEMPTER) || !defined(DISABLE_SETGID))
static void
setEffectiveGroup(gid_t group)
{
    TRACE(("process %d setEffectiveGroup(%d)\n", (int) getpid(), (int) group));
    if (setegid(group) == -1) {
	xtermPerror("setegid(%d)", (int) group);
    }
    TRACE_IDS;
}
#endif

#if !defined(USE_UTMP_SETGID) && (!defined(USE_UTEMPTER) || !defined(DISABLE_SETUID))
static void
setEffectiveUser(uid_t user)
{
    TRACE(("process %d setEffectiveUser(%d)\n", (int) getpid(), (int) user));
    if (seteuid(user) == -1) {
	xtermPerror("seteuid(%d)", (int) user);
    }
    TRACE_IDS;
}
#endif
#endif /* HAVE_POSIX_SAVED_IDS */

#if OPT_LUIT_PROG
static Boolean
complex_command(char **args)
{
    Boolean result = False;
    if (x_countargv(args) == 1) {
	char *check = xtermFindShell(args[0], False);
	if (check == NULL) {
	    result = True;
	} else {
	    free(check);
	}
    }
    return result;
}
#endif

static unsigned
lookup_baudrate(const char *value)
{
    struct speed {
	unsigned given_speed;	/* values for 'ospeed' */
	unsigned actual_speed;	/* the actual speed */
    };

#define DATA(number) { B##number, number }

    static struct speed const speeds[] =
    {
	DATA(0),
	DATA(50),
	DATA(75),
	DATA(110),
	DATA(134),
	DATA(150),
	DATA(200),
	DATA(300),
	DATA(600),
	DATA(1200),
	DATA(1800),
	DATA(2400),
	DATA(4800),
	DATA(9600),
#ifdef B19200
	DATA(19200),
#elif defined(EXTA)
	{EXTA, 19200},
#endif
#ifdef B28800
	DATA(28800),
#endif
#ifdef B38400
	DATA(38400),
#elif defined(EXTB)
	{EXTB, 38400},
#endif
#ifdef B57600
	DATA(57600),
#endif
#ifdef B76800
	DATA(76800),
#endif
#ifdef B115200
	DATA(115200),
#endif
#ifdef B153600
	DATA(153600),
#endif
#ifdef B230400
	DATA(230400),
#endif
#ifdef B307200
	DATA(307200),
#endif
#ifdef B460800
	DATA(460800),
#endif
#ifdef B500000
	DATA(500000),
#endif
#ifdef B576000
	DATA(576000),
#endif
#ifdef B921600
	DATA(921600),
#endif
#ifdef B1000000
	DATA(1000000),
#endif
#ifdef B1152000
	DATA(1152000),
#endif
#ifdef B1500000
	DATA(1500000),
#endif
#ifdef B2000000
	DATA(2000000),
#endif
#ifdef B2500000
	DATA(2500000),
#endif
#ifdef B3000000
	DATA(3000000),
#endif
#ifdef B3500000
	DATA(3500000),
#endif
#ifdef B4000000
	DATA(4000000),
#endif
    };
#undef DATA
    unsigned result = 0;

    if (x_toupper(*value) == 'B')
	value++;

    if (isdigit(CharOf(*value))) {
	char *next;
	long check = strtol(value, &next, 10);

	if (FullS2L(value, next) && (check > 0)) {
	    Cardinal n;
	    for (n = 0; n < XtNumber(speeds); ++n) {
		if (speeds[n].actual_speed == (unsigned) check) {
		    result = speeds[n].given_speed;
		    break;
		}
	    }
	}
    }
    if (result == 0) {
	fprintf(stderr, "unsupported value for baudrate: %s\n", value);
    }
    return result;
}

int
get_tty_erase(int fd, int default_erase, const char *tag)
{
    int result = default_erase;
    int rc;

#ifdef TERMIO_STRUCT
    TERMIO_STRUCT my_tio;
    rc = ttyGetAttr(fd, &my_tio);
    if (rc == 0)
	result = my_tio.c_cc[VERASE];
#else /* !TERMIO_STRUCT */
    struct sgttyb my_sg;
    rc = ioctl(fd, TIOCGETP, (char *) &my_sg);
    if (rc == 0)
	result = my_sg.sg_erase;
#endif /* TERMIO_STRUCT */
    TRACE(("%s erase:%d (from %s)\n", (rc == 0) ? "OK" : "FAIL", result, tag));
    (void) tag;
    return result;
}

int
get_tty_lnext(int fd, int default_lnext, const char *tag)
{
#ifdef TERMIO_STRUCT
    TERMIO_STRUCT my_tio;
    int rc = ttyGetAttr(fd, &my_tio);
    int result = (rc == 0) ? my_tio.c_cc[VLNEXT] : default_lnext;
#elif defined(HAS_LTCHARS)
    struct ltchars my_ltc;
    int rc = ioctl(fd, TIOCGLTC, (char *) &my_ltc);
    int result = (rc == 0) ? my_ltc.t_lnextc : default_lnext;
#else
    int result = XTERM_LNEXT;
#endif /* TERMIO_STRUCT */
    TRACE(("%s lnext:%d (from %s)\n", (rc == 0) ? "OK" : "FAIL", result, tag));
    (void) tag;
    return result;
}

int
main(int argc, char *argv[]ENVP_ARG)
{
#if OPT_MAXIMIZE
#define DATA(name) { #name, es##name }
    static const FlagList tblFullscreen[] =
    {
	DATA(Always),
	DATA(Never)
    };
#undef DATA
#endif

    Widget form_top, menu_top;
    Dimension menu_high;
    TScreen *screen;
    int mode;
    char *my_class = x_strdup(DEFCLASS);
    unsigned line_speed = VAL_LINE_SPEED;
    Window winToEmbedInto = None;
#if defined(HAVE_LIB_XAW3DXFT)
    Xaw3dXftData *xaw3dxft_data;
#endif

    ProgramName = x_strdup(x_basename(argv[0]));
    ProgramPath = xtermFindShell(argv[0], True);
    if (ProgramPath != NULL)
	argv[0] = ProgramPath;

#ifdef HAVE_POSIX_SAVED_IDS
    save_euid = geteuid();
    save_egid = getegid();
#endif

    save_ruid = getuid();
    save_rgid = getgid();

#if defined(DISABLE_SETUID) || defined(DISABLE_SETGID)
#if defined(DISABLE_SETUID)
    disableSetUid();
#endif
#if defined(DISABLE_SETGID)
    disableSetGid();
#endif
    TRACE_IDS;
#endif

    /* extra length in case longer tty name like /dev/ttyq255 */
    ttydev = TypeMallocN(char, sizeof(TTYDEV) + 80);
#ifdef USE_PTY_DEVICE
    ptydev = TypeMallocN(char, sizeof(PTYDEV) + 80);
    if (!ttydev || !ptydev)
#else
    if (!ttydev)
#endif
    {
	xtermWarning("unable to allocate memory for ttydev or ptydev\n");
	exit(ERROR_MISC);
    }
    strcpy(ttydev, TTYDEV);
#ifdef USE_PTY_DEVICE
    strcpy(ptydev, PTYDEV);
#endif

#if defined(USE_UTMP_SETGID)
    get_pty(NULL, NULL);
    disableSetUid();
    disableSetGid();
    TRACE_IDS;
#define get_pty(pty, from) really_get_pty(pty, from)
#endif

    /* Do these first, since we may not be able to open the display */
    TRACE_OPTS(xtermOptions, optionDescList, XtNumber(optionDescList));
    TRACE_ARGV("Before XtOpenApplication", argv);
    restart_params = 0;
    if (argc > 1) {
	int n;
	Bool quit = False;

	for (n = 1; n < argc; n++) {
	    XrmOptionDescRec *option_ptr;
	    char *option_value;

	    if ((option_ptr = parseArg(&n, argv, &option_value)) == NULL) {
		if (argv[n] == NULL) {
		    break;
		} else if (isOption(argv[n])) {
		    Syntax(argv[n]);
		} else if (explicit_shname != NULL) {
		    xtermWarning("Explicit shell already was %s\n", explicit_shname);
		    Syntax(argv[n]);
		}
		explicit_shname = xtermFindShell(argv[n], True);
		if (explicit_shname == NULL)
		    exit(0);
		TRACE(("...explicit shell %s\n", explicit_shname));
		restart_params = (argc - n);
	    } else if (!strcmp(option_ptr->option, "-e")) {
		command_to_exec = (argv + n + 1);
		if (!command_to_exec[0])
		    Syntax(argv[n]);
		restart_params = (argc - n);
		break;
	    } else if (!strcmp(option_ptr->option, "-version")) {
		Version();
		quit = True;
	    } else if (!strcmp(option_ptr->option, "-help")) {
		Help();
		quit = True;
	    } else if (!strcmp(option_ptr->option, "-baudrate")) {
		NeedParam(option_ptr, option_value);
		if ((line_speed = lookup_baudrate(option_value)) == 0) {
		    Help();
		    quit = True;
		}
	    } else if (!strcmp(option_ptr->option, "-class")) {
		NeedParam(option_ptr, option_value);
		free(my_class);
		if ((my_class = x_strdup(option_value)) == NULL) {
		    Help();
		    quit = True;
		}
	    } else if (!strcmp(option_ptr->option, "-into")) {
		char *endPtr;
		NeedParam(option_ptr, option_value);
		winToEmbedInto = (Window) strtol(option_value, &endPtr, 0);
		if (!FullS2L(option_value, endPtr)) {
		    Help();
		    quit = True;
		}
	    }
	}
	if (quit)
	    exit(0);
	/*
	 * If there is anything left unparsed, and we're not using "-e",
	 * then give up.
	 */
	if (n < argc && !command_to_exec) {
	    Syntax(argv[n]);
	}
    }

    /* This dumped core on HP-UX 9.05 with X11R5 */
#if OPT_I18N_SUPPORT
    XtSetLanguageProc(NULL, NULL, NULL);
#endif

    /* enable Xft support in Xaw3DXft */
#if defined(HAVE_LIB_XAW3DXFT)
    GET_XAW3DXFT_DATA(xaw3dxft_data);
    xaw3dxft_data->encoding = -1;
#endif

#ifdef TERMIO_STRUCT		/* { */
    /* Initialization is done here rather than above in order
     * to prevent any assumptions about the order of the contents
     * of the various terminal structures (which may change from
     * implementation to implementation).
     */
    memset(&d_tio, 0, sizeof(d_tio));
    d_tio.c_iflag = ICRNL | IXON;
    d_tio.c_oflag = TAB3 | D_TIO_FLAGS;
    {
	Cardinal nn;

	/* fill in default-values */
	for (nn = 0; nn < XtNumber(ttyChars); ++nn) {
	    if (validTtyChar(d_tio, nn)) {
		d_tio.c_cc[ttyChars[nn].sysMode] =
		    (cc_t) ttyChars[nn].myDefault;
	    }
	}
    }
#if defined(macII) || defined(ATT) || defined(CRAY)	/* { */
    d_tio.c_cflag = line_speed | CS8 | CREAD | PARENB | HUPCL;
    d_tio.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK;
#ifdef ECHOKE
    d_tio.c_lflag |= ECHOKE | IEXTEN;
#endif
#ifdef ECHOCTL
    d_tio.c_lflag |= ECHOCTL | IEXTEN;
#endif
#ifndef USE_TERMIOS		/* { */
    d_tio.c_line = 0;
#endif /* } */
#ifdef HAS_LTCHARS		/* { */
    d_ltc.t_suspc = CSUSP;	/* t_suspc */
    d_ltc.t_dsuspc = CDSUSP;	/* t_dsuspc */
    d_ltc.t_rprntc = CRPRNT;
    d_ltc.t_flushc = CFLUSH;
    d_ltc.t_werasc = CWERASE;
    d_ltc.t_lnextc = CLNEXT;
#endif /* } HAS_LTCHARS */
#ifdef TIOCLSET			/* { */
    d_lmode = 0;
#endif /* } TIOCLSET */
#else /* }{ else !macII, ATT, CRAY */
#ifndef USE_POSIX_TERMIOS
#ifdef BAUD_0			/* { */
    d_tio.c_cflag = CS8 | CREAD | PARENB | HUPCL;
#else /* }{ !BAUD_0 */
    d_tio.c_cflag = line_speed | CS8 | CREAD | PARENB | HUPCL;
#endif /* } !BAUD_0 */
#else /* USE_POSIX_TERMIOS */
    d_tio.c_cflag = CS8 | CREAD | PARENB | HUPCL;
    cfsetispeed(&d_tio, line_speed);
    cfsetospeed(&d_tio, line_speed);
#endif
    d_tio.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK;
#ifdef ECHOKE
    d_tio.c_lflag |= ECHOKE | IEXTEN;
#endif
#ifdef ECHOCTL
    d_tio.c_lflag |= ECHOCTL | IEXTEN;
#endif
#ifndef USE_POSIX_TERMIOS
#ifdef NTTYDISC
    d_tio.c_line = NTTYDISC;
#else
    d_tio.c_line = 0;
#endif
#endif /* USE_POSIX_TERMIOS */
#ifdef __sgi
    d_tio.c_cflag &= ~(HUPCL | PARENB);
    d_tio.c_iflag |= BRKINT | ISTRIP | IGNPAR;
#endif
    {
	Cardinal nn;
	int i;

	/* try to inherit tty settings */
	for (i = 0; i <= 2; i++) {
	    TERMIO_STRUCT deftio;
	    if (ttyGetAttr(i, &deftio) == 0) {
		for (nn = 0; nn < XtNumber(ttyChars); ++nn) {
		    if (validTtyChar(d_tio, nn)) {
			d_tio.c_cc[ttyChars[nn].sysMode] =
			    deftio.c_cc[ttyChars[nn].sysMode];
		    }
		}
		break;
	    }
	}
    }
#if defined(USE_TERMIOS) || defined(USE_POSIX_TERMIOS)	/* { */
    d_tio.c_cc[VMIN] = 1;
    d_tio.c_cc[VTIME] = 0;
#endif /* } */
#ifdef HAS_LTCHARS		/* { */
    d_ltc.t_suspc = CharOf('\000');	/* t_suspc */
    d_ltc.t_dsuspc = CharOf('\000');	/* t_dsuspc */
    d_ltc.t_rprntc = CharOf('\377');	/* reserved... */
    d_ltc.t_flushc = CharOf('\377');
    d_ltc.t_werasc = CharOf('\377');
    d_ltc.t_lnextc = CharOf('\377');
#endif /* } HAS_LTCHARS */

#ifdef TIOCLSET			/* { */
    d_lmode = 0;
#endif /* } TIOCLSET */
#endif /* } macII, ATT, CRAY */
#endif /* } TERMIO_STRUCT */

    /* Init the Toolkit. */
    {
#if defined(HAVE_POSIX_SAVED_IDS) && !defined(USE_UTMP_SETGID) && !defined(USE_UTEMPTER)
	setEffectiveGroup(save_rgid);
	setEffectiveUser(save_ruid);
	TRACE_IDS;
#endif
	toplevel = xtermOpenApplication(&app_con,
					my_class,
					optionDescList,
					XtNumber(optionDescList),
					&argc, argv,
					fallback_resources,
					sessionShellWidgetClass,
					NULL, 0);
	TRACE(("created toplevel widget %p, window %#lx\n",
	       (void *) toplevel, XtWindow(toplevel)));

	XtGetApplicationResources(toplevel, (XtPointer) &resource,
				  application_resources,
				  XtNumber(application_resources), NULL, 0);
	TRACE_XRES();
#ifdef HAVE_LIB_XCURSOR
	if (!strcmp(resource.cursorTheme, "none")) {
	    TRACE(("startup with no cursorTheme\n"));
	    init_colored_cursor(XtDisplay(toplevel));
	} else {
	    const char *theme = resource.cursorTheme;
	    if (IsEmpty(theme))
		theme = "default";
	    TRACE(("startup with \"%s\" cursorTheme\n", theme));
	    xtermSetenv("XCURSOR_THEME", theme);
	}
#endif
#if USE_DOUBLE_BUFFER
	if (resource.buffered_fps <= 0)
	    resource.buffered_fps = DEF_BUFFER_RATE;
	if (resource.buffered_fps > 100)
	    resource.buffered_fps = 100;
#endif
#if OPT_MAXIMIZE
	resource.fullscreen = extendedBoolean(resource.fullscreen_s,
					      tblFullscreen,
					      esLAST);
#endif
	VTInitTranslations();
#if OPT_PTY_HANDSHAKE
	resource.wait_for_map0 = resource.wait_for_map;
#endif

#if defined(HAVE_POSIX_SAVED_IDS) && !defined(USE_UTMP_SETGID)
#if !defined(DISABLE_SETUID) || !defined(DISABLE_SETGID)
#if !defined(DISABLE_SETUID)
	setEffectiveUser(save_euid);
#endif
#if !defined(DISABLE_SETGID)
	setEffectiveGroup(save_egid);
#endif
	TRACE_IDS;
#endif
#endif
    }

    /*
     * ICCCM delete_window.
     */
    XtAppAddActions(app_con, actionProcs, XtNumber(actionProcs));

    /*
     * fill in terminal modes
     */
    if (resource.tty_modes) {
	int n = parse_tty_modes(resource.tty_modes);
	if (n < 0) {
	    xtermWarning("bad tty modes \"%s\"\n", resource.tty_modes);
	} else if (n > 0) {
	    override_tty_modes = True;
	}
    }
    initZIconBeep();
    hold_screen = resource.hold_screen ? 1 : 0;
    if (resource.icon_geometry != NULL) {
	int scr, junk;
	int ix, iy;
	Arg args[2];

	for (scr = 0;		/* yyuucchh */
	     XtScreen(toplevel) != ScreenOfDisplay(XtDisplay(toplevel), scr);
	     scr++) ;

	args[0].name = XtNiconX;
	args[1].name = XtNiconY;
	XGeometry(XtDisplay(toplevel), scr, resource.icon_geometry, "",
		  0, 0, 0, 0, 0, &ix, &iy, &junk, &junk);
	args[0].value = (XtArgVal) ix;
	args[1].value = (XtArgVal) iy;
	XtSetValues(toplevel, args, 2);
    }

    XtSetValues(toplevel, ourTopLevelShellArgs,
		number_ourTopLevelShellArgs);

#if OPT_WIDE_CHARS
    /* seems as good a place as any */
    init_classtab();
#endif

    /* Parse the rest of the command line */
    TRACE_ARGV("After XtOpenApplication", argv);
    for (argc--, argv++; argc > 0; argc--, argv++) {
	if (!isOption(*argv)) {
	    if (argc > 1)
		Syntax(*argv);
	    continue;
	}

	TRACE(("parsing %s\n", argv[0]));
	switch (argv[0][1]) {
	case 'C':
#if defined(TIOCCONS) || defined(SRIOCSREDIR)
#ifndef __sgi
	    {
		struct stat sbuf;

		/* Must be owner and have read/write permission.
		   xdm cooperates to give the console the right user. */
		if (!stat("/dev/console", &sbuf) &&
		    (sbuf.st_uid == save_ruid) &&
		    !access("/dev/console", R_OK | W_OK)) {
		    Console = True;
		} else
		    Console = False;
	    }
#else /* __sgi */
	    Console = True;
#endif /* __sgi */
#endif /* TIOCCONS */
	    continue;
	case 'S':
	    if (!ParseSccn(*argv + 2))
		Syntax(*argv);
	    continue;
#ifdef DEBUG
	case 'D':
	    debug = True;
	    continue;
#endif /* DEBUG */
	case 'b':
	    if (strcmp(argv[0], "-baudrate"))
		Syntax(*argv);
	    argc--;
	    argv++;
	    continue;
	case 'c':
	    if (strcmp(argv[0], "-class"))
		Syntax(*argv);
	    argc--;
	    argv++;
	    continue;
	case 'e':
	    if (strcmp(argv[0], "-e"))
		Syntax(*argv);
	    command_to_exec = (argv + 1);
	    break;
	case 'i':
	    if (strcmp(argv[0], "-into"))
		Syntax(*argv);
	    argc--;
	    argv++;
	    continue;

	default:
	    Syntax(*argv);
	}
	break;
    }

    SetupMenus(toplevel, &form_top, &menu_top, &menu_high);

    term = (XtermWidget) XtVaCreateManagedWidget("vt100", xtermWidgetClass,
						 form_top,
#if OPT_TOOLBAR
						 XtNmenuBar, menu_top,
						 XtNresizable, True,
						 XtNfromVert, menu_top,
						 XtNleft, XawChainLeft,
						 XtNright, XawChainRight,
						 XtNtop, XawChainTop,
						 XtNbottom, XawChainBottom,
						 XtNmenuHeight, menu_high,
#endif
						 (XtPointer) 0);
    TRACE(("created vt100 widget %p, window %#lx\n",
	   (void *) term, XtWindow(term)));
    decode_keyboard_type(term, &resource);

    screen = TScreenOf(term);
    screen->inhibit = 0;

#ifdef ALLOWLOGGING
    if (term->misc.logInhibit)
	screen->inhibit |= I_LOG;
#endif
    if (term->misc.signalInhibit)
	screen->inhibit |= I_SIGNAL;
#if OPT_TEK4014
    if (term->misc.tekInhibit)
	screen->inhibit |= I_TEK;
#endif

    /*
     * We might start by showing the tek4014 window.
     */
#if OPT_TEK4014
    if (screen->inhibit & I_TEK)
	TEK4014_ACTIVE(term) = False;

    if (TEK4014_ACTIVE(term) && !TekInit())
	SysError(ERROR_INIT);
#endif

    /*
     * Start the toolbar at this point, after the first window has been setup.
     */
#if OPT_TOOLBAR
    ShowToolbar(resource.toolBar);
#endif

    xtermOpenSession();

    /*
     * Set title and icon name if not specified
     */
    if (command_to_exec) {
	Arg args[2];

	if (!resource.title)
	    resource.title = x_basename(command_to_exec[0]);
	if (!resource.icon_name)
	    resource.icon_name = resource.title;

	XtSetArg(args[0], XtNtitle, resource.title);
	XtSetArg(args[1], XtNiconName, resource.icon_name);

	TRACE(("setting:\n\ttitle \"%s\"\n\ticon \"%s\"\n\thint \"%s\"\n\tbased on command \"%s\"\n",
	       resource.title,
	       resource.icon_name,
	       NonNull(resource.icon_hint),
	       *command_to_exec));

	XtSetValues(toplevel, args, 2);
    } else if (IsEmpty(resource.title) && strcmp(my_class, DEFCLASS)) {
	Arg args[2];
	int n;

	resource.title = x_strdup(my_class);
	for (n = 0; resource.title[n]; ++n) {
	    if (isalpha(CharOf(resource.title[n])))
		resource.title[n] = (char) tolower(CharOf(resource.title[n]));
	}
	TRACE(("setting:\n\ttitle \"%s\"\n", resource.title));
	XtSetArg(args[0], XtNtitle, resource.title);
	XtSetValues(toplevel, args, 1);
    }
#if OPT_LUIT_PROG
    if (term->misc.callfilter) {
	char **split_filter = x_splitargs(term->misc.localefilter);
	unsigned count_split = x_countargv(split_filter);
	unsigned count_exec = x_countargv(command_to_exec);
	unsigned count_using = (unsigned) (term->misc.use_encoding ? 2 : 0);

	command_to_exec_with_luit = TypeCallocN(char *,
						  (count_split
						   + count_exec
						   + count_using
						   + 8));
	if (command_to_exec_with_luit == NULL)
	    SysError(ERROR_LUMALLOC);

	x_appendargv(command_to_exec_with_luit, split_filter);
	if (count_using) {
	    char *encoding_opt[4];
	    encoding_opt[0] = x_strdup("-encoding");
	    encoding_opt[1] = term->misc.locale_str;
	    encoding_opt[2] = NULL;
	    x_appendargv(command_to_exec_with_luit, encoding_opt);
	}
	command_length_with_luit = x_countargv(command_to_exec_with_luit);
	if (count_exec) {
	    char *delimiter[2];
	    delimiter[0] = x_strdup("--");
	    delimiter[1] = NULL;
	    x_appendargv(command_to_exec_with_luit, delimiter);
	    if (complex_command(command_to_exec)) {
		static char shell_name[] = "sh";
		static char c_option[] = "-c";
		static char *fixup_shell[] =
		{
		    shell_name,
		    c_option,
		    NULL
		};
		x_appendargv(command_to_exec_with_luit, fixup_shell);
	    }
	    x_appendargv(command_to_exec_with_luit, command_to_exec);
	}
	TRACE_ARGV("luit command", command_to_exec_with_luit);
	xtermSetenv("XTERM_FILTER", *command_to_exec_with_luit);
    }
#endif

    if_DEBUG({
	/* Set up stderr properly.  Opening this log file cannot be
	   done securely by a privileged xterm process (although we try),
	   so the debug feature is disabled by default. */
	char dbglogfile[TIMESTAMP_LEN + 20];
	int i = -1;
	timestamp_filename(dbglogfile, "xterm.debug.log.");
	if (creat_as(save_ruid, save_rgid, False, dbglogfile, 0600) > 0) {
	    i = open(dbglogfile, O_WRONLY | O_TRUNC);
	}
	if (i >= 0) {
	    dup2(i, 2);

	    /* mark this file as close on exec */
	    (void) fcntl(i, F_SETFD, 1);
	}
    });

    spawnXTerm(term, line_speed);

#ifdef OPT_LUA_SCRIPTING
    /* Initialize Lua scripting after terminal is set up */
    if (!lua_xterm_init()) {
        fprintf(stderr, "Warning: Failed to initialize Lua scripting\n");
    } else {
        lua_xterm_call_hook(LUA_HOOK_STARTUP);
    }
#endif

    /* Child process is out there, let's catch its termination */

#ifdef USE_POSIX_SIGNALS
    (void) posix_signal(SIGCHLD, reapchild);
#else
    (void) signal(SIGCHLD, reapchild);
#endif
    /* Realize procs have now been executed */

    if (am_slave >= 0) {	/* Write window id so master end can read and use */
	char buf[80];

	buf[0] = '\0';
	sprintf(buf, "%lx\n", XtWindow(SHELL_OF(CURRENT_EMU())));
	IGNORE_RC(write(screen->respond, buf, strlen(buf)));
    }
#ifdef AIXV3
#if (OSMAJORVERSION < 4)
    /* In AIXV3, xterms started from /dev/console have CLOCAL set.
     * This means we need to clear CLOCAL so that SIGHUP gets sent
     * to the slave-pty process when xterm exits.
     */

    {
	TERMIO_STRUCT tio;

	if (ttyGetAttr(screen->respond, &tio) == -1)
	    SysError(ERROR_TIOCGETP);

	tio.c_cflag &= ~(CLOCAL);

	if (ttySetAttr(screen->respond, &tio) == -1)
	    SysError(ERROR_TIOCSETP);
    }
#endif
#endif
#if defined(USE_ANY_SYSV_TERMIO) || defined(__minix)
    if (0 > (mode = fcntl(screen->respond, F_GETFL, 0)))
	SysError(ERROR_F_GETFL);
#ifdef O_NDELAY
    mode |= O_NDELAY;
#else
    mode |= O_NONBLOCK;
#endif /* O_NDELAY */
    if (fcntl(screen->respond, F_SETFL, mode))
	SysError(ERROR_F_SETFL);
#else /* !USE_ANY_SYSV_TERMIO */
    mode = 1;
    if (ioctl(screen->respond, FIONBIO, (char *) &mode) == -1)
	SysError(ERROR_FIONBIO);
#endif /* USE_ANY_SYSV_TERMIO, etc */

    /* The erase character is used to delete the current completion */
#if OPT_DABBREV
#ifdef TERMIO_STRUCT
    screen->dabbrev_erase_char = d_tio.c_cc[VERASE];
#else
    screen->dabbrev_erase_char = d_sg.sg_erase;
#endif
    TRACE(("set dabbrev erase_char %#x\n", screen->dabbrev_erase_char));
#endif

    FD_ZERO(&pty_mask);
    FD_ZERO(&X_mask);
    FD_ZERO(&Select_mask);
    FD_SET(screen->respond, &pty_mask);
    FD_SET(ConnectionNumber(screen->display), &X_mask);
    FD_SET(screen->respond, &Select_mask);
    FD_SET(ConnectionNumber(screen->display), &Select_mask);
    max_plus1 = ((screen->respond < ConnectionNumber(screen->display))
		 ? (1 + ConnectionNumber(screen->display))
		 : (1 + screen->respond));

    if_DEBUG({
	TRACE(("debugging on pid %d\n", (int) getpid()));
    });
    XSetErrorHandler(xerror);
    XSetIOErrorHandler(xioerror);
#if OPT_SESSION_MGT
    IceSetIOErrorHandler(ice_error);
#endif

    initPtyData(&VTbuffer);
#ifdef ALLOWLOGGING
    if (term->misc.log_on) {
	StartLog(term);
    }
#endif

    xtermEmbedWindow(winToEmbedInto);

    TRACE(("checking reverseVideo before rv %s fg %s, bg %s\n",
	   term->misc.re_verse0 ? "reverse" : "normal",
	   NonNull(TScreenOf(term)->Tcolors[TEXT_FG].resource),
	   NonNull(TScreenOf(term)->Tcolors[TEXT_BG].resource)));

    if (term->misc.re_verse0) {
	if (isDefaultForeground(TScreenOf(term)->Tcolors[TEXT_FG].resource)
	    && isDefaultBackground(TScreenOf(term)->Tcolors[TEXT_BG].resource)) {
	    TScreenOf(term)->Tcolors[TEXT_FG].resource = x_strdup(XtDefaultBackground);
	    TScreenOf(term)->Tcolors[TEXT_BG].resource = x_strdup(XtDefaultForeground);
	} else {
	    ReverseVideo(term);
	}
	term->misc.re_verse = True;
	update_reversevideo();
	TRACE(("updated  reverseVideo after  rv %s fg %s, bg %s\n",
	       term->misc.re_verse ? "reverse" : "normal",
	       NonNull(TScreenOf(term)->Tcolors[TEXT_FG].resource),
	       NonNull(TScreenOf(term)->Tcolors[TEXT_BG].resource)));
    }
#if OPT_MAXIMIZE
    if (resource.maximized)
	RequestMaximize(term, True);
#endif
    for (;;) {
#if OPT_TEK4014
	if (TEK4014_ACTIVE(term))
	    TekRun();
	else
#endif
	    VTRun(term);
    }
}

#if defined(__osf__) || (defined(__linux__) && !defined(USE_USG_PTYS)) || defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
#define USE_OPENPTY 1
static int opened_tty = -1;
#endif

/*
 * This function opens up a pty master and stuffs its value into pty.
 *
 * If it finds one, it returns a value of 0.  If it does not find one,
 * it returns a value of !0.  This routine is designed to be re-entrant,
 * so that if a pty master is found and later, we find that the slave
 * has problems, we can re-enter this function and get another one.
 */
static int
get_pty(int *pty, char *from GCC_UNUSED)
{
    int result = 1;

#if defined(USE_OPENPTY)
    result = openpty(pty, &opened_tty, ttydev, NULL, NULL);
    if (opened_tty >= 0) {
	close(opened_tty);
	opened_tty = -1;
    }
#elif defined(HAVE_POSIX_OPENPT) && defined(HAVE_PTSNAME) && defined(HAVE_GRANTPT_PTY_ISATTY)
    if ((*pty = posix_openpt(O_RDWR)) >= 0) {
	char *name = ptsname(*pty);
	if (name != NULL) {
	    strcpy(ttydev, name);
	    result = 0;
	}
    }
#ifdef USE_PTY_SEARCH
    if (result) {
	result = pty_search(pty);
    }
#endif
#elif defined(PUCC_PTYD)
    result = ((*pty = openrpty(ttydev, ptydev,
			       (resource.utmpInhibit ? OPTY_NOP : OPTY_LOGIN),
			       save_ruid, from)) < 0);
#elif defined(__QNXNTO__)
    result = pty_search(pty);
#else
#if defined(USE_USG_PTYS) || defined(__CYGWIN__)
    result = ((*pty = open("/dev/ptmx", O_RDWR)) < 0);
#if defined(SVR4) || defined(__SCO__)
    if (!result)
	strcpy(ttydev, ptsname(*pty));
#endif

#elif defined(AIXV3)

    if ((*pty = open("/dev/ptc", O_RDWR)) >= 0) {
	strcpy(ttydev, ttyname(*pty));
	result = 0;
    }
#elif defined(__convex__)

    char *pty_name;
    extern char *getpty(void);

    while ((pty_name = getpty()) != NULL) {
	if ((*pty = open(pty_name, O_RDWR)) >= 0) {
	    strcpy(ptydev, pty_name);
	    strcpy(ttydev, pty_name);
	    *x_basename(ttydev) = 't';
	    result = 0;
	    break;
	}
    }

#elif defined(sequent)

    result = ((*pty = getpseudotty(&ttydev, &ptydev)) < 0);

#elif defined(__sgi) && (OSMAJORVERSION >= 4)

    char *tty_name;

    tty_name = _getpty(pty, O_RDWR, 0622, 0);
    if (tty_name != 0) {
	strcpy(ttydev, tty_name);
	result = 0;
    }
#elif (defined(__sgi) && (OSMAJORVERSION < 4)) || (defined(umips) && defined (SYSTYPE_SYSV))

    struct stat fstat_buf;

    *pty = open("/dev/ptc", O_RDWR);
    if (*pty >= 0 && (fstat(*pty, &fstat_buf)) >= 0) {
	result = 0;
	sprintf(ttydev, "/dev/ttyq%d", minor(fstat_buf.st_rdev));
    }
#elif defined(__hpux)

    /*
     * Use the clone device if it works, otherwise use pty_search logic.
     */
    if ((*pty = open("/dev/ptym/clone", O_RDWR)) >= 0) {
	char *name = ptsname(*pty);
	if (name != 0) {
	    strcpy(ttydev, name);
	    result = 0;
	} else {		/* permissions, or other unexpected problem */
	    close(*pty);
	    *pty = -1;
	    result = pty_search(pty);
	}
    } else {
	result = pty_search(pty);
    }

#else

    result = pty_search(pty);

#endif
#endif

    TRACE(("get_pty(ttydev=%s, ptydev=%s) %s fd=%d\n",
	   ttydev != NULL ? ttydev : "?",
	   ptydev != NULL ? ptydev : "?",
	   result ? "FAIL" : "OK",
	   pty != NULL ? *pty : -1));
    return result;
}

static void
set_pty_permissions(uid_t uid, unsigned gid, unsigned mode)
{
#ifdef USE_TTY_GROUP
    struct group *ttygrp;

    if ((ttygrp = getgrnam(TTY_GROUP_NAME)) != NULL) {
	gid = (unsigned) ttygrp->gr_gid;
	mode &= 0660U;
    }
    endgrent();
#endif /* USE_TTY_GROUP */

    TRACE_IDS;
    set_owner(ttydev, (unsigned) uid, gid, mode);
}

#ifdef get_pty			/* USE_UTMP_SETGID */
#undef get_pty
/*
 * Call the real get_pty() before relinquishing root-setuid, caching the
 * result.
 */
static int
get_pty(int *pty, char *from)
{
    static int m_pty = -1;
    int result = -1;

    if (pty == NULL) {
	result = really_get_pty(&m_pty, from);

	seteuid(0);
	set_pty_permissions(save_ruid, save_rgid, 0600U);
	seteuid(save_ruid);
	TRACE_IDS;

    } else if (m_pty != -1) {
	*pty = m_pty;
	result = 0;
    } else {
	result = -1;
    }
    TRACE(("get_pty(ttydev=%s, ptydev=%s) %s fd=%d (utmp setgid)\n",
	   ttydev != 0 ? ttydev : "?",
	   ptydev != 0 ? ptydev : "?",
	   result ? "FAIL" : "OK",
	   pty != 0 ? *pty : -1));
#ifdef USE_OPENPTY
    if (opened_tty >= 0) {
	close(opened_tty);
	opened_tty = -1;
    }
#endif
    return result;
}
#endif

/*
 * Called from get_pty to iterate over likely pseudo terminals
 * we might allocate.  Used on those systems that do not have
 * a functional interface for allocating a pty.
 * Returns 0 if found a pty, 1 if fails.
 */
#ifdef USE_PTY_SEARCH
static int
pty_search(int *pty)
{
    static int devindex = 0, letter = 0;

#if defined(CRAY)
    while (devindex < MAXPTTYS) {
	sprintf(ttydev, TTYFORMAT, devindex);
	sprintf(ptydev, PTYFORMAT, devindex);
	devindex++;

	TRACE(("pty_search(ttydev=%s, ptydev=%s)\n", ttydev, ptydev));
	if ((*pty = open(ptydev, O_RDWR)) >= 0) {
	    return 0;
	}
    }
#else /* CRAY */
    while (PTYCHAR1[letter]) {
	ttydev[strlen(ttydev) - 2] =
	    ptydev[strlen(ptydev) - 2] = PTYCHAR1[letter];

	while (PTYCHAR2[devindex]) {
	    ttydev[strlen(ttydev) - 1] =
		ptydev[strlen(ptydev) - 1] = PTYCHAR2[devindex];
	    devindex++;

	    TRACE(("pty_search(ttydev=%s, ptydev=%s)\n", ttydev, ptydev));
	    if ((*pty = open(ptydev, O_RDWR)) >= 0) {
#ifdef sun
		/* Need to check the process group of the pty.
		 * If it exists, then the slave pty is in use,
		 * and we need to get another one.
		 */
		int pgrp_rtn;
		if (ioctl(*pty, TIOCGPGRP, &pgrp_rtn) == 0 || errno != EIO) {
		    close(*pty);
		    continue;
		}
#endif /* sun */
		return 0;
	    }
	}
	devindex = 0;
	letter++;
    }
#endif /* CRAY else */
    /*
     * We were unable to allocate a pty master!  Return an error
     * condition and let our caller terminate cleanly.
     */
    return 1;
}
#endif /* USE_PTY_SEARCH */

/*
 * The only difference in /etc/termcap between 4014 and 4015 is that
 * the latter has support for switching character sets.  We support the
 * 4015 protocol, but ignore the character switches.  Therefore, we
 * choose 4014 over 4015.
 *
 * Features of the 4014 over the 4012: larger (19") screen, 12-bit
 * graphics addressing (compatible with 4012 10-bit addressing),
 * special point plot mode, incremental plot mode (not implemented in
 * later Tektronix terminals), and 4 character sizes.
 * All of these are supported by xterm.
 */

#if OPT_TEK4014
static const char *const tekterm[] =
{
    "tek4014",
    "tek4015",			/* 4014 with APL character set support */
    "tek4012",			/* 4010 with lower case */
    "tek4013",			/* 4012 with APL character set support */
    "tek4010",			/* small screen, upper-case only */
    "dumb",
    NULL
};
#endif

/* The VT102 is a VT100 with the Advanced Video Option included standard.
 * It also adds Escape sequences for insert/delete character/line.
 * The VT220 adds 8-bit character sets, selective erase.
 * The VT320 adds a 25th status line, terminal state interrogation.
 * The VT420 has up to 48 lines on the screen.
 */

static const char *const vtterm[] =
{
#ifdef USE_X11TERM
    "x11term",			/* for people who want special term name */
#endif
    DFT_TERMTYPE,		/* for people who want special term name */
    "xterm",			/* the preferred name, should be fastest */
    "vt102",
    "vt100",
    "ansi",
    "dumb",
    NULL
};

/* ARGSUSED */
static void
hungtty(int i GCC_UNUSED)
{
    DEBUG_MSG("handle:hungtty\n");
    siglongjmp(env, 1);
}

#if OPT_PTY_HANDSHAKE
#define NO_FDS {-1, -1}

static int cp_pipe[2] = NO_FDS;	/* this pipe is used for child to parent transfer */
static int pc_pipe[2] = NO_FDS;	/* this pipe is used for parent to child transfer */

typedef enum {			/* c == child, p == parent                        */
    PTY_BAD,			/* c->p: can't open pty slave for some reason     */
    PTY_FATALERROR,		/* c->p: we had a fatal error with the pty        */
    PTY_GOOD,			/* c->p: we have a good pty, let's go on          */
    PTY_NEW,			/* p->c: here is a new pty slave, try this        */
    PTY_NOMORE,			/* p->c; no more pty's, terminate                 */
    UTMP_ADDED,			/* c->p: utmp entry has been added                */
    UTMP_TTYSLOT,		/* c->p: here is my ttyslot                       */
    PTY_EXEC			/* p->c: window has been mapped the first time    */
} status_t;

#define HANDSHAKE_LEN	1024

typedef struct {
    status_t status;
    int error;
    int fatal_error;
    int tty_slot;
    int rows;
    int cols;
    char buffer[HANDSHAKE_LEN];
} handshake_t;

/* the buffer is large enough that we can always have a trailing null */
#define copy_handshake(dst, src) \
	strncpy(dst.buffer, src, (size_t)HANDSHAKE_LEN - 1)[HANDSHAKE_LEN - 1] = '\0'

#if OPT_TRACE
static void
trace_handshake(const char *tag, handshake_t * data)
{
    const char *status = "?";
    switch (data->status) {
    case PTY_BAD:
	status = "PTY_BAD";
	break;
    case PTY_FATALERROR:
	status = "PTY_FATALERROR";
	break;
    case PTY_GOOD:
	status = "PTY_GOOD";
	break;
    case PTY_NEW:
	status = "PTY_NEW";
	break;
    case PTY_NOMORE:
	status = "PTY_NOMORE";
	break;
    case UTMP_ADDED:
	status = "UTMP_ADDED";
	break;
    case UTMP_TTYSLOT:
	status = "UTMP_TTYSLOT";
	break;
    case PTY_EXEC:
	status = "PTY_EXEC";
	break;
    }
    TRACE(("handshake %s %s errno=%d, error=%d device \"%s\"\n",
	   tag,
	   status,
	   data->error,
	   data->fatal_error,
	   data->buffer));
}
#define TRACE_HANDSHAKE(tag, data) trace_handshake(tag, data)
#else
#define TRACE_HANDSHAKE(tag, data)	/* nothing */
#endif

/* HsSysError()
 *
 * This routine does the equivalent of a SysError but it handshakes
 * over the errno and error exit to the master process so that it can
 * display our error message and exit with our exit code so that the
 * user can see it.
 */

static void
HsSysError(int error)
{
    handshake_t handshake;

    memset(&handshake, 0, sizeof(handshake));
    handshake.status = PTY_FATALERROR;
    handshake.error = errno;
    handshake.fatal_error = error;
    copy_handshake(handshake, ttydev);

    if (resource.ptyHandshake && (cp_pipe[1] >= 0)) {
	TRACE(("HsSysError errno=%d, error=%d device \"%s\"\n",
	       handshake.error,
	       handshake.fatal_error,
	       handshake.buffer));
	TRACE_HANDSHAKE("writing", &handshake);
	IGNORE_RC(write(cp_pipe[1],
			(const char *) &handshake,
			sizeof(handshake)));
    } else {
	xtermWarning("fatal pty error errno=%d, error=%d device \"%s\"\n",
		     handshake.error,
		     handshake.fatal_error,
		     handshake.buffer);
	fprintf(stderr, "%s\n", SysErrorMsg(handshake.error));
	fprintf(stderr, "Reason: %s\n", SysReasonMsg(handshake.fatal_error));
    }
    exit(error);
}

void
first_map_occurred(void)
{
    if (resource.wait_for_map) {
	if (pc_pipe[1] >= 0) {
	    handshake_t handshake;
	    TScreen *screen = TScreenOf(term);

	    memset(&handshake, 0, sizeof(handshake));
	    handshake.status = PTY_EXEC;
	    handshake.rows = screen->max_row;
	    handshake.cols = screen->max_col;

	    TRACE(("first_map_occurred: %dx%d\n", MaxRows(screen), MaxCols(screen)));
	    TRACE_HANDSHAKE("writing", &handshake);
	    IGNORE_RC(write(pc_pipe[1],
			    (const char *) &handshake,
			    sizeof(handshake)));
	    close(cp_pipe[0]);
	    close(pc_pipe[1]);
	}
	resource.wait_for_map = False;
    }
}
#else
/*
 * temporary hack to get xterm working on att ptys
 */
static void
HsSysError(int error)
{
    xtermWarning("fatal pty error %d (errno=%d) on tty %s\n",
		 error, errno, ttydev);
    exit(error);
}
#endif /* OPT_PTY_HANDSHAKE else !OPT_PTY_HANDSHAKE */

static void
set_owner(char *device, unsigned uid, unsigned gid, unsigned mode)
{
    int why;

    TRACE_IDS;
    TRACE(("set_owner(%s, uid=%d, gid=%d, mode=%#o\n",
	   device, (int) uid, (int) gid, (unsigned) mode));

    if (chown(device, (uid_t) uid, (gid_t) gid) < 0) {
	why = errno;
	if (why != ENOENT
	    && save_ruid == 0) {
	    xtermPerror("Cannot chown %s to %ld,%ld",
			device, (long) uid, (long) gid);
	}
	TRACE(("...chown failed: %s\n", strerror(why)));
    } else if (chmod(device, (mode_t) mode) < 0) {
	why = errno;
	if (why != ENOENT) {
	    struct stat sb;
	    if (stat(device, &sb) < 0) {
		xtermPerror("Cannot chmod %s to %03o",
			    device, (unsigned) mode);
	    } else if (mode != (sb.st_mode & 0777U)) {
		xtermPerror("Cannot chmod %s to %03lo currently %03lo",
			    device,
			    (unsigned long) mode,
			    (unsigned long) (sb.st_mode & 0777U));
		TRACE(("...stat uid=%d, gid=%d, mode=%#o\n",
		       (int) sb.st_uid, (int) sb.st_gid, (unsigned) sb.st_mode));
	    }
	}
	TRACE(("...chmod failed: %s\n", strerror(why)));
    }
}

/*
 * utmp data may not be null-terminated; even if it is, there may be garbage
 * after the null.  This fills the unused part of the result with nulls.
 */
static void
copy_filled(char *target, const char *source, size_t len)
{
    size_t used = 0;
    while (used < len) {
	if ((target[used] = source[used]) == 0)
	    break;
	++used;
    }
    while (used < len) {
	target[used++] = '\0';
    }
}

#if defined(HAVE_UTMP) && defined(USE_SYSV_UTMP) && !defined(USE_UTEMPTER)
/*
 * getutid() only looks at ut_type and ut_id.
 * But we'll also check ut_line in find_utmp().
 */
static void
init_utmp(int type, struct UTMP_STR *tofind)
{
    memset(tofind, 0, sizeof(*tofind));
    tofind->ut_type = (short) type;
    copy_filled(tofind->ut_id, my_utmp_id(ttydev), sizeof(tofind->ut_id));
    copy_filled(tofind->ut_line, my_pty_name(ttydev), sizeof(tofind->ut_line));
}

/*
 * We could use getutline() if we didn't support old systems.
 */
static struct UTMP_STR *
find_utmp(struct UTMP_STR *tofind)
{
    struct UTMP_STR *result;
    struct UTMP_STR limited;
    struct UTMP_STR working;

    for (;;) {
	memset(&working, 0, sizeof(working));
	working.ut_type = tofind->ut_type;
	copy_filled(working.ut_id, tofind->ut_id, sizeof(tofind->ut_id));
#if defined(__digital__) && defined(__unix__) && (defined(OSMAJORVERSION) && OSMAJORVERSION < 5)
	working.ut_type = 0;
#endif
	if ((result = call_getutid(&working)) == NULL)
	    break;
	copy_filled(limited.ut_line, result->ut_line, sizeof(result->ut_line));
	if (!memcmp(limited.ut_line, tofind->ut_line, sizeof(limited.ut_line)))
	    break;
	/*
	 * Solaris, IRIX64 and HPUX manpages say to fill the static area
	 * pointed to by the return-value to zeros if searching for multiple
	 * occurrences.  Otherwise it will continue to return the same value.
	 */
	memset(result, 0, sizeof(*result));
    }
    return result;
}
#endif /* HAVE_UTMP... */

#define close_fd(fd) close(fd), fd = -1

#if defined(TIOCNOTTY) && (!defined(__GLIBC__) || (__GLIBC__ < 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ < 1)))
#define USE_NO_DEV_TTY 1
#else
#define USE_NO_DEV_TTY 0
#endif

static int
same_leaf(char *a, char *b)
{
    char *p = x_basename(a);
    char *q = x_basename(b);
    return !strcmp(p, q);
}

/*
 * "good enough" (inode wouldn't port to Cygwin)
 */
static int
same_file(const char *a, const char *b)
{
    struct stat asb;
    struct stat bsb;
    int result = 0;

    if ((stat(a, &asb) == 0)
	&& (stat(b, &bsb) == 0)
	&& ((asb.st_mode & S_IFMT) == S_IFREG)
	&& ((bsb.st_mode & S_IFMT) == S_IFREG)
	&& (asb.st_mtime == bsb.st_mtime)
	&& (asb.st_size == bsb.st_size)) {
	result = 1;
    }
    return result;
}

static int
findValidShell(const char *haystack, const char *needle)
{
    int result = -1;
    int count = -1;
    const char *s, *t;
    size_t want = strlen(needle);

    TRACE(("findValidShell:\n%s\n", NonNull(haystack)));

    for (s = haystack; (s != NULL) && (*s != '\0'); s = t) {
	size_t have;

	++count;
	if ((t = strchr(s, '\n')) == NULL) {
	    t = s + strlen(s);
	}
	have = (size_t) (t - s);

	if ((have >= want) && (*s != '#')) {
	    char *p = (char *) malloc(have + 1);

	    if (p != NULL) {
		char *q;

		memcpy(p, s, have);
		p[have] = '\0';
		if ((q = x_strtrim(p)) != NULL) {
		    TRACE(("...test %s\n", q));
		    if (!strcmp(q, needle)) {
			result = count;
		    } else if (same_leaf(q, (char *) needle) &&
			       same_file(q, needle)) {
			result = count;
		    }
		    free(q);
		}
		free(p);
	    }
	    if (result >= 0)
		break;
	}
	while (*t == '\n') {
	    ++t;
	}
    }
    return result;
}

static int
ourValidShell(const char *pathname)
{
    char *trimmed = x_strtrim(resource.valid_shells);
    int result = findValidShell(trimmed, pathname);
    free(trimmed);
    return result;
}

#if defined(HAVE_GETUSERSHELL) && defined(HAVE_ENDUSERSHELL)
static Boolean
validShell(const char *pathname)
{
    int result = -1;

    if (validProgram(pathname)) {
	char *q;
	int count = -1;

	TRACE(("validShell:getusershell\n"));
	while ((q = getusershell()) != NULL) {
	    ++count;
	    TRACE(("...test \"%s\"\n", q));
	    if (!strcmp(q, pathname)) {
		result = count;
		break;
	    }
	}
	endusershell();

	if (result < 0)
	    result = ourValidShell(pathname);
    }

    TRACE(("validShell %s ->%d\n", NonNull(pathname), result));
    return (result >= 0);
}
#else
/*
 * Only set $SHELL for paths found in the standard location.
 */
static Boolean
validShell(const char *pathname)
{
    int result = -1;
    const char *ok_shells = "/etc/shells";
    char *blob;
    struct stat sb;
    size_t rc;
    FILE *fp;

    if (validProgram(pathname)) {

	TRACE(("validShell:%s\n", ok_shells));

	if (stat(ok_shells, &sb) == 0
	    && (sb.st_mode & S_IFMT) == S_IFREG
	    && ((size_t) sb.st_size > 0)
	    && ((size_t) sb.st_size < (((size_t) ~0) - 2))
	    && (blob = calloc((size_t) sb.st_size + 2, sizeof(char))) != 0) {

	    if ((fp = fopen(ok_shells, "r")) != 0) {
		rc = fread(blob, sizeof(char), (size_t) sb.st_size, fp);
		fclose(fp);

		if (rc == (size_t) sb.st_size) {
		    blob[rc] = '\0';
		    result = findValidShell(blob, pathname);
		}
	    }
	    free(blob);
	}
	if (result < 0)
	    result = ourValidShell(pathname);
    }
    TRACE(("validShell %s ->%d\n", NonNull(pathname), result));
    return (result > 0);
}
#endif

static char *
resetShell(char *oldPath)
{
    char *newPath = x_strdup("/bin/sh");
    char *envPath = getenv("SHELL");
    free(oldPath);
    if (!IsEmpty(envPath))
	xtermSetenv("SHELL", newPath);
    return newPath;
}

/*
 * Malware tampers with the terminal database.  Let's discourage that.
 */
static void
xtermTrimEnvPath(const char *name)
{
    if (getuid() == 0) {
	xtermUnsetenv(name);
    } else {
	char *value = getenv(name);
	if (value != NULL) {
	    char *copyPath = x_strdup(value);
	    char *editPath = x_strdup(value);
	    if (copyPath != NULL && editPath != NULL) {
		char *source = copyPath;
		char *item;
		Bool concat = False;
		*editPath = '\0';
		while ((item = strtok(source, ":")) != NULL) {
		    if (access(item, W_OK) == 0) {
			TRACE(("found writable item in %s: %s\n", name, item));
		    } else {
			if (concat)
			    strcat(editPath, ":");
			strcat(editPath, item);
			concat = True;
		    }
		    source = NULL;
		}
		if (strcmp(editPath, copyPath)) {
		    if (*editPath)
			xtermSetenv(name, editPath);
		    else
			xtermUnsetenv(name);
		} else {
		    free(editPath);
		}
		free(copyPath);
	    }
	}
    }
}

/*
 * Trim unwanted environment variables:
 *
 * DESKTOP_STARTUP_ID
 *	standards.freedesktop.org/startup-notification-spec/
 * notes that this variable is used when a "reliable" mechanism is
 * not available; in practice it must be unset to avoid confusing
 * GTK applications.
 *
 * XCURSOR_PATH
 * We set this temporarily to work around poor design of Xcursor.  Unset it
 * here to avoid confusion.
 *
 * Other...
 * These are set by other terminal emulators or non-standard libraries, and are
 * a nuisance if one starts xterm from a shell inside one of those.
 */
static void
xtermTrimEnv(void)
{
#define KEEP(wild,name) { 0, wild, #name }
#define TRIM(wild,name) { 1, wild, #name }
    /* *INDENT-OFF* */
    static const struct {
	int trim;
	int wild;
	const char *name;
    } table[] = {
	TRIM(0, COLUMNS),
	TRIM(0, DEFAULT_COLORS),
	TRIM(0, DESKTOP_STARTUP_ID),
	TRIM(0, LINES),
	TRIM(0, SHLVL),		/* ksh, bash */
	TRIM(0, STY),		/* screen */
	TRIM(0, TERMCAP),
	TRIM(0, TMUX),
	TRIM(0, TMUX_PANE),
	TRIM(0, WCWIDTH_CJK_LEGACY),
	TRIM(0, WINDOW),	/* screen */
	TRIM(0, XCURSOR_PATH),
	KEEP(0, MC_XDG_OPEN),
	KEEP(0, TERM_INGRES),
	TRIM(1, ALACRITTY_),
	TRIM(1, COLORFGBG),
	TRIM(1, COLORTERM),
	TRIM(1, GIO_LAUNCHED_),
	TRIM(1, GHOSTTY_),
	TRIM(1, GTK_),
	TRIM(1, ITERM_),
	TRIM(1, ITERM2_),
	TRIM(1, KITTY_),
	TRIM(1, KONSOLE_),
	TRIM(1, LC_TERMINAL),
	TRIM(1, MC_),
	TRIM(1, MINTTY_),
	TRIM(1, MOSH_),
	TRIM(1, PUTTY),
	TRIM(1, RXVT_),
	TRIM(1, TERM_),		/* e.g., TERM_PROGRAM */
	TRIM(1, TERMINAL_),	/* e.g., TERMINAL_NAME */
	TRIM(1, URXVT_),
	TRIM(1, VTE_),
	TRIM(1, WT_SESSION),
	TRIM(1, WT_PROFILE),
	TRIM(1, XTERM_),
	TRIM(1, ZUTTY_),
    };
#undef TRIM
    /* *INDENT-ON* */
    Cardinal j, k;
    int trimmed = 0;

    for (j = 0; environ[j] != NULL; ++j) {
	char *equals = strchr(environ[j], '=');
	size_t dstlen = strlen(environ[j]);

	if (equals != NULL)
	    dstlen = (size_t) (equals - environ[j]);

	for (k = 0; k < XtNumber(table); ++k) {
	    size_t srclen = strlen(table[k].name);
	    if (table[k].wild) {
		if (dstlen >= srclen &&
		    !strncmp(environ[j], table[k].name, srclen)) {
		    char *my_var;
		    if (table[k].trim &&
			(my_var = x_strdup(environ[j])) != NULL) {
			my_var[dstlen] = '\0';
			trimmed++;
			xtermUnsetenv(my_var);
			free(my_var);
			/* When removing an entry, check the same slot again. */
			if (j != 0)
			    j--;
		    }
		    break;
		}
	    } else if (dstlen == srclen &&
		       !strncmp(environ[j], table[k].name, srclen)) {
		if (table[k].trim) {
		    trimmed++;
		    xtermUnsetenv(table[k].name);
		    /* When removing an entry, check the same slot again. */
		    if (j != 0)
			j--;
		}
		break;
	    }
	}
    }
    if (trimmed) {
	TRACE(("%d environment variables trimmed\n", trimmed));
	xtermTrimEnvPath("TERMINFO");
	xtermTrimEnvPath("TERMINFO_DIRS");
	xtermTrimEnvPath("TERMPATH");
    }
}

/*
 *  Inits pty and tty and forks a login process.
 *  Does not close fd Xsocket.
 *  If slave, the pty named in passedPty is already open for use
 */
static int
spawnXTerm(XtermWidget xw, unsigned line_speed)
{
    TScreen *screen = TScreenOf(xw);
#if OPT_PTY_HANDSHAKE
    Bool got_handshake_size = False;
    handshake_t handshake;
    int done;
#endif
#if OPT_INITIAL_ERASE
    int initial_erase = XTERM_ERASE;
    Bool setInitialErase;
#endif
    int rc = 0;
    int ttyfd = -1;
    Bool ok_termcap;
    char *newtc;

#ifdef TERMIO_STRUCT
    Cardinal nn;
    TERMIO_STRUCT tio;
#ifdef TIOCLSET
    unsigned lmode;
#endif /* TIOCLSET */
#ifdef HAS_LTCHARS
    struct ltchars ltc;
#endif /* HAS_LTCHARS */
#else /* !TERMIO_STRUCT */
    int ldisc = 0;
    int discipline;
    unsigned lmode;
    struct tchars tc;
    struct ltchars ltc;
    struct sgttyb sg;
#ifdef sony
    int jmode;
    struct jtchars jtc;
#endif /* sony */
#endif /* TERMIO_STRUCT */

    char *shell_path = NULL;
    char *shname, *shname_minus;
    int i;
#if USE_NO_DEV_TTY
    int no_dev_tty = False;
#endif
    const char *const *envnew;	/* new environment */
    char buf[64];
    char *TermName = NULL;
#ifdef TTYSIZE_STRUCT
    TTYSIZE_STRUCT ts;
#endif
    struct passwd pw;
    char *login_name = NULL;
#ifndef USE_UTEMPTER
#ifdef HAVE_UTMP
    struct UTMP_STR utmp;
#ifdef USE_SYSV_UTMP
    struct UTMP_STR *utret = NULL;
#endif
#ifdef USE_LASTLOG
    struct lastlog lastlog;
#endif
#ifdef USE_LASTLOGX
    struct lastlogx lastlogx;
#endif /* USE_LASTLOG */
#endif /* HAVE_UTMP */
#endif /* !USE_UTEMPTER */

#if OPT_TRACE
    unsigned long xterm_parent = (unsigned long) getpid();
#endif

    /* Noisy compilers (suppress some unused-variable warnings) */
    (void) rc;
#if defined(HAVE_UTMP) && defined(USE_SYSV_UTMP) && !defined(USE_UTEMPTER)
    (void) utret;
#endif

    screen->uid = save_ruid;
    screen->gid = save_rgid;

#ifdef SIGTTOU
    /* so that TIOCSWINSZ || TIOCSIZE doesn't block */
    signal(SIGTTOU, SIG_IGN);
#endif

#if OPT_PTY_HANDSHAKE
    memset(&handshake, 0, sizeof(handshake));
#endif

    if (am_slave >= 0) {
	screen->respond = am_slave;
	set_pty_id(ttydev, passedPty);
#ifdef USE_PTY_DEVICE
	set_pty_id(ptydev, passedPty);
#endif
	if (xtermResetIds(screen) < 0)
	    exit(ERROR_MISC);
    } else {
	Bool tty_got_hung;

	/*
	 * Sometimes /dev/tty hangs on open (as in the case of a pty
	 * that has gone away).  Simply make up some reasonable
	 * defaults.
	 */

	if (!sigsetjmp(env, 1)) {
	    signal(SIGALRM, hungtty);
	    alarm(2);		/* alarm(1) might return too soon */
	    ttyfd = open("/dev/tty", O_RDWR);
	    alarm(0);
	    tty_got_hung = False;
	} else {
	    tty_got_hung = True;
	    ttyfd = -1;
	    errno = ENXIO;
	}
	shell_path = NULL;
	memset(&pw, 0, sizeof(pw));
#if OPT_PTY_HANDSHAKE
	got_handshake_size = False;
#endif /* OPT_PTY_HANDSHAKE */
#if OPT_INITIAL_ERASE
	initial_erase = XTERM_ERASE;
#endif
	signal(SIGALRM, SIG_DFL);

	/*
	 * Check results and ignore current control terminal if
	 * necessary.  ENXIO is what is normally returned if there is
	 * no controlling terminal, but some systems (e.g. SunOS 4.0)
	 * seem to return EIO.  Solaris 2.3 is said to return EINVAL.
	 * Cygwin returns ENOENT.  FreeBSD can return ENOENT, especially
	 * if xterm is run within a jail.
	 */
#if USE_NO_DEV_TTY
	no_dev_tty = False;
#endif
	if (ttyfd < 0) {
	    if (tty_got_hung || errno == ENXIO || errno == EIO ||
		errno == ENOENT ||
#ifdef ENODEV
		errno == ENODEV ||
#endif
		errno == EINVAL || errno == ENOTTY || errno == EACCES) {
#if USE_NO_DEV_TTY
		no_dev_tty = True;
#endif
#ifdef HAS_LTCHARS
		ltc = d_ltc;
#endif /* HAS_LTCHARS */
#ifdef TIOCLSET
		lmode = d_lmode;
#endif /* TIOCLSET */
#ifdef TERMIO_STRUCT
		tio = d_tio;
#else /* !TERMIO_STRUCT */
		sg = d_sg;
		tc = d_tc;
		discipline = d_disipline;
#ifdef sony
		jmode = d_jmode;
		jtc = d_jtc;
#endif /* sony */
#endif /* TERMIO_STRUCT */
	    } else {
		SysError(ERROR_OPDEVTTY);
	    }
	} else {

	    /* Get a copy of the current terminal's state,
	     * if we can.  Some systems (e.g., SVR4 and MacII)
	     * may not have a controlling terminal at this point
	     * if started directly from xdm or xinit,
	     * in which case we just use the defaults as above.
	     */
#ifdef HAS_LTCHARS
	    if (ioctl(ttyfd, TIOCGLTC, &ltc) == -1)
		ltc = d_ltc;
#endif /* HAS_LTCHARS */
#ifdef TIOCLSET
	    if (ioctl(ttyfd, TIOCLGET, &lmode) == -1)
		lmode = d_lmode;
#endif /* TIOCLSET */
#ifdef TERMIO_STRUCT
	    rc = ttyGetAttr(ttyfd, &tio);
	    if (rc == -1)
		tio = d_tio;
#else /* !TERMIO_STRUCT */
	    rc = ioctl(ttyfd, TIOCGETP, (char *) &sg);
	    if (rc == -1)
		sg = d_sg;
	    if (ioctl(ttyfd, TIOCGETC, (char *) &tc) == -1)
		tc = d_tc;
	    if (ioctl(ttyfd, TIOCGETD, (char *) &discipline) == -1)
		discipline = d_disipline;
#ifdef sony
	    if (ioctl(ttyfd, TIOCKGET, (char *) &jmode) == -1)
		jmode = d_jmode;
	    if (ioctl(ttyfd, TIOCKGETC, (char *) &jtc) == -1)
		jtc = d_jtc;
#endif /* sony */
#endif /* TERMIO_STRUCT */

	    /*
	     * If ptyInitialErase is set, we want to get the pty's
	     * erase value.  Just in case that will fail, first get
	     * the value from /dev/tty, so we will have something
	     * at least.
	     */
#if OPT_INITIAL_ERASE
	    if (resource.ptyInitialErase) {
#ifdef TERMIO_STRUCT
		initial_erase = tio.c_cc[VERASE];
#else /* !TERMIO_STRUCT */
		initial_erase = sg.sg_erase;
#endif /* TERMIO_STRUCT */
		TRACE(("%s initial_erase:%d (from /dev/tty)\n",
		       rc == 0 ? "OK" : "FAIL",
		       initial_erase));
	    }
#endif
	    close_fd(ttyfd);
	}

	if (get_pty(&screen->respond, XDisplayString(screen->display))) {
	    SysError(ERROR_PTYS);
	}
	TRACE_GET_TTYSIZE(screen->respond, "after get_pty");
#if OPT_INITIAL_ERASE
	if (resource.ptyInitialErase) {
	    initial_erase = get_tty_erase(screen->respond,
					  initial_erase,
					  "pty");
	}
#endif /* OPT_INITIAL_ERASE */
    }

    /* avoid double MapWindow requests */
    XtSetMappedWhenManaged(SHELL_OF(CURRENT_EMU()), False);

    wm_delete_window = CachedInternAtom(XtDisplay(toplevel),
					"WM_DELETE_WINDOW");

    if (!TEK4014_ACTIVE(xw))
	VTInit(xw);		/* realize now so know window size for tty driver */
#if defined(TIOCCONS) || defined(SRIOCSREDIR)
    if (Console) {
	/*
	 * Inform any running xconsole program
	 * that we are going to steal the console.
	 */
	XmuGetHostname(mit_console_name + MIT_CONSOLE_LEN, 255);
	TRACE(("getting for console name property \"%s\"\n", mit_console_name));
	mit_console = CachedInternAtom(screen->display, mit_console_name);
	/* the user told us to be the console, so we can use CurrentTime */
	XtOwnSelection(SHELL_OF(CURRENT_EMU()),
		       mit_console, CurrentTime,
		       ConvertConsoleSelection, NULL, NULL);
    }
#endif
#if OPT_TEK4014
    if (TEK4014_ACTIVE(xw)) {
	envnew = tekterm;
    } else
#endif
    {
	envnew = vtterm;
    }

    /*
     * This used to exit if no termcap entry was found for the specified
     * terminal name.  That's a little unfriendly, so instead we'll allow
     * the program to proceed (but not to set $TERMCAP) if the termcap
     * entry is not found.
     */
    ok_termcap = True;
    if (!get_termcap(xw, TermName = resource.term_name)) {
	const char *last = NULL;
	char *next;

	TermName = x_strdup(*envnew);
	ok_termcap = False;
	while (*envnew != NULL) {
	    if (last == NULL || strcmp(last, *envnew)) {
		next = x_strdup(*envnew);
		if (get_termcap(xw, next)) {
		    free(TermName);
		    TermName = next;
		    ok_termcap = True + 1;
		    break;
		} else {
		    free(next);
		}
	    }
	    last = *envnew;
	    envnew++;
	}
    }
    if (ok_termcap) {
	resource.term_name = x_strdup(TermName);
	resize_termcap(xw);
    }

    /*
     * Check if ptyInitialErase is not set.  If so, we rely on the termcap
     * (or terminfo) to tell us what the erase mode should be set to.
     */
#if OPT_INITIAL_ERASE
    TRACE(("resource ptyInitialErase is %sset\n",
	   resource.ptyInitialErase ? "" : "not "));
    setInitialErase = False;
    if (override_tty_modes && ttyModes[XTTYMODE_erase].set) {
	initial_erase = ttyModes[XTTYMODE_erase].value;
	setInitialErase = True;
    } else if (resource.ptyInitialErase) {
	/* EMPTY */ ;
    } else if (ok_termcap) {
	char *s = get_tcap_erase(xw);
	TRACE(("...extracting initial_erase value from termcap\n"));
	if (s != NULL) {
	    char *save = s;
	    initial_erase = decode_keyvalue(&s, True);
	    setInitialErase = True;
	    free(save);
	}
    }
    TRACE(("...initial_erase:%d\n", initial_erase));

    TRACE(("resource backarrowKeyIsErase is %sset\n",
	   resource.backarrow_is_erase ? "" : "not "));
    if (resource.backarrow_is_erase) {	/* see input.c */
	if (initial_erase == ANSI_DEL) {
	    UIntClr(xw->keyboard.flags, MODE_DECBKM);
	} else {
	    xw->keyboard.flags |= MODE_DECBKM;
	    xw->keyboard.reset_DECBKM = 1;
	}
	TRACE(("...sets DECBKM %s\n",
	       (xw->keyboard.flags & MODE_DECBKM) ? "on" : "off"));
    } else {
	xw->keyboard.reset_DECBKM = 2;
    }
#endif /* OPT_INITIAL_ERASE */

#ifdef TTYSIZE_STRUCT
    /* tell tty how big window is */
#if OPT_TEK4014
    if (TEK4014_ACTIVE(xw)) {
	setup_winsize(ts, TDefaultRows, TDefaultCols,
		      TFullHeight(TekScreenOf(tekWidget)),
		      TFullWidth(TekScreenOf(tekWidget)));
    } else
#endif
    {
	setup_winsize(ts, MaxRows(screen), MaxCols(screen),
		      FullHeight(screen), FullWidth(screen));
    }
    TRACE_RC(i, SET_TTYSIZE(screen->respond, ts));
    TRACE(("spawn SET_TTYSIZE %dx%d return %d\n",
	   TTYSIZE_ROWS(ts),
	   TTYSIZE_COLS(ts), i));
#endif /* TTYSIZE_STRUCT */

#if !defined(USE_OPENPTY)
#if defined(USE_USG_PTYS) || defined(HAVE_POSIX_OPENPT)
    /*
     * utempter checks the ownership of the device; some implementations
     * set ownership in grantpt - do this first.
     */
    grantpt(screen->respond);
#endif
#if !defined(USE_USG_PTYS) && defined(HAVE_POSIX_OPENPT)
    unlockpt(screen->respond);
    TRACE_GET_TTYSIZE(screen->respond, "after unlockpt");
#endif
#endif /* !USE_OPENPTY */

    added_utmp_entry = False;
#if defined(USE_UTEMPTER)
#undef UTMP
    if ((xw->misc.login_shell || !command_to_exec) && !resource.utmpInhibit) {
	struct UTMP_STR dummy;

	/* Note: utempter may trim it anyway */
	SetUtmpHost(dummy.ut_host, screen);
	TRACE(("...calling addToUtmp(pty=%s, hostname=%s, master_fd=%d)\n",
	       ttydev, dummy.ut_host, screen->respond));
	UTEMPTER_ADD(ttydev, dummy.ut_host, screen->respond);
	added_utmp_entry = True;
    }
#endif

    if (am_slave < 0) {
#if OPT_PTY_HANDSHAKE
	if (resource.ptyHandshake && (pipe(pc_pipe) || pipe(cp_pipe)))
	    SysError(ERROR_FORK);
#endif
	TRACE(("Forking...\n"));
	if ((screen->pid = fork()) == -1)
	    SysError(ERROR_FORK);

	if (screen->pid == 0) {
#ifdef USE_USG_PTYS
	    int ptyfd = -1;
	    char *pty_name;
#endif
	    /*
	     * now in child process
	     */
#ifdef HAVE_SETSID
	    int pgrp = setsid();	/* variable may not be used... */
#else
	    int pgrp = getpid();
#endif
	    TRACE_CHILD;

#ifdef USE_USG_PTYS
#ifdef HAVE_SETPGID
	    setpgid(0, 0);
#else
	    setpgrp();
#endif
	    unlockpt(screen->respond);
	    TRACE_GET_TTYSIZE(screen->respond, "after unlockpt");
	    if ((pty_name = ptsname(screen->respond)) == NULL) {
		SysError(ERROR_PTSNAME);
	    } else if ((ptyfd = open(pty_name, O_RDWR)) < 0) {
		SysError(ERROR_OPPTSNAME);
	    }
#ifdef I_PUSH
	    else if (PUSH_FAILS(ptyfd, "ptem")) {
		SysError(ERROR_PTEM);
	    }
#if !defined(SVR4) && !(defined(SYSV) && defined(i386))
	    else if (!x_getenv("CONSEM")
		     && PUSH_FAILS(ptyfd, "consem")) {
		SysError(ERROR_CONSEM);
	    }
#endif /* !SVR4 */
	    else if (PUSH_FAILS(ptyfd, "ldterm")) {
		SysError(ERROR_LDTERM);
	    }
#ifdef SVR4			/* from Sony */
	    else if (PUSH_FAILS(ptyfd, "ttcompat")) {
		SysError(ERROR_TTCOMPAT);
	    }
#endif /* SVR4 */
#endif /* I_PUSH */
	    ttyfd = ptyfd;
	    close_fd(screen->respond);

#ifdef TTYSIZE_STRUCT
	    /* tell tty how big window is */
#if OPT_TEK4014
	    if (TEK4014_ACTIVE(xw)) {
		setup_winsize(ts, TDefaultRows, TDefaultCols,
			      TFullHeight(TekScreenOf(tekWidget)),
			      TFullWidth(TekScreenOf(tekWidget)));
	    } else
#endif /* OPT_TEK4014 */
	    {
		setup_winsize(ts, MaxRows(screen), MaxCols(screen),
			      FullHeight(screen), FullWidth(screen));
	    }
	    trace_winsize(ts, "initial tty size");
#endif /* TTYSIZE_STRUCT */

#endif /* USE_USG_PTYS */

	    (void) pgrp;	/* not all branches use this variable */

#if OPT_PTY_HANDSHAKE		/* warning, goes for a long ways */
	    if (resource.ptyHandshake) {
		char *ptr;

		/* close parent's sides of the pipes */
		close(cp_pipe[0]);
		close(pc_pipe[1]);

		/* Make sure that our sides of the pipes are not in the
		 * 0, 1, 2 range so that we don't fight with stdin, out
		 * or err.
		 */
		if (cp_pipe[1] <= 2) {
		    if ((i = fcntl(cp_pipe[1], F_DUPFD, 3)) >= 0) {
			IGNORE_RC(close(cp_pipe[1]));
			cp_pipe[1] = i;
		    }
		}
		if (pc_pipe[0] <= 2) {
		    if ((i = fcntl(pc_pipe[0], F_DUPFD, 3)) >= 0) {
			IGNORE_RC(close(pc_pipe[0]));
			pc_pipe[0] = i;
		    }
		}

		/* we don't need the socket, or the pty master anymore */
		close(ConnectionNumber(screen->display));
		if (screen->respond >= 0)
		    close(screen->respond);

		/* Now is the time to set up our process group and
		 * open up the pty slave.
		 */
#ifdef USE_SYSV_PGRP
#if defined(CRAY) && (OSMAJORVERSION > 5)
		IGNORE_RC(setsid());
#else
		IGNORE_RC(setpgrp());
#endif
#endif /* USE_SYSV_PGRP */

#if defined(__QNX__) && !defined(__QNXNTO__)
		qsetlogin(getlogin(), ttydev);
#endif
		if (ttyfd >= 0) {
		    close_fd(ttyfd);
		}

		for (;;) {
#if USE_NO_DEV_TTY
		    if (!no_dev_tty
			&& (ttyfd = open("/dev/tty", O_RDWR)) >= 0) {
			ioctl(ttyfd, TIOCNOTTY, (char *) NULL);
			close_fd(ttyfd);
		    }
#endif /* USE_NO_DEV_TTY */
#ifdef CSRG_BASED
		    IGNORE_RC(revoke(ttydev));
#endif
		    if ((ttyfd = open(ttydev, O_RDWR)) >= 0) {
			TRACE_GET_TTYSIZE(ttyfd, "after open");
			TRACE_RC(i, SET_TTYSIZE(ttyfd, ts));
			TRACE_GET_TTYSIZE(ttyfd, "after SET_TTYSIZE fixup");
#if defined(CRAY) && defined(TCSETCTTY)
			/* make /dev/tty work */
			ioctl(ttyfd, TCSETCTTY, 0);
#endif
#if ((defined(__GLIBC__) && defined(__FreeBSD_kernel__)) || defined(__GNU__)) && defined(TIOCSCTTY)
			/* make /dev/tty work */
			ioctl(ttyfd, TIOCSCTTY, 0);
#endif
#ifdef USE_SYSV_PGRP
			/* We need to make sure that we are actually
			 * the process group leader for the pty.  If
			 * we are, then we should now be able to open
			 * /dev/tty.
			 */
			if ((i = open("/dev/tty", O_RDWR)) >= 0) {
			    /* success! */
			    close(i);
			    break;
			}
#else /* USE_SYSV_PGRP */
			break;
#endif /* USE_SYSV_PGRP */
		    }
		    perror("open ttydev");
#ifdef TIOCSCTTY
		    ioctl(ttyfd, TIOCSCTTY, 0);
#endif
		    /* let our master know that the open failed */
		    handshake.status = PTY_BAD;
		    handshake.error = errno;
		    copy_handshake(handshake, ttydev);
		    TRACE_HANDSHAKE("writing", &handshake);
		    IGNORE_RC(write(cp_pipe[1],
				    (const char *) &handshake,
				    sizeof(handshake)));

		    /* get reply from parent */
		    i = (int) read(pc_pipe[0], (char *) &handshake,
				   sizeof(handshake));
		    if (i <= 0) {
			/* parent terminated */
			exit(ERROR_MISC);
		    }

		    if (handshake.status == PTY_NOMORE) {
			/* No more ptys, let's shutdown. */
			exit(ERROR_MISC);
		    }

		    /* We have a new pty to try */
		    if (ttyfd >= 0)
			close(ttyfd);
		    free(ttydev);
		    handshake.buffer[HANDSHAKE_LEN - 1] = '\0';
		    ttydev = x_strdup(handshake.buffer);
		}

		/* use the same tty name that everyone else will use
		 * (from ttyname)
		 */
		if ((ptr = ttyname(ttyfd)) != NULL) {
		    free(ttydev);
		    ttydev = x_strdup(ptr);
		}
	    }
#endif /* OPT_PTY_HANDSHAKE -- from near fork */

	    set_pty_permissions(screen->uid,
				(unsigned) screen->gid,
				(resource.messages
				 ? 0622U
				 : 0600U));

	    /*
	     * set up the tty modes
	     */
	    {
#ifdef TERMIO_STRUCT
#if defined(umips) || defined(CRAY) || defined(__linux__)
		/* If the control tty had its modes screwed around with,
		   eg. by lineedit in the shell, or emacs, etc. then tio
		   will have bad values.  Let's just get termio from the
		   new tty and tailor it.  */
		if (ttyGetAttr(ttyfd, &tio) == -1)
		    SysError(ERROR_TIOCGETP);
		tio.c_lflag |= ECHOE;
#endif /* umips */
		/* Now is also the time to change the modes of the
		 * child pty.
		 */
		/* input: nl->nl, don't ignore cr, cr->nl */
		UIntClr(tio.c_iflag, (INLCR | IGNCR));
		tio.c_iflag |= ICRNL;
#if OPT_WIDE_CHARS && defined(IUTF8)
#if OPT_LUIT_PROG
		if (command_to_exec_with_luit == NULL)
#endif
		    if (screen->utf8_mode)
			tio.c_iflag |= IUTF8;
#endif
		/* output: cr->cr, nl is not return, no delays, ln->cr/nl */
#ifndef USE_POSIX_TERMIOS
		UIntClr(tio.c_oflag,
			(OCRNL
			 | ONLRET
			 | NLDLY
			 | CRDLY
			 | TABDLY
			 | BSDLY
			 | VTDLY
			 | FFDLY));
#endif /* USE_POSIX_TERMIOS */
		tio.c_oflag |= D_TIO_FLAGS;
#ifndef USE_POSIX_TERMIOS
# if defined(Lynx) && !defined(CBAUD)
#  define CBAUD V_CBAUD
# endif
		UIntClr(tio.c_cflag, CBAUD);
#ifdef BAUD_0
		/* baud rate is 0 (don't care) */
#elif defined(HAVE_TERMIO_C_ISPEED)
		tio.c_ispeed = tio.c_ospeed = line_speed;
#else /* !BAUD_0 */
		tio.c_cflag |= line_speed;
#endif /* !BAUD_0 */
#else /* USE_POSIX_TERMIOS */
		cfsetispeed(&tio, line_speed);
		cfsetospeed(&tio, line_speed);
		/* Clear CLOCAL so that SIGHUP is sent to us
		   when the xterm ends */
		tio.c_cflag &= (unsigned) ~CLOCAL;
#endif /* USE_POSIX_TERMIOS */
		/* enable signals, canonical processing (erase, kill, etc),
		 * echo
		 */
		tio.c_lflag |= ISIG | ICANON | ECHO | ECHOE | ECHOK;
#ifdef ECHOKE
		tio.c_lflag |= ECHOKE | IEXTEN;
#endif
#ifdef ECHOCTL
		tio.c_lflag |= ECHOCTL | IEXTEN;
#endif
		for (nn = 0; nn < XtNumber(ttyChars); ++nn) {
		    if (validTtyChar(tio, nn)) {
			int sysMode = ttyChars[nn].sysMode;
			tio.c_cc[sysMode] = (cc_t) ttyChars[nn].myDefault;
		    }
		}

		if (override_tty_modes) {
		    TRACE(("applying termios ttyModes\n"));
		    for (nn = 0; nn < XtNumber(ttyChars); ++nn) {
			if (validTtyChar(tio, nn)) {
			    TMODE(ttyChars[nn].myMode,
				  tio.c_cc[ttyChars[nn].sysMode]);
			} else if (isTabMode(nn)) {
			    unsigned tmp = (unsigned) tio.c_oflag;
			    tmp = tmp & (unsigned) ~TABDLY;
			    tmp |= (unsigned) ttyModes[ttyChars[nn].myMode].value;
			    tio.c_oflag = tmp;
			}
		    }
#ifdef HAS_LTCHARS
		    /* both SYSV and BSD have ltchars */
		    TMODE(XTTYMODE_susp, ltc.t_suspc);
		    TMODE(XTTYMODE_dsusp, ltc.t_dsuspc);
		    TMODE(XTTYMODE_rprnt, ltc.t_rprntc);
		    TMODE(XTTYMODE_flush, ltc.t_flushc);
		    TMODE(XTTYMODE_weras, ltc.t_werasc);
		    TMODE(XTTYMODE_lnext, ltc.t_lnextc);
#endif
		}
#ifdef HAS_LTCHARS
#ifdef __hpux
		/* ioctl chokes when the "reserved" process group controls
		 * are not set to _POSIX_VDISABLE */
		ltc.t_rprntc = _POSIX_VDISABLE;
		ltc.t_rprntc = _POSIX_VDISABLE;
		ltc.t_flushc = _POSIX_VDISABLE;
		ltc.t_werasc = _POSIX_VDISABLE;
		ltc.t_lnextc = _POSIX_VDISABLE;
#endif /* __hpux */
		if (ioctl(ttyfd, TIOCSLTC, &ltc) == -1)
		    HsSysError(ERROR_TIOCSETC);
#endif /* HAS_LTCHARS */
#ifdef TIOCLSET
		if (ioctl(ttyfd, TIOCLSET, (char *) &lmode) == -1)
		    HsSysError(ERROR_TIOCLSET);
#endif /* TIOCLSET */
		if (ttySetAttr(ttyfd, &tio) == -1)
		    HsSysError(ERROR_TIOCSETP);

		/* ignore errors here - some platforms don't work */
		UIntClr(tio.c_cflag, CSIZE);
		if (screen->input_eight_bits)
		    tio.c_cflag |= CS8;
		else
		    tio.c_cflag |= CS7;
		(void) ttySetAttr(ttyfd, &tio);

#else /* !TERMIO_STRUCT */
		sg.sg_flags &= ~(ALLDELAY | XTABS | CBREAK | RAW);
		sg.sg_flags |= ECHO | CRMOD;
		/* make sure speed is set on pty so that editors work right */
		sg.sg_ispeed = line_speed;
		sg.sg_ospeed = line_speed;
		/* reset t_brkc to default value */
		tc.t_brkc = -1;
#ifdef LPASS8
		if (screen->input_eight_bits)
		    lmode |= LPASS8;
		else
		    lmode &= ~(LPASS8);
#endif
#ifdef sony
		jmode &= ~KM_KANJI;
#endif /* sony */

		ltc = d_ltc;

		if (override_tty_modes) {
		    TRACE(("applying sgtty ttyModes\n"));
		    TMODE(XTTYMODE_intr, tc.t_intrc);
		    TMODE(XTTYMODE_quit, tc.t_quitc);
		    TMODE(XTTYMODE_erase, sg.sg_erase);
		    TMODE(XTTYMODE_kill, sg.sg_kill);
		    TMODE(XTTYMODE_eof, tc.t_eofc);
		    TMODE(XTTYMODE_start, tc.t_startc);
		    TMODE(XTTYMODE_stop, tc.t_stopc);
		    TMODE(XTTYMODE_brk, tc.t_brkc);
		    /* both SYSV and BSD have ltchars */
		    TMODE(XTTYMODE_susp, ltc.t_suspc);
		    TMODE(XTTYMODE_dsusp, ltc.t_dsuspc);
		    TMODE(XTTYMODE_rprnt, ltc.t_rprntc);
		    TMODE(XTTYMODE_flush, ltc.t_flushc);
		    TMODE(XTTYMODE_weras, ltc.t_werasc);
		    TMODE(XTTYMODE_lnext, ltc.t_lnextc);
		    if (ttyModes[XTTYMODE_tabs].set
			|| ttyModes[XTTYMODE__tabs].set) {
			sg.sg_flags &= ~XTABS;
			if (ttyModes[XTTYMODE__tabs].set.set)
			    sg.sg_flags |= XTABS;
		    }
		}

		if (ioctl(ttyfd, TIOCSETP, (char *) &sg) == -1)
		    HsSysError(ERROR_TIOCSETP);
		if (ioctl(ttyfd, TIOCSETC, (char *) &tc) == -1)
		    HsSysError(ERROR_TIOCSETC);
		if (ioctl(ttyfd, TIOCSETD, (char *) &discipline) == -1)
		    HsSysError(ERROR_TIOCSETD);
		if (ioctl(ttyfd, TIOCSLTC, (char *) &ltc) == -1)
		    HsSysError(ERROR_TIOCSLTC);
		if (ioctl(ttyfd, TIOCLSET, (char *) &lmode) == -1)
		    HsSysError(ERROR_TIOCLSET);
#ifdef sony
		if (ioctl(ttyfd, TIOCKSET, (char *) &jmode) == -1)
		    HsSysError(ERROR_TIOCKSET);
		if (ioctl(ttyfd, TIOCKSETC, (char *) &jtc) == -1)
		    HsSysError(ERROR_TIOCKSETC);
#endif /* sony */
#endif /* TERMIO_STRUCT */
#if defined(TIOCCONS) || defined(SRIOCSREDIR)
		if (Console) {
#ifdef TIOCCONS
		    int on = 1;
		    if (ioctl(ttyfd, TIOCCONS, (char *) &on) == -1)
			xtermPerror("cannot open console");
#endif
#ifdef SRIOCSREDIR
		    int fd = open("/dev/console", O_RDWR);
		    if (fd == -1 || ioctl(fd, SRIOCSREDIR, ttyfd) == -1)
			xtermPerror("cannot open console");
		    IGNORE_RC(close(fd));
#endif
		}
#endif /* TIOCCONS */
	    }

	    signal(SIGCHLD, SIG_DFL);
#ifdef USE_SYSV_SIGHUP
	    /* watch out for extra shells (I don't understand either) */
	    signal(SIGHUP, SIG_DFL);
#else
	    signal(SIGHUP, SIG_IGN);
#endif
	    /* restore various signals to their defaults */
	    signal(SIGINT, SIG_DFL);
	    signal(SIGQUIT, SIG_DFL);
	    signal(SIGTERM, SIG_DFL);

	    /*
	     * If we're not asked to let the parent process set the terminal's
	     * erase mode, or if we had the ttyModes erase resource, then set
	     * the terminal's erase mode from our best guess.
	     */
#if OPT_INITIAL_ERASE
	    TRACE(("check if we should set erase to %d:%s\n\tptyInitialErase:%d,\n\toveride_tty_modes:%d,\n\tXTTYMODE_erase:%d\n",
		   initial_erase,
		   setInitialErase ? "YES" : "NO",
		   resource.ptyInitialErase,
		   override_tty_modes,
		   ttyModes[XTTYMODE_erase].set));
	    if (setInitialErase) {
#if OPT_TRACE
		int old_erase;
#endif
#ifdef TERMIO_STRUCT
		if (ttyGetAttr(ttyfd, &tio) == -1)
		    tio = d_tio;
#if OPT_TRACE
		old_erase = tio.c_cc[VERASE];
#endif
		tio.c_cc[VERASE] = (cc_t) initial_erase;
		TRACE_RC(rc, ttySetAttr(ttyfd, &tio));
#else /* !TERMIO_STRUCT */
		if (ioctl(ttyfd, TIOCGETP, (char *) &sg) == -1)
		    sg = d_sg;
#if OPT_TRACE
		old_erase = sg.sg_erase;
#endif
		sg.sg_erase = initial_erase;
		rc = ioctl(ttyfd, TIOCSETP, (char *) &sg);
#endif /* TERMIO_STRUCT */
		TRACE(("%s setting erase to %d (was %d)\n",
		       rc ? "FAIL" : "OK", initial_erase, old_erase));
	    }
#endif

	    xtermCopyEnv(environ);
	    xtermTrimEnv();

	    xtermSetenv("TERM", resource.term_name);
	    if (!resource.term_name)
		*get_tcap_buffer(xw) = 0;

	    sprintf(buf, "%lu",
		    ((unsigned long) XtWindow(SHELL_OF(CURRENT_EMU()))));
	    xtermSetenv("WINDOWID", buf);

	    /* put the display into the environment of the shell */
	    xtermSetenv("DISPLAY", XDisplayString(screen->display));

	    xtermSetenv("XTERM_VERSION", xtermVersion());
	    xtermSetenv("XTERM_LOCALE", xtermEnvLocale());

	    /*
	     * For debugging only, add environment variables that can be used
	     * in scripts to selectively kill xterm's parent or child
	     * processes.
	     */
#if OPT_TRACE
	    sprintf(buf, "%lu", (unsigned long) xterm_parent);
	    xtermSetenv("XTERM_PARENT", buf);
	    sprintf(buf, "%lu", (unsigned long) getpid());
	    xtermSetenv("XTERM_CHILD", buf);
#endif

	    signal(SIGTERM, SIG_DFL);

	    /* this is the time to go and set up stdin, out, and err
	     */
	    {
#if defined(CRAY) && (OSMAJORVERSION >= 6)
		close_fd(ttyfd);

		IGNORE_RC(close(0));

		if (open("/dev/tty", O_RDWR)) {
		    SysError(ERROR_OPDEVTTY);
		}
		IGNORE_RC(close(1));
		IGNORE_RC(close(2));
		dup(0);
		dup(0);
#else
		/* dup the tty */
		for (i = 0; i <= 2; i++)
		    if (i != ttyfd) {
			IGNORE_RC(close(i));
			IGNORE_RC(dup(ttyfd));
		    }
#ifndef ATT
		/* and close the tty */
		if (ttyfd > 2)
		    close_fd(ttyfd);
#endif
#endif /* CRAY */
	    }

#if !defined(USE_SYSV_PGRP)
#ifdef TIOCSCTTY
	    setsid();
	    ioctl(0, TIOCSCTTY, 0);
#endif
	    ioctl(0, TIOCSPGRP, (char *) &pgrp);
	    setpgrp(0, 0);
	    close(open(ttydev, O_WRONLY));
	    setpgrp(0, pgrp);
#if defined(__QNX__)
	    tcsetpgrp(0, pgrp /*setsid() */ );
#endif
#endif /* !USE_SYSV_PGRP */

#ifdef Lynx
	    {
		TERMIO_STRUCT t;
		if (ttyGetAttr(0, &t) >= 0) {
		    /* this gets lost somewhere on our way... */
		    t.c_oflag |= OPOST;
		    ttySetAttr(0, &t);
		}
	    }
#endif

#ifdef HAVE_UTMP
	    login_name = NULL;
	    if (x_getpwuid(screen->uid, &pw)) {
		login_name = x_getlogin(screen->uid, &pw);
	    }
	    if (login_name != NULL) {
		xtermSetenv("LOGNAME", login_name);	/* for POSIX */
	    }
#ifndef USE_UTEMPTER
#ifdef USE_UTMP_SETGID
	    setEffectiveGroup(save_egid);
	    TRACE_IDS;
#endif
#ifdef USE_SYSV_UTMP
	    /* Set up our utmp entry now.  We need to do it here
	     * for the following reasons:
	     *   - It needs to have our correct process id (for
	     *     login).
	     *   - If our parent was to set it after the fork(),
	     *     it might make it out before we need it.
	     *   - We need to do it before we go and change our
	     *     user and group id's.
	     */
	    (void) call_setutent();
	    init_utmp(DEAD_PROCESS, &utmp);

	    /* position to entry in utmp file */
	    /* Test return value: beware of entries left behind: PSz 9 Mar 00 */
	    utret = find_utmp(&utmp);
	    if (utret == NULL) {
		(void) call_setutent();
		init_utmp(USER_PROCESS, &utmp);
		utret = find_utmp(&utmp);
		if (utret == NULL) {
		    (void) call_setutent();
		}
	    }
#if OPT_TRACE
	    if (!utret)
		TRACE(("getutid: NULL\n"));
	    else
		TRACE(("getutid: pid=%d type=%d user=%s line=%.*s id=%.*s\n",
		       (int) utret->ut_pid, utret->ut_type, utret->ut_user,
		       (int) sizeof(utret->ut_line), utret->ut_line,
		       (int) sizeof(utret->ut_id), utret->ut_id));
#endif

	    /* set up the new entry */
	    utmp.ut_type = USER_PROCESS;
#ifdef HAVE_UTMP_UT_XSTATUS
	    utmp.ut_xstatus = 2;
#endif
	    copy_filled(utmp.ut_user,
			(login_name != NULL) ? login_name : "????",
			sizeof(utmp.ut_user));
	    /* why are we copying this string again?  (see above) */
	    copy_filled(utmp.ut_id, my_utmp_id(ttydev), sizeof(utmp.ut_id));
	    copy_filled(utmp.ut_line,
			my_pty_name(ttydev), sizeof(utmp.ut_line));

#ifdef HAVE_UTMP_UT_HOST
	    SetUtmpHost(utmp.ut_host, screen);
#endif
#ifdef HAVE_UTMP_UT_SYSLEN
	    SetUtmpSysLen(utmp);
#endif

	    copy_filled(utmp.ut_name,
			(login_name) ? login_name : "????",
			sizeof(utmp.ut_name));

	    utmp.ut_pid = getpid();
#if defined(HAVE_UTMP_UT_XTIME)
#if defined(HAVE_UTMP_UT_SESSION)
	    utmp.ut_session = getsid(0);
#endif
	    utmp.ut_xtime = time((time_t *) 0);
	    utmp.ut_tv.tv_usec = 0;
#else
	    utmp.ut_time = time((time_t *) 0);
#endif

	    /* write out the entry */
	    if (!resource.utmpInhibit) {
		errno = 0;
		call_pututline(&utmp);
		TRACE(("pututline: id %.*s, line %.*s, pid %ld, errno %d %s\n",
		       (int) sizeof(utmp.ut_id), utmp.ut_id,
		       (int) sizeof(utmp.ut_line), utmp.ut_line,
		       (long) utmp.ut_pid,
		       errno, (errno != 0) ? strerror(errno) : ""));
	    }
#ifdef WTMP
#if defined(WTMPX_FILE) && (defined(SVR4) || defined(__SCO__))
	    if (xw->misc.login_shell)
		updwtmpx(WTMPX_FILE, &utmp);
#elif defined(__linux__) && defined(__GLIBC__) && (__GLIBC__ >= 2) && !(defined(__powerpc__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ == 0))
	    if (xw->misc.login_shell)
		call_updwtmp(etc_wtmp, &utmp);
#else
	    if (xw->misc.login_shell &&
		(i = open(etc_wtmp, O_WRONLY | O_APPEND)) >= 0) {
		IGNORE_RC(write(i, (char *) &utmp, sizeof(utmp)));
		close(i);
	    }
#endif
#endif
	    /* close the file */
	    (void) call_endutent();

#else /* USE_SYSV_UTMP */
	    /* We can now get our ttyslot!  We can also set the initial
	     * utmp entry.
	     */
	    tslot = ttyslot();
	    added_utmp_entry = False;
	    {
		if (tslot > 0 && OkPasswd(&pw) && !resource.utmpInhibit &&
		    (i = open(etc_utmp, O_WRONLY)) >= 0) {
		    memset(&utmp, 0, sizeof(utmp));
		    copy_filled(utmp.ut_line,
				my_pty_name(ttydev),
				sizeof(utmp.ut_line));
		    copy_filled(utmp.ut_name, login_name,
				sizeof(utmp.ut_name));
#ifdef HAVE_UTMP_UT_HOST
		    SetUtmpHost(utmp.ut_host, screen);
#endif
#ifdef HAVE_UTMP_UT_SYSLEN
		    SetUtmpSysLen(utmp);
#endif

		    utmp.ut_time = time((time_t *) 0);
		    lseek(i, (long) (tslot * sizeof(utmp)), 0);
		    IGNORE_RC(write(i, (char *) &utmp, sizeof(utmp)));
		    close(i);
		    added_utmp_entry = True;
#if defined(WTMP)
		    if (xw->misc.login_shell &&
			(i = open(etc_wtmp, O_WRONLY | O_APPEND)) >= 0) {
			int status;
			status = write(i, (char *) &utmp, sizeof(utmp));
			status = close(i);
		    }
#elif defined(MNX_LASTLOG)
		    if (xw->misc.login_shell &&
			(i = open(_U_LASTLOG, O_WRONLY)) >= 0) {
			lseek(i, (long) (screen->uid *
					 sizeof(utmp)), 0);
			IGNORE_RC(write(i, (char *) &utmp, sizeof(utmp)));
			close(i);
		    }
#endif /* WTMP or MNX_LASTLOG */
		} else
		    tslot = -tslot;
	    }

	    /* Let's pass our ttyslot to our parent so that it can
	     * clean up after us.
	     */
#if OPT_PTY_HANDSHAKE
	    if (resource.ptyHandshake) {
		handshake.tty_slot = tslot;
	    }
#endif /* OPT_PTY_HANDSHAKE */
#endif /* USE_SYSV_UTMP */

#ifdef USE_LASTLOGX
	    if (xw->misc.login_shell) {
		memset(&lastlogx, 0, sizeof(lastlogx));
		copy_filled(lastlogx.ll_line,
			    my_pty_name(ttydev),
			    sizeof(lastlogx.ll_line));
		X_GETTIMEOFDAY(&lastlogx.ll_tv);
		SetUtmpHost(lastlogx.ll_host, screen);
		updlastlogx(_PATH_LASTLOGX, screen->uid, &lastlogx);
	    }
#endif

#ifdef USE_LASTLOG
	    if (xw->misc.login_shell &&
		(i = open(etc_lastlog, O_WRONLY)) >= 0) {
		size_t size = sizeof(struct lastlog);
		off_t offset = (off_t) ((size_t) screen->uid * size);

		memset(&lastlog, 0, size);
		copy_filled(lastlog.ll_line,
			    my_pty_name(ttydev),
			    sizeof(lastlog.ll_line));
		SetUtmpHost(lastlog.ll_host, screen);
		lastlog.ll_time = time((time_t *) 0);
		if (lseek(i, offset, 0) != (off_t) (-1)) {
		    IGNORE_RC(write(i, (char *) &lastlog, size));
		}
		close(i);
	    }
#endif /* USE_LASTLOG */

#if defined(USE_UTMP_SETGID)
	    disableSetGid();
	    TRACE_IDS;
#endif

#if OPT_PTY_HANDSHAKE
	    /* Let our parent know that we set up our utmp entry
	     * so that it can clean up after us.
	     */
	    if (resource.ptyHandshake) {
		handshake.status = UTMP_ADDED;
		handshake.error = 0;
		copy_handshake(handshake, ttydev);
		TRACE_HANDSHAKE("writing", &handshake);
		IGNORE_RC(write(cp_pipe[1], (char *) &handshake, sizeof(handshake)));
	    }
#endif /* OPT_PTY_HANDSHAKE */
#endif /* USE_UTEMPTER */
#endif /* HAVE_UTMP */

	    IGNORE_RC(setgid(screen->gid));
	    TRACE_IDS;
#ifdef HAVE_INITGROUPS
	    if (geteuid() == 0 && OkPasswd(&pw)) {
		if (initgroups(login_name, pw.pw_gid)) {
		    perror("initgroups failed");
		    SysError(ERROR_INIGROUPS);
		}
	    }
#endif
	    if (setuid(screen->uid)) {
		SysError(ERROR_SETUID);
	    }
	    TRACE_IDS;
#if OPT_PTY_HANDSHAKE
	    if (resource.ptyHandshake) {
		/* mark the pipes as close on exec */
		(void) fcntl(cp_pipe[1], F_SETFD, 1);
		(void) fcntl(pc_pipe[0], F_SETFD, 1);

		/* We are at the point where we are going to
		 * exec our shell (or whatever).  Let our parent
		 * know we arrived safely.
		 */
		handshake.status = PTY_GOOD;
		handshake.error = 0;
		copy_handshake(handshake, ttydev);
		TRACE_HANDSHAKE("writing", &handshake);
		IGNORE_RC(write(cp_pipe[1],
				(const char *) &handshake,
				sizeof(handshake)));

		if (resource.wait_for_map) {
		    i = (int) read(pc_pipe[0], (char *) &handshake,
				   sizeof(handshake));
		    if (i != sizeof(handshake) ||
			handshake.status != PTY_EXEC) {
			/* some very bad problem occurred */
			exit(ERROR_PTY_EXEC);
		    }
		    if (handshake.rows > 0 && handshake.cols > 0) {
			TRACE(("handshake read ttysize: %dx%d\n",
			       handshake.rows, handshake.cols));
			set_max_row(screen, handshake.rows);
			set_max_col(screen, handshake.cols);
#ifdef TTYSIZE_STRUCT
			got_handshake_size = True;
			setup_winsize(ts, MaxRows(screen), MaxCols(screen),
				      FullHeight(screen), FullWidth(screen));
			trace_winsize(ts, "got handshake");
#endif /* TTYSIZE_STRUCT */
		    }
		}
	    }
#endif /* OPT_PTY_HANDSHAKE */

#ifdef USE_SYSV_ENVVARS
	    {
		char numbuf[12];
		sprintf(numbuf, "%d", MaxCols(screen));
		xtermSetenv("COLUMNS", numbuf);
		sprintf(numbuf, "%d", MaxRows(screen));
		xtermSetenv("LINES", numbuf);
	    }
#ifdef HAVE_UTMP
	    if (OkPasswd(&pw)) {	/* SVR4 doesn't provide these */
		if (!x_getenv("HOME"))
		    xtermSetenv("HOME", pw.pw_dir);
		if (!x_getenv("SHELL"))
		    xtermSetenv("SHELL", pw.pw_shell);
	    }
#endif /* HAVE_UTMP */
#else /* USE_SYSV_ENVVARS */
	    if (*(newtc = get_tcap_buffer(xw)) != '\0') {
		resize_termcap(xw);
		if (xw->misc.titeInhibit && !xw->misc.tiXtraScroll) {
		    remove_termcap_entry(newtc, "ti=");
		    remove_termcap_entry(newtc, "te=");
		}
		/*
		 * work around broken termcap entries */
		if (resource.useInsertMode) {
		    remove_termcap_entry(newtc, "ic=");
		    /* don't get duplicates */
		    remove_termcap_entry(newtc, "im=");
		    remove_termcap_entry(newtc, "ei=");
		    remove_termcap_entry(newtc, "mi");
		    if (*newtc)
			strcat(newtc, ":im=\\E[4h:ei=\\E[4l:mi:");
		}
		if (*newtc) {
#if OPT_INITIAL_ERASE
#define TERMCAP_ERASE "kb"
		    unsigned len;
		    remove_termcap_entry(newtc, TERMCAP_ERASE "=");
		    len = (unsigned) strlen(newtc);
		    if (len != 0 && newtc[len - 1] == ':')
			len--;
		    sprintf(newtc + len, ":%s=\\%03o:",
			    TERMCAP_ERASE,
			    CharOf(initial_erase));
#endif
		    xtermSetenv("TERMCAP", newtc);
		}
	    }
#endif /* USE_SYSV_ENVVARS */
#ifdef OWN_TERMINFO_ENV
	    xtermSetenv("TERMINFO", OWN_TERMINFO_DIR);
#endif

#if OPT_PTY_HANDSHAKE
	    /*
	     * Need to reset after all the ioctl bashing we did above.
	     *
	     * If we expect the waitForMap logic to set the handshake-size,
	     * use that to prevent races.
	     */
	    TRACE(("should we reset screensize after pty-handshake?\n"));
	    TRACE(("... ptyHandshake      :%d\n", resource.ptyHandshake));
	    TRACE(("... ptySttySize       :%d\n", resource.ptySttySize));
	    TRACE(("... got_handshake_size:%d\n", got_handshake_size));
	    TRACE(("... wait_for_map0     :%d\n", resource.wait_for_map0));
	    if (resource.ptyHandshake
		&& resource.ptySttySize
		&& (got_handshake_size || !resource.wait_for_map0)) {
#ifdef TTYSIZE_STRUCT
		TRACE_RC(i, SET_TTYSIZE(0, ts));
		trace_winsize(ts, "ptyHandshake SET_TTYSIZE");
#endif /* TTYSIZE_STRUCT */
	    }
#endif /* OPT_PTY_HANDSHAKE */
	    signal(SIGHUP, SIG_DFL);

	    /*
	     * If we have an explicit shell to run, make that set $SHELL.
	     * Next, allow an existing setting of $SHELL, for absolute paths.
	     * Otherwise, if $SHELL is not set, determine it from the user's
	     * password information, if possible.
	     *
	     * Incidentally, our setting of $SHELL tells luit to use that
	     * program rather than choosing between $SHELL and "/bin/sh".
	     */
	    if (validShell(explicit_shname)) {
		xtermSetenv("SHELL", explicit_shname);
	    } else if (validProgram(shell_path = x_getenv("SHELL"))) {
		if (!validShell(shell_path)) {
		    xtermUnsetenv("SHELL");
		}
	    } else if ((!OkPasswd(&pw) && !x_getpwuid(screen->uid, &pw))
		       || *(shell_path = x_strdup(pw.pw_shell)) == 0) {
		shell_path = resetShell(shell_path);
	    } else if (validShell(shell_path)) {
		xtermSetenv("SHELL", shell_path);
	    } else {
		shell_path = resetShell(shell_path);
	    }

	    /*
	     * Set $XTERM_SHELL, which is not necessarily a valid shell, but
	     * is executable.
	     */
	    if (validProgram(explicit_shname)) {
		shell_path = explicit_shname;
	    } else if (shell_path == NULL) {
		/* this could happen if the explicit shname lost a race */
		shell_path = resetShell(shell_path);
	    }
	    xtermSetenv("XTERM_SHELL", shell_path);

	    shname = x_basename(shell_path);
	    TRACE(("shell path '%s' leaf '%s'\n", shell_path, shname));

#if OPT_LUIT_PROG
	    /*
	     * Use two copies of command_to_exec, in case luit is not actually
	     * there, or refuses to run.  In that case we will fall-through to
	     * to command that the user gave anyway.
	     */
	    if (command_to_exec_with_luit && command_to_exec) {
		char *myShell = xtermFindShell(*command_to_exec_with_luit, False);
		xtermSetenv("XTERM_SHELL", myShell);
		free(myShell);
		TRACE_ARGV("spawning luit command", command_to_exec_with_luit);
		execvp(*command_to_exec_with_luit, command_to_exec_with_luit);
		xtermPerror("Can't execvp %s", *command_to_exec_with_luit);
		xtermWarning("cannot support your locale.\n");
	    }
#endif
	    if (command_to_exec) {
		char *myShell = xtermFindShell(*command_to_exec, False);
		xtermSetenv("XTERM_SHELL", myShell);
		free(myShell);
		TRACE_ARGV("spawning command", command_to_exec);
		execvp(*command_to_exec, command_to_exec);
		if (command_to_exec[1] == NULL)
		    execlp(shell_path, shname, "-c", command_to_exec[0],
			   (void *) 0);
		xtermPerror("Can't execvp %s", *command_to_exec);
	    }
#ifdef USE_SYSV_SIGHUP
	    /* fix pts sh hanging around */
	    signal(SIGHUP, SIG_DFL);
#endif

	    if ((shname_minus = (char *) malloc(strlen(shname) + 2)) != NULL) {
		(void) strcpy(shname_minus, "-");
		(void) strcat(shname_minus, shname);
	    } else {
		static char default_minus[] = "-sh";
		shname_minus = default_minus;
	    }
#ifndef TERMIO_STRUCT
	    ldisc = (!XStrCmp("csh", shname + strlen(shname) - 3)
		     ? NTTYDISC
		     : 0);
	    ioctl(0, TIOCSETD, (char *) &ldisc);
#endif /* !TERMIO_STRUCT */

#ifdef USE_LOGIN_DASH_P
	    if (xw->misc.login_shell && OkPasswd(&pw) && added_utmp_entry)
		execl(bin_login, "login", "-p", "-f", login_name, (void *) 0);
#endif

#if OPT_LUIT_PROG
	    if (command_to_exec_with_luit) {
		if (xw->misc.login_shell) {
		    char *params[4];
		    params[0] = x_strdup("-argv0");
		    params[1] = shname_minus;
		    params[2] = NULL;
		    x_appendargv(command_to_exec_with_luit
				 + command_length_with_luit,
				 params);
		}
		TRACE_ARGV("final luit command", command_to_exec_with_luit);
		execvp(*command_to_exec_with_luit, command_to_exec_with_luit);
		/* Exec failed. */
		xtermPerror("Can't execvp %s", *command_to_exec_with_luit);
	    }
#endif
	    execlp(shell_path,
		   (xw->misc.login_shell ? shname_minus : shname),
		   (void *) 0);

	    /* Exec failed. */
	    xtermPerror("Could not exec %s", shell_path);
	    IGNORE_RC(sleep(5));
	    free(shell_path);
	    exit(ERROR_EXEC);
	}
	/* end if in child after fork */
#if OPT_PTY_HANDSHAKE
	if (resource.ptyHandshake) {
	    /* Parent process.  Let's handle handshaked requests to our
	     * child process.
	     */

	    /* close childs's sides of the pipes */
	    close(cp_pipe[1]);
	    close(pc_pipe[0]);

	    for (done = 0; !done;) {
		if (read(cp_pipe[0],
			 (char *) &handshake,
			 sizeof(handshake)) <= 0) {
		    /* Our child is done talking to us.  If it terminated
		     * due to an error, we will catch the death of child
		     * and clean up.
		     */
		    break;
		}

		TRACE_HANDSHAKE("read", &handshake);
		switch (handshake.status) {
		case PTY_GOOD:
		    /* Success!  Let's free up resources and
		     * continue.
		     */
		    done = 1;
		    break;

		case PTY_BAD:
		    /* The open of the pty failed!  Let's get
		     * another one.
		     */
		    IGNORE_RC(close(screen->respond));
		    if (get_pty(&screen->respond, XDisplayString(screen->display))) {
			/* no more ptys! */
			xtermPerror("child process can find no available ptys");
			handshake.status = PTY_NOMORE;
			TRACE_HANDSHAKE("writing", &handshake);
			IGNORE_RC(write(pc_pipe[1],
					(const char *) &handshake,
					sizeof(handshake)));
			exit(ERROR_PTYS);
		    }
		    handshake.status = PTY_NEW;
		    copy_handshake(handshake, ttydev);
		    TRACE_HANDSHAKE("writing", &handshake);
		    IGNORE_RC(write(pc_pipe[1],
				    (const char *) &handshake,
				    sizeof(handshake)));
		    break;

		case PTY_FATALERROR:
		    errno = handshake.error;
		    close(cp_pipe[0]);
		    close(pc_pipe[1]);
		    SysError(handshake.fatal_error);
		    /*NOTREACHED */

		case UTMP_ADDED:
		    /* The utmp entry was set by our slave.  Remember
		     * this so that we can reset it later.
		     */
		    added_utmp_entry = True;
#ifndef	USE_SYSV_UTMP
		    tslot = handshake.tty_slot;
#endif /* USE_SYSV_UTMP */
		    free(ttydev);
		    handshake.buffer[HANDSHAKE_LEN - 1] = '\0';
		    ttydev = x_strdup(handshake.buffer);
		    break;
		case PTY_NEW:
		case PTY_NOMORE:
		case UTMP_TTYSLOT:
		case PTY_EXEC:
		default:
		    xtermWarning("unexpected handshake status %d\n",
				 (int) handshake.status);
		}
	    }
	    /* close our sides of the pipes */
	    if (!resource.wait_for_map) {
		close(cp_pipe[0]);
		close(pc_pipe[1]);
	    }
	}
#endif /* OPT_PTY_HANDSHAKE */
    }

    /* end if no slave */
    /*
     * still in parent (xterm process)
     */
#ifdef USE_SYSV_SIGHUP
    /* hung sh problem? */
    signal(SIGHUP, SIG_DFL);
#else
    signal(SIGHUP, SIG_IGN);
#endif

/*
 * Unfortunately, System V seems to have trouble divorcing the child process
 * from the process group of xterm.  This is a problem because hitting the
 * INTR or QUIT characters on the keyboard will cause xterm to go away if we
 * don't ignore the signals.  This is annoying.
 */

#if defined(USE_SYSV_SIGNALS) && !defined(SIGTSTP)
    signal(SIGINT, SIG_IGN);

#ifndef SYSV
    /* hung shell problem */
    signal(SIGQUIT, SIG_IGN);
#endif
    signal(SIGTERM, SIG_IGN);
#elif defined(SYSV) || defined(__osf__)
    /* if we were spawned by a jobcontrol smart shell (like ksh or csh),
     * then our pgrp and pid will be the same.  If we were spawned by
     * a jobcontrol dumb shell (like /bin/sh), then we will be in our
     * parent's pgrp, and we must ignore keyboard signals, or we will
     * tank on everything.
     */
    if (getpid() == getpgrp()) {
	(void) signal(SIGINT, Exit);
	(void) signal(SIGQUIT, Exit);
	(void) signal(SIGTERM, Exit);
    } else {
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	(void) signal(SIGTERM, SIG_IGN);
    }
    (void) signal(SIGPIPE, Exit);
#else /* SYSV */
    signal(SIGINT, Exit);
    signal(SIGQUIT, Exit);
    signal(SIGTERM, Exit);
    signal(SIGPIPE, Exit);
#endif /* USE_SYSV_SIGNALS and not SIGTSTP */
#ifdef NO_LEAKS
    if (ok_termcap != True)
	free(TermName);
#endif

    return 0;
}				/* end spawnXTerm */

void
Exit(int n)
{
    XtermWidget xw = term;
    TScreen *screen = TScreenOf(xw);

#ifdef OPT_LUA_SCRIPTING
    /* Clean up Lua scripting */
    lua_xterm_call_hook(LUA_HOOK_SHUTDOWN);
    lua_xterm_cleanup();
#endif

#ifdef USE_UTEMPTER
    DEBUG_MSG("handle:Exit USE_UTEMPTER\n");
    if (!resource.utmpInhibit && added_utmp_entry) {
	TRACE(("...calling removeFromUtmp\n"));
	UTEMPTER_DEL();
    }
#elif defined(HAVE_UTMP)
#ifdef USE_SYSV_UTMP
    struct UTMP_STR utmp;
    struct UTMP_STR *utptr;

    DEBUG_MSG("handle:Exit USE_SYSV_UTMP\n");
    /* don't do this more than once */
    if (xterm_exiting) {
	exit(n);
    }
    xterm_exiting = True;

#ifdef PUCC_PTYD
    closepty(ttydev, ptydev, (resource.utmpInhibit ? OPTY_NOP : OPTY_LOGIN), screen->respond);
#endif /* PUCC_PTYD */

    /* cleanup the utmp entry we forged earlier */
    if (!resource.utmpInhibit
#if OPT_PTY_HANDSHAKE		/* without handshake, no way to know */
	&& (resource.ptyHandshake && added_utmp_entry)
#endif /* OPT_PTY_HANDSHAKE */
	) {
#if defined(USE_UTMP_SETGID)
	setEffectiveGroup(save_egid);
	TRACE_IDS;
#endif
	init_utmp(USER_PROCESS, &utmp);
	(void) call_setutent();

	/*
	 * We could use getutline() if we didn't support old systems.
	 */
	while ((utptr = find_utmp(&utmp)) != NULL) {
	    if (utptr->ut_pid == screen->pid) {
		utptr->ut_type = DEAD_PROCESS;
#if defined(HAVE_UTMP_UT_XTIME)
#if defined(HAVE_UTMP_UT_SESSION)
		utptr->ut_session = getsid(0);
#endif
		utptr->ut_xtime = time((time_t *) 0);
		utptr->ut_tv.tv_usec = 0;
#else
		*utptr->ut_user = 0;
		utptr->ut_time = time((time_t *) 0);
#endif
		(void) call_pututline(utptr);
#ifdef WTMP
#if defined(WTMPX_FILE) && (defined(SVR4) || defined(__SCO__))
		if (xw->misc.login_shell)
		    updwtmpx(WTMPX_FILE, utptr);
#elif defined(__linux__) && defined(__GLIBC__) && (__GLIBC__ >= 2) && !(defined(__powerpc__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ == 0))
		copy_filled(utmp.ut_line, utptr->ut_line, sizeof(utmp.ut_line));
		if (xw->misc.login_shell)
		    call_updwtmp(etc_wtmp, utptr);
#else
		/* set wtmp entry if wtmp file exists */
		if (xw->misc.login_shell) {
		    int fd;
		    if ((fd = open(etc_wtmp, O_WRONLY | O_APPEND)) >= 0) {
			IGNORE_RC(write(fd, utptr, sizeof(*utptr)));
			close(fd);
		    }
		}
#endif
#endif
		break;
	    }
	    memset(utptr, 0, sizeof(*utptr));	/* keep searching */
	}
	(void) call_endutent();
#ifdef USE_UTMP_SETGID
	disableSetGid();
	TRACE_IDS;
#endif
    }
#else /* not USE_SYSV_UTMP */
    int wfd;
    struct utmp utmp;

    DEBUG_MSG("handle:Exit !USE_SYSV_UTMP\n");
    if (!resource.utmpInhibit && added_utmp_entry &&
	(am_slave < 0 && tslot > 0)) {
#if defined(USE_UTMP_SETGID)
	setEffectiveGroup(save_egid);
	TRACE_IDS;
#endif
	if ((wfd = open(etc_utmp, O_WRONLY)) >= 0) {
	    memset(&utmp, 0, sizeof(utmp));
	    lseek(wfd, (long) (tslot * sizeof(utmp)), 0);
	    IGNORE_RC(write(wfd, (char *) &utmp, sizeof(utmp)));
	    close(wfd);
	}
#ifdef WTMP
	if (xw->misc.login_shell &&
	    (wfd = open(etc_wtmp, O_WRONLY | O_APPEND)) >= 0) {
	    copy_filled(utmp.ut_line,
			my_pty_name(ttydev),
			sizeof(utmp.ut_line));
	    utmp.ut_time = time((time_t *) 0);
	    IGNORE_RC(write(wfd, (char *) &utmp, sizeof(utmp)));
	    close(wfd);
	}
#endif /* WTMP */
#ifdef USE_UTMP_SETGID
	disableSetGid();
	TRACE_IDS;
#endif
    }
#endif /* USE_SYSV_UTMP */
#endif /* HAVE_UTMP */

    cleanup_colored_cursor();

    /*
     * Flush pending data before releasing ownership, so nobody else can write
     * in the middle of the data.
     */
    ttyFlush(screen->respond);

#ifdef USE_PTY_SEARCH
    if (am_slave < 0) {
	TRACE_IDS;
	/* restore ownership of tty and pty */
	set_owner(ttydev, 0, 0, 0666U);
#if (defined(USE_PTY_DEVICE) && !defined(__sgi) && !defined(__hpux))
	set_owner(ptydev, 0, 0, 0666U);
#endif
    }
#endif

    /*
     * Close after releasing ownership to avoid race condition: other programs
     * grabbing it, and *then* having us release ownership....
     */
    close(screen->respond);	/* close explicitly to avoid race with slave side */
#ifdef ALLOWLOGGING
    if (screen->logging)
	CloseLog(xw);
#endif

    xtermPrintOnXError(xw, n);

#ifdef NO_LEAKS
    if (n == 0) {
	Display *dpy = TScreenOf(xw)->display;

	TRACE(("Freeing memory leaks\n"));

	if (toplevel) {
	    XtDestroyWidget(toplevel);
	    TRACE(("destroyed top-level widget\n"));
	}
	sortedOpts(NULL, NULL, 0);
	noleaks_charproc();
	noleaks_ptydata();
#if OPT_GRAPHICS
	noleaks_graphics(dpy);
#endif
#if OPT_WIDE_CHARS
	noleaks_CharacterClass();
#endif
	/* XrmSetDatabase(dpy, 0); increases leaks ;-) */
	XtCloseDisplay(dpy);
	XtDestroyApplicationContext(app_con);
	xtermCloseSession();
	TRACE(("closed display\n"));

	TRACE_CLOSE();
    }
#endif

    exit(n);
}

/* ARGSUSED */
static void
resize_termcap(XtermWidget xw)
{
    char *newtc = get_tcap_buffer(xw);

#ifndef USE_SYSV_ENVVARS
    if (!TEK4014_ACTIVE(xw) && newtc != NULL && *newtc) {
	TScreen *screen = TScreenOf(xw);
	char *ptr1, *ptr2;
	size_t i;
	int li_first = 0;
	char *temp;
	char oldtc[TERMCAP_SIZE];

	strcpy(oldtc, newtc);
	TRACE(("resize %s\n", oldtc));
	if ((ptr1 = x_strindex(oldtc, "co#")) == NULL) {
	    strcat(oldtc, "co#80:");
	    ptr1 = x_strindex(oldtc, "co#");
	}
	if ((ptr2 = x_strindex(oldtc, "li#")) == NULL) {
	    strcat(oldtc, "li#24:");
	    ptr2 = x_strindex(oldtc, "li#");
	}
	if (ptr1 > ptr2) {
	    li_first++;
	    temp = ptr1;
	    ptr1 = ptr2;
	    ptr2 = temp;
	}
	ptr1 += 3;
	ptr2 += 3;
	strncpy(newtc, oldtc, i = (size_t) (ptr1 - oldtc));
	if (i >= TERMCAP_SIZE - 10) {
	    TRACE(("...insufficient space: %lu\n", (unsigned long) i));
	    return;
	}
	temp = newtc + i;
	sprintf(temp, "%d", (li_first
			     ? MaxRows(screen)
			     : MaxCols(screen)));
	temp += strlen(temp);
	if ((ptr1 = strchr(ptr1, ':')) != NULL && (ptr1 < ptr2)) {
	    strncpy(temp, ptr1, i = (size_t) (ptr2 - ptr1));
	    temp += i;
	    sprintf(temp, "%d", (li_first
				 ? MaxCols(screen)
				 : MaxRows(screen)));
	    if ((ptr2 = strchr(ptr2, ':')) != NULL) {
		strcat(temp, ptr2);
	    }
	}
	TRACE(("   ==> %s\n", newtc));
	TRACE(("   new size %dx%d\n", MaxRows(screen), MaxCols(screen)));
    }
#endif /* USE_SYSV_ENVVARS */
}

/*
 * Does a non-blocking wait for a child process.  If the system
 * doesn't support non-blocking wait, do nothing.
 * Returns the pid of the child, or 0 or -1 if none or error.
 */
int
nonblocking_wait(void)
{
#ifdef USE_POSIX_WAIT
    pid_t pid;

    pid = waitpid(-1, NULL, WNOHANG);
#elif defined(USE_SYSV_SIGNALS) && (defined(CRAY) || !defined(SIGTSTP))
    /* cannot do non-blocking wait */
    int pid = 0;
#else /* defined(USE_SYSV_SIGNALS) && (defined(CRAY) || !defined(SIGTSTP)) */
#if defined(Lynx)
    int status;
#else
    union wait status;
#endif
    int pid;

    pid = wait3(&status, WNOHANG, (struct rusage *) NULL);
#endif /* USE_POSIX_WAIT else */
    return pid;
}

/* ARGSUSED */
static void
reapchild(int n GCC_UNUSED)
{
    int olderrno = errno;
    int pid;

    DEBUG_MSG("handle:reapchild\n");

    pid = wait(NULL);

#ifdef USE_SYSV_SIGNALS
    /* cannot re-enable signal before waiting for child
     * because then SVR4 loops.  Sigh.  HP-UX 9.01 too.
     */
    (void) signal(SIGCHLD, reapchild);
#endif

    do {
	if (pid == TScreenOf(term)->pid) {
	    DEBUG_MSG("Exiting\n");
	    if (hold_screen)
		caught_intr = True;
	    else
		need_cleanup = True;
	}
    } while ((pid = nonblocking_wait()) > 0);

    errno = olderrno;
}

static void
remove_termcap_entry(char *buf, const char *str)
{
    char *base = buf;
    char *first = base;
    int count = 0;
    size_t len = strlen(str);

    TRACE(("*** remove_termcap_entry('%s', '%s')\n", str, buf));

    while (*buf != 0) {
	if (!count && !strncmp(buf, str, len)) {
	    while (*buf != 0) {
		if (*buf == '\\')
		    buf++;
		else if (*buf == ':')
		    break;
		if (*buf != 0)
		    buf++;
	    }
	    while ((*first++ = *buf++) != 0) {
		;
	    }
	    TRACE(("...removed_termcap_entry('%s', '%s')\n", str, base));
	    return;
	} else if (*buf == '\\') {
	    buf++;
	} else if (*buf == ':') {
	    first = buf;
	    count = 0;
	} else if (!isspace(CharOf(*buf))) {
	    count++;
	}
	if (*buf != 0)
	    buf++;
    }
    TRACE(("...cannot remove\n"));
}

/*
 * parse_tty_modes accepts lines of the following form:
 *
 *         [SETTING] ...
 *
 * where setting consists of the words in the ttyModes[] array followed by a
 * character or ^char.
 */
static int
parse_tty_modes(char *s)
{
    int c;
    Cardinal j, k;
    int count = 0;
    Boolean found;

    TRACE(("parse_tty_modes\n"));
    for (;;) {
	size_t len;

	while (*s && isspace(CharOf(*s))) {
	    s++;
	}
	if (!*s) {
	    return count;
	}

	for (len = 0; s[len] && !isspace(CharOf(s[len])); ++len) {
	    ;
	}
	found = False;
	for (j = 0; j < XtNumber(ttyModes); ++j) {
	    if (len == ttyModes[j].len
		&& strncmp(s,
			   ttyModes[j].name,
			   ttyModes[j].len) == 0) {
		found = True;
		break;
	    }
	}
	if (!found) {
	    return -1;
	}

	s += ttyModes[j].len;
	while (*s && isspace(CharOf(*s))) {
	    s++;
	}

	/* check if this needs a parameter */
	found = False;
	for (k = 0, c = 0; k < XtNumber(ttyChars); ++k) {
	    if ((int) j == ttyChars[k].myMode) {
		if (ttyChars[k].sysMode < 0) {
		    found = True;
		    c = ttyChars[k].myDefault;
		}
		break;
	    }
	}

	if (!found) {
	    if (!*s
		|| (c = decode_keyvalue(&s, False)) == -1) {
		return -1;
	    }
	}
	ttyModes[j].value = c;
	ttyModes[j].set = 1;
	count++;
	TRACE(("...parsed #%d: %s=%#x\n", count, ttyModes[j].name, c));
    }
}

#ifndef GetBytesAvailable
int
GetBytesAvailable(Display *dpy)
{
    int fd = ConnectionNumber(dpy);
    int result;
#if defined(FIONREAD)
    ioctl(fd, FIONREAD, (char *) &result);
#elif defined(__CYGWIN__)
    fd_set set;
    struct timeval select_timeout =
    {0, 0};

    FD_ZERO(&set);
    FD_SET(fd, &set);
    result = (Select(fd + 1, &set, NULL, NULL, &select_timeout) > 0) ? 1 : 0;
#else
    struct pollfd pollfds[1];

    pollfds[0].fd = fd;
    pollfds[0].events = POLLIN;
    result = poll(pollfds, 1, 0);
#endif
    return result;
}
#endif /* !GetBytesAvailable */

/* Utility function to try to hide system differences from
   everybody who used to call killpg() */

int
kill_process_group(int pid, int sig)
{
    TRACE(("kill_process_group(pid=%d, sig=%d)\n", pid, sig));
#if defined(SVR4) || defined(SYSV) || !defined(X_NOT_POSIX)
    return kill(-pid, sig);
#else
    return killpg(pid, sig);
#endif
}

#if defined(__QNX__) && !defined(__QNXNTO__)
#include <sys/types.h>
#include <sys/proc_msg.h>
#include <sys/kernel.h>
#include <string.h>
#include <errno.h>

struct _proc_session ps;
struct _proc_session_reply rps;

int
qsetlogin(char *login, char *ttyname)
{
    int v = getsid(getpid());

    memset(&ps, 0, sizeof(ps));
    memset(&rps, 0, sizeof(rps));

    ps.type = _PROC_SESSION;
    ps.subtype = _PROC_SUB_ACTION1;
    ps.sid = v;
    strcpy(ps.name, login);

    Send(1, &ps, &rps, sizeof(ps), sizeof(rps));

    if (rps.status < 0)
	return (rps.status);

    ps.type = _PROC_SESSION;
    ps.subtype = _PROC_SUB_ACTION2;
    ps.sid = v;
    sprintf(ps.name, "//%d%s", getnid(), ttyname);
    Send(1, &ps, &rps, sizeof(ps), sizeof(rps));

    return (rps.status);
}
#endif

#ifdef __minix
int
setpgrp(void)
{
    return 0;
}

void
_longjmp(jmp_buf _env, int _val)
{
    longjmp(_env, _val);
}
#endif
