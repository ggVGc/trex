dnl $XTermId: aclocal.m4,v 1.542 2025/06/14 10:47:49 tom Exp $
dnl
dnl ---------------------------------------------------------------------------
dnl
dnl Copyright 1997-2024,2025 by Thomas E. Dickey
dnl
dnl                         All Rights Reserved
dnl
dnl Permission is hereby granted, free of charge, to any person obtaining a
dnl copy of this software and associated documentation files (the
dnl "Software"), to deal in the Software without restriction, including
dnl without limitation the rights to use, copy, modify, merge, publish,
dnl distribute, sublicense, and/or sell copies of the Software, and to
dnl permit persons to whom the Software is furnished to do so, subject to
dnl the following conditions:
dnl
dnl The above copyright notice and this permission notice shall be included
dnl in all copies or substantial portions of the Software.
dnl
dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
dnl OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
dnl MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
dnl IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
dnl CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
dnl TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
dnl SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
dnl
dnl Except as contained in this notice, the name(s) of the above copyright
dnl holders shall not be used in advertising or otherwise to promote the
dnl sale, use or other dealings in this Software without prior written
dnl authorization.
dnl
dnl ---------------------------------------------------------------------------
dnl See
dnl     https://invisible-island.net/autoconf/autoconf.html
dnl     https://invisible-island.net/autoconf/my-autoconf.html
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl AM_LANGINFO_CODESET version: 7 updated: 2023/01/11 04:05:23
dnl -------------------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl codeset.m4
dnl ====================
dnl serial AM1
dnl
dnl From Bruno Haible.
AC_DEFUN([AM_LANGINFO_CODESET],
[
AC_CACHE_CHECK([for nl_langinfo and CODESET], am_cv_langinfo_codeset,
	[AC_TRY_LINK([
$ac_includes_default
#include <langinfo.h>],
	[char* cs = nl_langinfo(CODESET); (void)cs],
	am_cv_langinfo_codeset=yes,
	am_cv_langinfo_codeset=no)
	])
	if test "$am_cv_langinfo_codeset" = yes; then
		AC_DEFINE(HAVE_LANGINFO_CODESET, 1,
		[Define if you have <langinfo.h> and nl_langinfo(CODESET).])
	fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ACVERSION_CHECK version: 5 updated: 2014/06/04 19:11:49
dnl ------------------
dnl Conditionally generate script according to whether we're using a given autoconf.
dnl
dnl $1 = version to compare against
dnl $2 = code to use if AC_ACVERSION is at least as high as $1.
dnl $3 = code to use if AC_ACVERSION is older than $1.
define([CF_ACVERSION_CHECK],
[
ifdef([AC_ACVERSION], ,[ifdef([AC_AUTOCONF_VERSION],[m4_copy([AC_AUTOCONF_VERSION],[AC_ACVERSION])],[m4_copy([m4_PACKAGE_VERSION],[AC_ACVERSION])])])dnl
ifdef([m4_version_compare],
[m4_if(m4_version_compare(m4_defn([AC_ACVERSION]), [$1]), -1, [$3], [$2])],
[CF_ACVERSION_COMPARE(
AC_PREREQ_CANON(AC_PREREQ_SPLIT([$1])),
AC_PREREQ_CANON(AC_PREREQ_SPLIT(AC_ACVERSION)), AC_ACVERSION, [$2], [$3])])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ACVERSION_COMPARE version: 3 updated: 2012/10/03 18:39:53
dnl --------------------
dnl CF_ACVERSION_COMPARE(MAJOR1, MINOR1, TERNARY1,
dnl                      MAJOR2, MINOR2, TERNARY2,
dnl                      PRINTABLE2, not FOUND, FOUND)
define([CF_ACVERSION_COMPARE],
[ifelse(builtin([eval], [$2 < $5]), 1,
[ifelse([$8], , ,[$8])],
[ifelse([$9], , ,[$9])])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_CFLAGS version: 15 updated: 2020/12/31 10:54:15
dnl -------------
dnl Copy non-preprocessor flags to $CFLAGS, preprocessor flags to $CPPFLAGS
dnl $1 = flags to add
dnl $2 = if given makes this macro verbose.
dnl
dnl Put any preprocessor definitions that use quoted strings in $EXTRA_CPPFLAGS,
dnl to simplify use of $CPPFLAGS in compiler checks, etc., that are easily
dnl confused by the quotes (which require backslashes to keep them usable).
AC_DEFUN([CF_ADD_CFLAGS],
[
cf_fix_cppflags=no
cf_new_cflags=
cf_new_cppflags=
cf_new_extra_cppflags=

for cf_add_cflags in $1
do
case "$cf_fix_cppflags" in
(no)
	case "$cf_add_cflags" in
	(-undef|-nostdinc*|-I*|-D*|-U*|-E|-P|-C)
		case "$cf_add_cflags" in
		(-D*)
			cf_tst_cflags=`echo "${cf_add_cflags}" |sed -e 's/^-D[[^=]]*='\''\"[[^"]]*//'`

			test "x${cf_add_cflags}" != "x${cf_tst_cflags}" \
				&& test -z "${cf_tst_cflags}" \
				&& cf_fix_cppflags=yes

			if test "$cf_fix_cppflags" = yes ; then
				CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)
				continue
			elif test "${cf_tst_cflags}" = "\"'" ; then
				CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)
				continue
			fi
			;;
		esac
		case "$CPPFLAGS" in
		(*$cf_add_cflags)
			;;
		(*)
			case "$cf_add_cflags" in
			(-D*)
				cf_tst_cppflags=`echo "x$cf_add_cflags" | sed -e 's/^...//' -e 's/=.*//'`
				CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,$cf_tst_cppflags)
				;;
			esac
			CF_APPEND_TEXT(cf_new_cppflags,$cf_add_cflags)
			;;
		esac
		;;
	(*)
		CF_APPEND_TEXT(cf_new_cflags,$cf_add_cflags)
		;;
	esac
	;;
(yes)
	CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)

	cf_tst_cflags=`echo "${cf_add_cflags}" |sed -e 's/^[[^"]]*"'\''//'`

	test "x${cf_add_cflags}" != "x${cf_tst_cflags}" \
		&& test -z "${cf_tst_cflags}" \
		&& cf_fix_cppflags=no
	;;
esac
done

if test -n "$cf_new_cflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$CFLAGS $cf_new_cflags)])
	CF_APPEND_TEXT(CFLAGS,$cf_new_cflags)
fi

if test -n "$cf_new_cppflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$CPPFLAGS $cf_new_cppflags)])
	CF_APPEND_TEXT(CPPFLAGS,$cf_new_cppflags)
fi

if test -n "$cf_new_extra_cppflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$EXTRA_CPPFLAGS $cf_new_extra_cppflags)])
	CF_APPEND_TEXT(EXTRA_CPPFLAGS,$cf_new_extra_cppflags)
fi

AC_SUBST(EXTRA_CPPFLAGS)

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIB version: 2 updated: 2010/06/02 05:03:05
dnl ----------
dnl Add a library, used to enforce consistency.
dnl
dnl $1 = library to add, without the "-l"
dnl $2 = variable to update (default $LIBS)
AC_DEFUN([CF_ADD_LIB],[CF_ADD_LIBS(-l$1,ifelse($2,,LIBS,[$2]))])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIBS version: 3 updated: 2019/11/02 16:47:33
dnl -----------
dnl Add one or more libraries, used to enforce consistency.  Libraries are
dnl prepended to an existing list, since their dependencies are assumed to
dnl already exist in the list.
dnl
dnl $1 = libraries to add, with the "-l", etc.
dnl $2 = variable to update (default $LIBS)
AC_DEFUN([CF_ADD_LIBS],[
cf_add_libs="[$]ifelse($2,,LIBS,[$2])"
# reverse order
cf_add_0lib=
for cf_add_1lib in $1; do cf_add_0lib="$cf_add_1lib $cf_add_0lib"; done
# filter duplicates
for cf_add_1lib in $cf_add_0lib; do
	for cf_add_2lib in $cf_add_libs; do
		if test "x$cf_add_1lib" = "x$cf_add_2lib"; then
			cf_add_1lib=
			break
		fi
	done
	test -n "$cf_add_1lib" && cf_add_libs="$cf_add_1lib $cf_add_libs"
done
ifelse($2,,LIBS,[$2])="$cf_add_libs"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIB_AFTER version: 4 updated: 2025/06/14 06:46:23
dnl ----------------
dnl Add a given library after another, e.g., following the one it satisfies a
dnl dependency for.
dnl
dnl $1 = the first library
dnl $2 = its dependency
dnl $3 = variable to update (default $LIBS)
AC_DEFUN([CF_ADD_LIB_AFTER],[
cf_add_libs="[$]ifelse($3,,LIBS,[$3])"
CF_VERBOSE(...before $cf_add_libs)
for cf_add_1lib in $2; do
	# filter duplicates
	cf_found_2lib=no
	for cf_add_2lib in $cf_add_libs; do
		if test "x$cf_add_1lib" = "x$cf_add_2lib"; then
			cf_found_2lib=yes
			break
		fi
	done
	# if not a duplicate, find the dependent library
	if test "$cf_found_2lib" = no
	then
		cf_found_2lib=no
		cf_add_2libs=
		for cf_add_2lib in $cf_add_libs
		do
			test -n "$cf_add_2libs" && cf_add_2libs="$cf_add_2libs "
			cf_add_2libs="$cf_add_2libs$cf_add_2lib"
			if test "x$cf_add_2lib" = "x$1"
			then
				cf_found_2lib=yes
				cf_add_2libs="$cf_add_2libs $cf_add_1lib"
			fi
		done
		if test "$cf_found_2lib" = yes
		then
			cf_add_libs="$cf_add_2libs"
		else
			CF_VERBOSE(...missed $1)
		fi
	fi
done
CF_VERBOSE(...after  $cf_add_libs)
ifelse($3,,LIBS,[$3])="$cf_add_libs"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_APPEND_CFLAGS version: 3 updated: 2021/09/05 17:25:40
dnl ----------------
dnl Use CF_ADD_CFLAGS after first checking for potential redefinitions.
dnl $1 = flags to add
dnl $2 = if given makes this macro verbose.
define([CF_APPEND_CFLAGS],
[
for cf_add_cflags in $1
do
	case "x$cf_add_cflags" in
	(x-[[DU]]*)
		CF_REMOVE_CFLAGS($cf_add_cflags,CFLAGS,[$2])
		CF_REMOVE_CFLAGS($cf_add_cflags,CPPFLAGS,[$2])
		;;
	esac
	CF_ADD_CFLAGS([$cf_add_cflags],[$2])
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_APPEND_TEXT version: 1 updated: 2017/02/25 18:58:55
dnl --------------
dnl use this macro for appending text without introducing an extra blank at
dnl the beginning
define([CF_APPEND_TEXT],
[
	test -n "[$]$1" && $1="[$]$1 "
	$1="[$]{$1}$2"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_DISABLE version: 3 updated: 1999/03/30 17:24:31
dnl --------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_ENABLE version: 3 updated: 1999/03/30 17:24:31
dnl -------------
dnl Allow user to enable a normally-off option.
AC_DEFUN([CF_ARG_ENABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],no)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_OPTION version: 5 updated: 2015/05/10 19:52:14
dnl -------------
dnl Restricted form of AC_ARG_ENABLE that ensures user doesn't give bogus
dnl values.
dnl
dnl Parameters:
dnl $1 = option name
dnl $2 = help-string
dnl $3 = action to perform if option is not default
dnl $4 = action if perform if option is default
dnl $5 = default option value (either 'yes' or 'no')
AC_DEFUN([CF_ARG_OPTION],
[AC_ARG_ENABLE([$1],[$2],[test "$enableval" != ifelse([$5],no,yes,no) && enableval=ifelse([$5],no,no,yes)
	if test "$enableval" != "$5" ; then
ifelse([$3],,[    :]dnl
,[    $3]) ifelse([$4],,,[
	else
		$4])
	fi],[enableval=$5 ifelse([$4],,,[
	$4
])dnl
])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_C11_NORETURN version: 4 updated: 2023/02/18 17:41:25
dnl ---------------
AC_DEFUN([CF_C11_NORETURN],
[
AC_MSG_CHECKING(if you want to use C11 _Noreturn feature)
CF_ARG_ENABLE(stdnoreturn,
	[  --enable-stdnoreturn    enable C11 _Noreturn feature for diagnostics],
	[enable_stdnoreturn=yes],
	[enable_stdnoreturn=no])
AC_MSG_RESULT($enable_stdnoreturn)

if test $enable_stdnoreturn = yes; then
AC_CACHE_CHECK([for C11 _Noreturn feature], cf_cv_c11_noreturn,
	[AC_TRY_COMPILE([
$ac_includes_default
#include <stdnoreturn.h>
static _Noreturn void giveup(void) { exit(0); }
	],
	[if (feof(stdin)) giveup()],
	cf_cv_c11_noreturn=yes,
	cf_cv_c11_noreturn=no)
	])
else
	cf_cv_c11_noreturn=no,
fi

if test "$cf_cv_c11_noreturn" = yes; then
	AC_DEFINE(HAVE_STDNORETURN_H, 1,[Define if <stdnoreturn.h> header is available and working])
	AC_DEFINE_UNQUOTED(STDC_NORETURN,_Noreturn,[Define if C11 _Noreturn keyword is supported])
	HAVE_STDNORETURN_H=1
else
	HAVE_STDNORETURN_H=0
fi

AC_SUBST(HAVE_STDNORETURN_H)
AC_SUBST(STDC_NORETURN)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CC_ENV_FLAGS version: 11 updated: 2023/02/20 11:15:46
dnl ---------------
dnl Check for user's environment-breakage by stuffing CFLAGS/CPPFLAGS content
dnl into CC.  This will not help with broken scripts that wrap the compiler
dnl with options, but eliminates a more common category of user confusion.
dnl
dnl In particular, it addresses the problem of being able to run the C
dnl preprocessor in a consistent manner.
dnl
dnl Caveat: this also disallows blanks in the pathname for the compiler, but
dnl the nuisance of having inconsistent settings for compiler and preprocessor
dnl outweighs that limitation.
AC_DEFUN([CF_CC_ENV_FLAGS],
[
# This should have been defined by AC_PROG_CC
: "${CC:=cc}"

AC_MSG_CHECKING(\$CFLAGS variable)
case "x$CFLAGS" in
(*-[[IUD]]*)
	AC_MSG_RESULT(broken)
	AC_MSG_WARN(your environment uses the CFLAGS variable to hold CPPFLAGS options)
	cf_flags="$CFLAGS"
	CFLAGS=
	for cf_arg in $cf_flags
	do
		CF_ADD_CFLAGS($cf_arg)
	done
	;;
(*)
	AC_MSG_RESULT(ok)
	;;
esac

AC_MSG_CHECKING(\$CC variable)
case "$CC" in
(*[[\ \	]]-*)
	AC_MSG_RESULT(broken)
	AC_MSG_WARN(your environment uses the CC variable to hold CFLAGS/CPPFLAGS options)
	# humor him...
	cf_prog=`echo "$CC" | sed -e 's/	/ /g' -e 's/[[ ]]* / /g' -e 's/[[ ]]*[[ ]]-[[^ ]].*//'`
	cf_flags=`echo "$CC" | sed -e "s%^$cf_prog%%"`
	CC="$cf_prog"
	for cf_arg in $cf_flags
	do
		case "x$cf_arg" in
		(x-[[IUDfgOW]]*)
			CF_ADD_CFLAGS($cf_arg)
			;;
		(*)
			CC="$CC $cf_arg"
			;;
		esac
	done
	CF_VERBOSE(resulting CC: '$CC')
	CF_VERBOSE(resulting CFLAGS: '$CFLAGS')
	CF_VERBOSE(resulting CPPFLAGS: '$CPPFLAGS')
	;;
(*)
	AC_MSG_RESULT(ok)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_CACHE version: 13 updated: 2020/12/31 10:54:15
dnl --------------
dnl Check if we're accidentally using a cache from a different machine.
dnl Derive the system name, as a check for reusing the autoconf cache.
dnl
dnl If we've packaged config.guess and config.sub, run that (since it does a
dnl better job than uname).  Normally we'll use AC_CANONICAL_HOST, but allow
dnl an extra parameter that we may override, e.g., for AC_CANONICAL_SYSTEM
dnl which is useful in cross-compiles.
dnl
dnl Note: we would use $ac_config_sub, but that is one of the places where
dnl autoconf 2.5x broke compatibility with autoconf 2.13
AC_DEFUN([CF_CHECK_CACHE],
[
if test -f "$srcdir/config.guess" || test -f "$ac_aux_dir/config.guess" ; then
	ifelse([$1],,[AC_CANONICAL_HOST],[$1])
	system_name="$host_os"
else
	system_name="`(uname -s -r) 2>/dev/null`"
	if test -z "$system_name" ; then
		system_name="`(hostname) 2>/dev/null`"
	fi
fi
test -n "$system_name" && AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name",[Define to the system name.])
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])

test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT(Configuring for $cf_cv_system_name)

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT(Cached system name ($system_name) does not agree with actual ($cf_cv_system_name))
	AC_MSG_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_CFLAGS version: 4 updated: 2021/01/02 19:22:58
dnl ---------------
dnl Conditionally add to $CFLAGS and $CPPFLAGS values which are derived from
dnl a build-configuration such as imake.  These have the pitfall that they
dnl often contain compiler-specific options which we cannot use, mixed with
dnl preprocessor options that we usually can.
AC_DEFUN([CF_CHECK_CFLAGS],
[
CF_VERBOSE(checking additions to CFLAGS)
cf_check_cflags="$CFLAGS"
cf_check_cppflags="$CPPFLAGS"
CF_ADD_CFLAGS($1,yes)
if test "x$cf_check_cflags" != "x$CFLAGS" ; then
AC_TRY_LINK([#include <stdio.h>],[printf("Hello world");],,
	[CF_VERBOSE(test-compile failed.  Undoing change to \$CFLAGS)
	 if test "x$cf_check_cppflags" != "x$CPPFLAGS" ; then
		 CF_VERBOSE(but keeping change to \$CPPFLAGS)
	 fi
	 CFLAGS="$cf_check_cflags"])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_ERRNO version: 14 updated: 2023/02/18 17:41:25
dnl --------------
dnl Check for data that is usually declared in <stdio.h> or <errno.h>, e.g.,
dnl the 'errno' variable.  Define a DECL_xxx symbol if we must declare it
dnl ourselves.
dnl
dnl $1 = the name to check
dnl $2 = the assumed type
AC_DEFUN([CF_CHECK_ERRNO],
[
AC_CACHE_CHECK(if external $1 is declared, cf_cv_dcl_$1,[
	AC_TRY_COMPILE([
$ac_includes_default
#include <errno.h> ],
	ifelse([$2],,int,[$2]) x = (ifelse([$2],,int,[$2])) $1; (void)x,
	[cf_cv_dcl_$1=yes],
	[cf_cv_dcl_$1=no])
])

if test "$cf_cv_dcl_$1" = no ; then
	CF_UPPER(cf_result,decl_$1)
	AC_DEFINE_UNQUOTED($cf_result)
fi

# It's possible (for near-UNIX clones) that the data doesn't exist
CF_CHECK_EXTERN_DATA($1,ifelse([$2],,int,[$2]))
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_EXTERN_DATA version: 5 updated: 2021/09/04 06:35:04
dnl --------------------
dnl Check for existence of external data in the current set of libraries.  If
dnl we can modify it, it is real enough.
dnl $1 = the name to check
dnl $2 = its type
AC_DEFUN([CF_CHECK_EXTERN_DATA],
[
AC_CACHE_CHECK(if external $1 exists, cf_cv_have_$1,[
	AC_TRY_LINK([
#undef $1
extern $2 $1;
],
	[$1 = 2],
	[cf_cv_have_$1=yes],
	[cf_cv_have_$1=no])
])

if test "$cf_cv_have_$1" = yes ; then
	CF_UPPER(cf_result,have_$1)
	AC_DEFINE_UNQUOTED($cf_result)
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CLANG_COMPILER version: 9 updated: 2023/02/18 17:41:25
dnl -----------------
dnl Check if the given compiler is really clang.  clang's C driver defines
dnl __GNUC__ (fooling the configure script into setting $GCC to yes) but does
dnl not ignore some gcc options.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = CLANG_COMPILER (default)
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_CLANG_COMPILER],[
ifelse([$2],,CLANG_COMPILER,[$2])=no

if test "$ifelse([$1],,[$1],GCC)" = yes ; then
	AC_MSG_CHECKING(if this is really Clang ifelse([$1],GXX,C++,C) compiler)
	cf_save_CFLAGS="$ifelse([$3],,CFLAGS,[$3])"
	AC_TRY_COMPILE([],[
#ifdef __clang__
#else
#error __clang__ is not defined
#endif
],[ifelse([$2],,CLANG_COMPILER,[$2])=yes
],[])
	ifelse([$3],,CFLAGS,[$3])="$cf_save_CFLAGS"
	AC_MSG_RESULT($ifelse([$2],,CLANG_COMPILER,[$2]))
fi

CLANG_VERSION=none

if test "x$ifelse([$2],,CLANG_COMPILER,[$2])" = "xyes" ; then
	case "$CC" in
	(c[[1-9]][[0-9]]|*/c[[1-9]][[0-9]])
		AC_MSG_WARN(replacing broken compiler alias $CC)
		CFLAGS="$CFLAGS -std=`echo "$CC" | sed -e 's%.*/%%'`"
		CC=clang
		;;
	esac

	AC_MSG_CHECKING(version of $CC)
	CLANG_VERSION="`$CC --version 2>/dev/null | sed -e '2,$d' -e 's/^.*(CLANG[[^)]]*) //' -e 's/^.*(Debian[[^)]]*) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$CLANG_VERSION" && CLANG_VERSION=unknown
	AC_MSG_RESULT($CLANG_VERSION)

	for cf_clang_opt in \
		-Qunused-arguments \
		-Wno-error=implicit-function-declaration
	do
		AC_MSG_CHECKING(if option $cf_clang_opt works)
		cf_save_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $cf_clang_opt"
		AC_TRY_LINK([
			#include <stdio.h>],[
			printf("hello!\\n");],[
			cf_clang_optok=yes],[
			cf_clang_optok=no])
		AC_MSG_RESULT($cf_clang_optok)
		CFLAGS="$cf_save_CFLAGS"
		if test "$cf_clang_optok" = yes; then
			CF_VERBOSE(adding option $cf_clang_opt)
			CF_APPEND_TEXT(CFLAGS,$cf_clang_opt)
		fi
	done
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_CONST_X_STRING version: 9 updated: 2024/12/04 03:49:57
dnl -----------------
dnl The X11R4-X11R6 Xt specification uses an ambiguous String type for most
dnl character-strings.
dnl
dnl It is ambiguous because the specification accommodated the pre-ANSI
dnl compilers bundled by more than one vendor in lieu of providing a standard C
dnl compiler other than by costly add-ons.  Because of this, the specification
dnl did not take into account the use of const for telling the compiler that
dnl string literals would be in readonly memory.
dnl
dnl As a workaround, one could (starting with X11R5) define XTSTRINGDEFINES, to
dnl let the compiler decide how to represent Xt's strings which were #define'd.
dnl That does not solve the problem of using the block of Xt's strings which
dnl are compiled into the library (and is less efficient than one might want).
dnl
dnl Xt specification 7 introduces the _CONST_X_STRING symbol which is used both
dnl when compiling the library and compiling using the library, to tell the
dnl compiler that String is const.
AC_DEFUN([CF_CONST_X_STRING],
[
AC_REQUIRE([AC_PATH_XTRA])

CF_SAVE_XTRA_FLAGS([CF_CONST_X_STRING])

AC_TRY_COMPILE(
[
$ac_includes_default
#include <X11/Intrinsic.h>
],
[String foo = malloc(1); free((void*)foo)],[

AC_CACHE_CHECK(for X11/Xt const-feature,cf_cv_const_x_string,[
	AC_TRY_COMPILE(
		[
#undef  _CONST_X_STRING
#define _CONST_X_STRING	/* X11R7.8 (perhaps) */
#undef  XTSTRINGDEFINES	/* X11R5 and later */
$ac_includes_default
#include <X11/Intrinsic.h>
		],[String foo = malloc(1); *foo = 0],[
			cf_cv_const_x_string=no
		],[
			cf_cv_const_x_string=yes
		])
])

CF_RESTORE_XTRA_FLAGS([CF_CONST_X_STRING])

case "$cf_cv_const_x_string" in
(no)
	CF_APPEND_TEXT(CPPFLAGS,-DXTSTRINGDEFINES)
	;;
(*)
	CF_APPEND_TEXT(CPPFLAGS,-D_CONST_X_STRING)
	;;
esac

])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_DESKTOP version: 2 updated: 2011/04/22 05:17:37
dnl ------------------
dnl Handle a configure option "--disable-desktop", which sets a shell
dnl variable $desktop_utils to a "#" if the feature is not wanted, or to an
dnl empty string if enabled.  The variable is used to substitute in front of
dnl corresponding makefile-rules.
dnl
dnl It also tells the configure script to substitute the environment variable
dnl $DESKTOP_FLAGS, which can be used by external scripts to customize the
dnl invocation of desktop-file-util.
dnl
dnl $1 = program name
AC_DEFUN([CF_DISABLE_DESKTOP],[
# Comment-out the install-desktop rule if the desktop-utils are not found.
AC_MSG_CHECKING(if you want to install desktop files)
CF_ARG_OPTION(desktop,
	[  --disable-desktop       disable install of $1 desktop files],
	[enable_desktop=$enableval],
	[enable_desktop=$enableval],yes)
AC_MSG_RESULT($enable_desktop)

desktop_utils=
if test "$enable_desktop" = yes ; then
AC_CHECK_PROG(desktop_utils,desktop-file-install,yes,no)
fi

test "$desktop_utils" = yes && desktop_utils= || desktop_utils="#"
AC_SUBST(DESKTOP_FLAGS)
])
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_ECHO version: 14 updated: 2021/09/04 06:35:04
dnl ---------------
dnl You can always use "make -n" to see the actual options, but it is hard to
dnl pick out/analyze warning messages when the compile-line is long.
dnl
dnl Sets:
dnl	ECHO_LT - symbol to control if libtool is verbose
dnl	ECHO_LD - symbol to prefix "cc -o" lines
dnl	RULE_CC - symbol to put before implicit "cc -c" lines (e.g., .c.o)
dnl	SHOW_CC - symbol to put before explicit "cc -c" lines
dnl	ECHO_CC - symbol to put before any "cc" line
dnl
AC_DEFUN([CF_DISABLE_ECHO],[
AC_MSG_CHECKING(if you want to see long compiling messages)
CF_ARG_DISABLE(echo,
	[  --disable-echo          do not display "compiling" commands],
	[
	ECHO_LT='--silent'
	ECHO_LD='@echo linking [$]@;'
	RULE_CC='@echo compiling [$]<'
	SHOW_CC='@echo compiling [$]@'
	ECHO_CC='@'
],[
	ECHO_LT=''
	ECHO_LD=''
	RULE_CC=''
	SHOW_CC=''
	ECHO_CC=''
])
AC_MSG_RESULT($enableval)
AC_SUBST(ECHO_LT)
AC_SUBST(ECHO_LD)
AC_SUBST(RULE_CC)
AC_SUBST(SHOW_CC)
AC_SUBST(ECHO_CC)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_LEAKS version: 9 updated: 2021/04/03 16:41:50
dnl ----------------
dnl Combine no-leak checks with the libraries or tools that are used for the
dnl checks.
AC_DEFUN([CF_DISABLE_LEAKS],[

AC_REQUIRE([CF_WITH_DMALLOC])
AC_REQUIRE([CF_WITH_DBMALLOC])
AC_REQUIRE([CF_WITH_VALGRIND])

AC_MSG_CHECKING(if you want to perform memory-leak testing)
AC_ARG_ENABLE(leaks,
	[  --disable-leaks         test: free permanent memory, analyze leaks],
	[enable_leaks=$enableval],
	[enable_leaks=yes])
dnl with_no_leaks is more readable...
if test "x$enable_leaks" = xno; then with_no_leaks=yes; else with_no_leaks=no; fi
AC_MSG_RESULT($with_no_leaks)

if test "$enable_leaks" = no ; then
	AC_DEFINE(NO_LEAKS,1,[Define to 1 if you want to perform memory-leak testing.])
	AC_DEFINE(YY_NO_LEAKS,1,[Define to 1 if you want to perform memory-leak testing.])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_RPATH_HACK version: 3 updated: 2021/01/05 20:14:44
dnl ---------------------
dnl The rpath-hack makes it simpler to build programs, particularly with the
dnl *BSD ports which may have essential libraries in unusual places.  But it
dnl can interfere with building an executable for the base system.  Use this
dnl option in that case.
AC_DEFUN([CF_DISABLE_RPATH_HACK],
[
AC_MSG_CHECKING(if rpath-hack should be disabled)
CF_ARG_DISABLE(rpath-hack,
	[  --disable-rpath-hack    don't add rpath options for additional libraries],
	[enable_rpath_hack=no],
	[enable_rpath_hack=yes])
dnl TODO - drop cf_disable_rpath_hack
if test "x$enable_rpath_hack" = xno; then cf_disable_rpath_hack=yes; else cf_disable_rpath_hack=no; fi
AC_MSG_RESULT($cf_disable_rpath_hack)

if test "$enable_rpath_hack" = yes ; then
	CF_RPATH_HACK
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_ENABLE_NARROWPROTO version: 6 updated: 2020/12/31 18:40:20
dnl ---------------------
dnl If this is not set properly, Xaw's scrollbars will not work.
dnl The so-called "modular" configuration for X.org omits most of the
dnl configure checks that would be needed to provide compatibility with
dnl older X builds.  This one breaks things noticeably.
AC_DEFUN([CF_ENABLE_NARROWPROTO],
[
AC_MSG_CHECKING(if you want narrow prototypes for X libraries)

case `$ac_config_guess` in
(*freebsd*|*gnu*|*irix5*|*irix6*|*netbsd*|*openbsd*|*qnx*|*sco*|*sgi*)
	cf_default_narrowproto=yes
	;;
(*)
	cf_default_narrowproto=no
	;;
esac

CF_ARG_OPTION(narrowproto,
	[  --enable-narrowproto    enable narrow prototypes for X libraries],
	[enable_narrowproto=$enableval],
	[enable_narrowproto=$cf_default_narrowproto],
	[$cf_default_narrowproto])
AC_MSG_RESULT($enable_narrowproto)
])
dnl ---------------------------------------------------------------------------
dnl CF_ENABLE_WARNINGS version: 9 updated: 2021/01/05 19:40:50
dnl ------------------
dnl Configure-option to enable gcc warnings
dnl
dnl $1 = extra options to add, if supported
dnl $2 = option for checking attributes.  By default, this is done when
dnl      warnings are enabled.  For other values:
dnl      yes: always do this, e.g., to use in generated library-headers
dnl      no: never do this
AC_DEFUN([CF_ENABLE_WARNINGS],[
if test "$GCC" = yes || test "$GXX" = yes
then
CF_FIX_WARNINGS(CFLAGS)
CF_FIX_WARNINGS(CPPFLAGS)
CF_FIX_WARNINGS(LDFLAGS)
AC_MSG_CHECKING(if you want to turn on gcc warnings)
CF_ARG_ENABLE(warnings,
	[  --enable-warnings       test: turn on gcc compiler warnings],
	[enable_warnings=yes],
	[enable_warnings=no])
AC_MSG_RESULT($enable_warnings)
if test "$enable_warnings" = "yes"
then
	ifelse($2,,[CF_GCC_ATTRIBUTES])
	CF_GCC_WARNINGS($1)
fi
ifelse($2,yes,[CF_GCC_ATTRIBUTES])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ERRNO version: 5 updated: 1997/11/30 12:44:39
dnl --------
dnl Check if 'errno' is declared in <errno.h>
AC_DEFUN([CF_ERRNO],
[
CF_CHECK_ERRNO(errno)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIX_WARNINGS version: 4 updated: 2021/12/16 18:22:31
dnl ---------------
dnl Warning flags do not belong in CFLAGS, CPPFLAGS, etc.  Any of gcc's
dnl "-Werror" flags can interfere with configure-checks.  Those go into
dnl EXTRA_CFLAGS.
dnl
dnl $1 = variable name to repair
define([CF_FIX_WARNINGS],[
if test "$GCC" = yes || test "$GXX" = yes
then
	case [$]$1 in
	(*-Werror=*)
		cf_temp_flags=
		for cf_temp_scan in [$]$1
		do
			case "x$cf_temp_scan" in
			(x-Werror=format*)
				CF_APPEND_TEXT(cf_temp_flags,$cf_temp_scan)
				;;
			(x-Werror=*)
				CF_APPEND_TEXT(EXTRA_CFLAGS,$cf_temp_scan)
				;;
			(*)
				CF_APPEND_TEXT(cf_temp_flags,$cf_temp_scan)
				;;
			esac
		done
		if test "x[$]$1" != "x$cf_temp_flags"
		then
			CF_VERBOSE(repairing $1: [$]$1)
			$1="$cf_temp_flags"
			CF_VERBOSE(... fixed [$]$1)
			CF_VERBOSE(... extra $EXTRA_CFLAGS)
		fi
		;;
	esac
fi
AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_GETTIME version: 3 updated: 2024/05/11 13:40:02
dnl ---------------
dnl Check for gettimeofday or clock_gettime.  In 2023, the former is still more
dnl widely supported, but "deprecated" (2008), so we will use the latter if it
dnl is available, to reduce compiler warnings.
AC_DEFUN([CF_FUNC_GETTIME],[
cf_save_libs="$LIBS"
AC_CHECK_FUNC(clock_gettime,
	cf_cv_test_clock_gettime=yes,
	AC_CHECK_LIB(rt, clock_gettime,
		[LIBS="-lrt $LIBS"
		 cf_cv_test_clock_gettime=yes],
		 cf_cv_test_clock_gettime=no))

if test "$cf_cv_test_clock_gettime" = yes ; then
AC_CACHE_CHECK(if clock_gettime links,cf_cv_func_clock_gettime,[
		AC_TRY_LINK([
$ac_includes_default
#include <time.h>
		],
		[struct timespec ts;
		int rc = clock_gettime(CLOCK_REALTIME, &ts)
			   + clock_gettime(CLOCK_MONOTONIC, &ts);
		 (void) rc; (void)ts],
		[cf_cv_func_clock_gettime=yes],
		[cf_cv_func_clock_gettime=no])
])
else
	cf_cv_func_clock_gettime=no
fi

if test "$cf_cv_func_clock_gettime" = yes
then
	AC_DEFINE(HAVE_CLOCK_GETTIME,1,[Define to 1 if we have clock_gettime function])
else
AC_CHECK_FUNC(gettimeofday,
	AC_DEFINE(HAVE_GETTIMEOFDAY,1,[Define to 1 if we have gettimeofday function]),[

AC_CHECK_LIB(bsd, gettimeofday,
	AC_DEFINE(HAVE_GETTIMEOFDAY,1,[Define to 1 if we have gettimeofday function])
	CF_ADD_LIB(bsd))])dnl CLIX: bzero, select, gettimeofday
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_GRANTPT version: 15 updated: 2020/12/31 18:40:20
dnl ---------------
dnl Check for grantpt versus openpty, as well as functions that "should" be
dnl available if grantpt is available.
AC_DEFUN([CF_FUNC_GRANTPT],[

AC_CHECK_HEADERS( \
stropts.h \
)

cf_func_grantpt="grantpt ptsname"
cf_prefer_openpt=no
case $host_os in
(darwin[[0-9]].*)
	;;
(openbsd[[0-9]].*)
	# The POSIX entrypoints exist, but have never worked.
	;;
(linux*)
	cf_func_grantpt="$cf_func_grantpt posix_openpt"
	cf_prefer_openpt=yes
	;;
(*)
	cf_func_grantpt="$cf_func_grantpt posix_openpt"
	;;
esac

AC_CHECK_FUNCS($cf_func_grantpt)

cf_grantpt_opts=
if test "x$ac_cv_func_grantpt" = "xyes" ; then
	AC_MSG_CHECKING(if grantpt really works)
	AC_TRY_LINK(CF__GRANTPT_HEAD,CF__GRANTPT_BODY,[
	AC_TRY_RUN(CF__GRANTPT_HEAD
int main(void)
{
CF__GRANTPT_BODY
}
,
,ac_cv_func_grantpt=no
,ac_cv_func_grantpt=maybe)
	],ac_cv_func_grantpt=no)
	AC_MSG_RESULT($ac_cv_func_grantpt)

	if test "x$ac_cv_func_grantpt" != "xno" ; then

		if test "x$ac_cv_func_grantpt" = "xyes" ; then
			AC_MSG_CHECKING(for pty features)
dnl if we have no stropts.h, skip the checks for streams modules
			if test "x$ac_cv_header_stropts_h" = xyes
			then
				cf_pty_this=0
			else
				cf_pty_this=3
			fi

			cf_pty_defines=
			while test $cf_pty_this != 6
			do

				cf_pty_feature=
				cf_pty_next="`expr $cf_pty_this + 1`"
				CF_MSG_LOG(pty feature test $cf_pty_next:5)
				AC_TRY_RUN(#define CONFTEST $cf_pty_this
$cf_pty_defines
CF__GRANTPT_HEAD
int main(void)
{
CF__GRANTPT_BODY
}
,
[
				case $cf_pty_next in
				(1) # - streams
					cf_pty_feature=ptem
					;;
				(2) # - streams
					cf_pty_feature=ldterm
					;;
				(3) # - streams
					cf_pty_feature=ttcompat
					;;
				(4)
					cf_pty_feature=pty_isatty
					;;
				(5)
					cf_pty_feature=pty_tcsetattr
					;;
				(6)
					cf_pty_feature=tty_tcsetattr
					;;
				esac
],[
				case $cf_pty_next in
				(1|2|3)
					CF_MSG_LOG(skipping remaining streams features $cf_pty_this..2)
					cf_pty_next=3
					;;
				esac
])
				if test -n "$cf_pty_feature"
				then
					cf_pty_defines="$cf_pty_defines
#define CONFTEST_$cf_pty_feature 1
"
					cf_grantpt_opts="$cf_grantpt_opts $cf_pty_feature"
				fi

				cf_pty_this=$cf_pty_next
			done
			AC_MSG_RESULT($cf_grantpt_opts)
			cf_grantpt_opts=`echo "$cf_grantpt_opts" | sed -e 's/ isatty//'`
		fi
	fi
fi

dnl If we found grantpt, but no features, e.g., for streams or if we are not
dnl able to use tcsetattr, then give openpty a try.  In particular, Darwin 10.7
dnl has a more functional openpty than posix_openpt.
dnl
dnl There is no configure run-test for openpty, since older implementations do
dnl not always run properly as a non-root user.  For that reason, we also allow
dnl the configure script to suppress this check entirely with $disable_openpty.
if test "x$cf_prefer_posix_openpt" = "xyes" && test "x$ac_cv_func_posix_openpt" = "xyes" ; then
	CF_VERBOSE(prefer posix_openpt over openpty)
elif test "x$disable_openpty" != "xyes" || test -z "$cf_grantpt_opts" ; then
	AC_CHECK_LIB(util, openpty, [cf_have_openpty=yes],[cf_have_openpty=no])
	if test "$cf_have_openpty" = yes ; then
		ac_cv_func_grantpt=no
		LIBS="-lutil $LIBS"
		AC_DEFINE(HAVE_OPENPTY,1,[Define to 1 if you have the openpty function])
		AC_CHECK_HEADERS( \
			util.h \
			libutil.h \
			pty.h \
		)
	fi
fi

dnl If we did not settle on using openpty, fill in the definitions for grantpt.
if test "x$ac_cv_func_grantpt" != xno
then
	CF_VERBOSE(will rely upon grantpt)
	AC_DEFINE(HAVE_WORKING_GRANTPT,1,[Define to 1 if the grantpt function seems to work])
	for cf_feature in $cf_grantpt_opts
	do
		cf_feature=`echo "$cf_feature" | sed -e 's/ //g'`
		CF_UPPER(cf_FEATURE,$cf_feature)
		AC_DEFINE_UNQUOTED(HAVE_GRANTPT_$cf_FEATURE)
	done
elif test "x$cf_have_openpty" = xno
then
	CF_VERBOSE(will rely upon BSD-pseudoterminals)
else
	CF_VERBOSE(will rely upon openpty)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_STRFTIME version: 2 updated: 2023/12/01 20:44:51
dnl ----------------
AC_DEFUN([CF_FUNC_STRFTIME],
[
AC_CACHE_CHECK(for strftime function,cf_cv_func_strftime,[
AC_TRY_LINK([
$ac_includes_default
#include <time.h>
],[
	time_t now = time((time_t*)0);
	struct tm *tm = localtime(&now);
	char buffer[80];
	size_t result = strftime(buffer, sizeof(buffer), "%c", tm);

	(void)result;
	(void)buffer;
],[cf_cv_func_strftime=yes],[cf_cv_func_strftime=no])
])

test "$cf_cv_func_strftime" = yes && AC_DEFINE(HAVE_STRFTIME,1,[Define to 1 to indicate that strftime function is present])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_TGETENT version: 27 updated: 2023/12/01 17:22:50
dnl ---------------
dnl Check for tgetent function in termcap library.  If we cannot find this,
dnl we'll use the $LINES and $COLUMNS environment variables to pass screen
dnl size information to subprocesses.  (We cannot use terminfo's compatibility
dnl function, since it cannot provide the termcap-format data).
dnl
dnl If the --disable-full-tgetent option is given, we'll settle for the first
dnl tgetent function we find.  Since the search list in that case does not
dnl include the termcap library, that allows us to default to terminfo.
AC_DEFUN([CF_FUNC_TGETENT],
[
# compute a reasonable value for $TERM to give tgetent(), since we may be
# running in 'screen', which sets $TERMCAP to a specific entry that is not
# necessarily in /etc/termcap - unsetenv is not portable, so we cannot simply
# discard $TERMCAP.
cf_TERMVAR=vt100
if test -n "$TERMCAP"
then
	cf_TERMCAP=`echo "$TERMCAP" | tr '\n' ' ' | sed -e 's/^..|//' -e 's/|.*//'`
	case "$cf_TERMCAP" in
	(screen*.*)
		;;
	(*)
		cf_TERMVAR="$cf_TERMCAP"
		;;
	esac
fi
test -z "$cf_TERMVAR" && cf_TERMVAR=vt100

# BSD termcap used no header file
# SVr4 provided termcap prototypes as a legacy feature in term.h
# GNU termcap provided termcap prototypes in termcap.h
# ncurses provides termcap prototypes in both term.h and termcap.h
#
# The terminfo-based termcap interfaces do not provide a full tgetent (i.e., do
# not return the text of the termcap entry in the buffer), but as a special
# case, FreeBSD provides ncurses' termcap.h with a modified termcap reader that
# returns the termcap text.
AC_CHECK_HEADERS(termcap.h)

cf_termcap_h="\
#ifdef HAVE_TERMCAP_H
#include <termcap.h>
#else
extern int tgetent(char *, const char *);
#endif"

AC_MSG_CHECKING(if we want full tgetent function)
CF_ARG_DISABLE(full-tgetent,
	[  --disable-full-tgetent  disable check for full tgetent function],
	cf_full_tgetent=no,
	cf_full_tgetent=yes,yes)
AC_MSG_RESULT($cf_full_tgetent)

if test "$cf_full_tgetent" = yes ; then
	cf_test_message="full tgetent"
else
	cf_test_message="tgetent"
fi

AC_CACHE_CHECK(for $cf_test_message function,cf_cv_lib_tgetent,[
cf_save_LIBS="$LIBS"
cf_cv_lib_tgetent=no
cf_TERMLIB="termlib ncursesw ncurses curses"
if test "$cf_full_tgetent" = yes ; then
	cf_TERMLIB="otermcap termcap $cf_TERMLIB"
	cf_TERMTST="buffer[[0]] == 0"
else
	cf_TERMLIB="tinfow tinfo $cf_TERMLIB"
	cf_TERMTST="0"
fi
for cf_termlib in '' $cf_TERMLIB ; do
	LIBS="$cf_save_LIBS"
	test -n "$cf_termlib" && { CF_ADD_LIB($cf_termlib) }
	AC_TRY_RUN([
$cf_termcap_h

/* terminfo implementations ignore the buffer argument, making it useless for
 * the xterm application, which uses this information to make a new TERMCAP
 * environment variable.
 */
int main(void)
{
	char buffer[1024];
	buffer[0] = 0;
	tgetent(buffer, "$cf_TERMVAR");
	${cf_cv_main_return:-return} ($cf_TERMTST); }],
	[echo "yes, there is a termcap/tgetent in $cf_termlib" 1>&AC_FD_CC
	 if test -n "$cf_termlib" ; then
	 	cf_cv_lib_tgetent="-l$cf_termlib"
	 else
	 	cf_cv_lib_tgetent=yes
	 fi
	 break],
	[echo "no, there is no termcap/tgetent in $cf_termlib" 1>&AC_FD_CC],
	[echo "cross-compiling, cannot verify if a termcap/tgetent is present in $cf_termlib" 1>&AC_FD_CC])
done
LIBS="$cf_save_LIBS"
])

# If we found a working tgetent(), set LIBS and check for termcap.h.
# (LIBS cannot be set inside AC_CACHE_CHECK; the commands there should
# not have side effects other than setting the cache variable, because
# they are not executed when a cached value exists.)
if test "x$cf_cv_lib_tgetent" != xno ; then
	test "x$cf_cv_lib_tgetent" != xyes && { CF_ADD_LIBS($cf_cv_lib_tgetent) }
	AC_DEFINE(USE_TERMCAP,1,[Define 1 to indicate that working tgetent is found])
	if test "$cf_full_tgetent" = no ; then
		AC_TRY_COMPILE([
#include <termcap.h>],[
#ifdef NCURSES_VERSION
#error do not use ncurses termcap.h
#endif],[AC_DEFINE(HAVE_TERMCAP_H)])
	fi
else
        # If we didn't find a tgetent() that supports the buffer
        # argument, look again to see whether we can find even
        # a crippled one.  A crippled tgetent() is still useful to
        # validate values for the TERM environment variable given to
        # child processes.
	AC_CACHE_CHECK(for partial tgetent function,cf_cv_lib_part_tgetent,[
	cf_cv_lib_part_tgetent=no
	for cf_termlib in $cf_TERMLIB ; do
		LIBS="$cf_save_LIBS -l$cf_termlib"
		AC_TRY_LINK([$cf_termcap_h],[tgetent(0, "$cf_TERMVAR")],
			[echo "there is a terminfo/tgetent in $cf_termlib" 1>&AC_FD_CC
			 cf_cv_lib_part_tgetent="-l$cf_termlib"
			 break])
	done
	LIBS="$cf_save_LIBS"
	])

	if test "$cf_cv_lib_part_tgetent" != no ; then
		CF_ADD_LIBS($cf_cv_lib_part_tgetent)
                # If this is linking against ncurses, we'll trigger the
                # ifdef in resize.c that turns the termcap stuff back off.
		AC_DEFINE(USE_TERMINFO,1,[Define to 1 to indicate that terminfo provides the tgetent interface])
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_ATTRIBUTES version: 24 updated: 2021/03/20 12:00:25
dnl -----------------
dnl Test for availability of useful gcc __attribute__ directives to quiet
dnl compiler warnings.  Though useful, not all are supported -- and contrary
dnl to documentation, unrecognized directives cause older compilers to barf.
AC_DEFUN([CF_GCC_ATTRIBUTES],
[AC_REQUIRE([AC_PROG_FGREP])dnl
AC_REQUIRE([CF_C11_NORETURN])dnl

if test "$GCC" = yes || test "$GXX" = yes
then
cat > conftest.i <<EOF
#ifndef GCC_PRINTF
#define GCC_PRINTF 0
#endif
#ifndef GCC_SCANF
#define GCC_SCANF 0
#endif
#ifndef GCC_NORETURN
#define GCC_NORETURN /* nothing */
#endif
#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif
EOF
if test "$GCC" = yes
then
	AC_CHECKING([for $CC __attribute__ directives])
cat > "conftest.$ac_ext" <<EOF
#line __oline__ "${as_me:-configure}"
#include <stdio.h>
#include "confdefs.h"
#include "conftest.h"
#include "conftest.i"
#if	GCC_PRINTF
#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#else
#define GCC_PRINTFLIKE(fmt,var) /*nothing*/
#endif
#if	GCC_SCANF
#define GCC_SCANFLIKE(fmt,var)  __attribute__((format(scanf,fmt,var)))
#else
#define GCC_SCANFLIKE(fmt,var)  /*nothing*/
#endif
extern void wow(char *,...) GCC_SCANFLIKE(1,2);
extern GCC_NORETURN void oops(char *,...) GCC_PRINTFLIKE(1,2);
extern GCC_NORETURN void foo(void);
int main(int argc GCC_UNUSED, char *argv[[]] GCC_UNUSED) { (void)argc; (void)argv; return 0; }
EOF
	cf_printf_attribute=no
	cf_scanf_attribute=no
	for cf_attribute in scanf printf unused noreturn
	do
		CF_UPPER(cf_ATTRIBUTE,$cf_attribute)
		cf_directive="__attribute__(($cf_attribute))"
		echo "checking for $CC $cf_directive" 1>&AC_FD_CC

		case "$cf_attribute" in
		(printf)
			cf_printf_attribute=yes
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
			;;
		(scanf)
			cf_scanf_attribute=yes
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
			;;
		(*)
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE $cf_directive
EOF
			;;
		esac

		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... $cf_attribute)
			cat conftest.h >>confdefs.h
			case "$cf_attribute" in
			(noreturn)
				AC_DEFINE_UNQUOTED(GCC_NORETURN,$cf_directive,[Define to noreturn-attribute for gcc])
				;;
			(printf)
				cf_value='/* nothing */'
				if test "$cf_printf_attribute" != no ; then
					cf_value='__attribute__((format(printf,fmt,var)))'
					AC_DEFINE(GCC_PRINTF,1,[Define to 1 if the compiler supports gcc-like printf attribute.])
				fi
				AC_DEFINE_UNQUOTED(GCC_PRINTFLIKE(fmt,var),$cf_value,[Define to printf-attribute for gcc])
				;;
			(scanf)
				cf_value='/* nothing */'
				if test "$cf_scanf_attribute" != no ; then
					cf_value='__attribute__((format(scanf,fmt,var)))'
					AC_DEFINE(GCC_SCANF,1,[Define to 1 if the compiler supports gcc-like scanf attribute.])
				fi
				AC_DEFINE_UNQUOTED(GCC_SCANFLIKE(fmt,var),$cf_value,[Define to sscanf-attribute for gcc])
				;;
			(unused)
				AC_DEFINE_UNQUOTED(GCC_UNUSED,$cf_directive,[Define to unused-attribute for gcc])
				;;
			esac
		fi
	done
else
	${FGREP-fgrep} define conftest.i >>confdefs.h
fi
rm -rf ./conftest*
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_VERSION version: 9 updated: 2023/03/05 14:30:13
dnl --------------
dnl Find version of gcc, and (because icc/clang pretend to be gcc without being
dnl compatible), attempt to determine if icc/clang is actually used.
AC_DEFUN([CF_GCC_VERSION],[
AC_REQUIRE([AC_PROG_CC])
GCC_VERSION=none
if test "$GCC" = yes ; then
	AC_MSG_CHECKING(version of $CC)
	GCC_VERSION="`${CC} --version 2>/dev/null | sed -e '2,$d' -e 's/^[[^(]]*([[^)]][[^)]]*) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$GCC_VERSION" && GCC_VERSION=unknown
	AC_MSG_RESULT($GCC_VERSION)
fi
CF_INTEL_COMPILER(GCC,INTEL_COMPILER,CFLAGS)
CF_CLANG_COMPILER(GCC,CLANG_COMPILER,CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_WARNINGS version: 43 updated: 2024/12/21 08:44:12
dnl ---------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Winline (usually not worthwhile)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-Wwrite-strings (too noisy, but should review occasionally).  This
dnl		is enabled for ncurses using "--enable-const".
dnl	-pedantic
dnl
dnl Parameter:
dnl	$1 is an optional list of gcc warning flags that a particular
dnl		application might want to use, e.g., "no-unused" for
dnl		-Wno-unused
dnl Special:
dnl	If $with_ext_const is "yes", add a check for -Wwrite-strings
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[
AC_REQUIRE([CF_GCC_VERSION])
if test "x$have_x" = xyes; then CF_CONST_X_STRING fi
cat > "conftest.$ac_ext" <<EOF
#line __oline__ "${as_me:-configure}"
int main(int argc, char *argv[[]]) { return (argv[[argc-1]] == 0) ; }
EOF
if test "$INTEL_COMPILER" = yes
then
# The "-wdXXX" options suppress warnings:
# remark #1419: external declaration in primary source file
# remark #1683: explicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1684: conversion from pointer to same-sized integral type (potential portability problem)
# remark #193: zero used for undefined preprocessing identifier
# remark #593: variable "curs_sb_left_arrow" was set but never used
# remark #810: conversion from "int" to "Dimension={unsigned short}" may lose significant bits
# remark #869: parameter "tw" was never referenced
# remark #981: operands are evaluated in unspecified order
# warning #279: controlling expression is constant

	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="$EXTRA_CFLAGS -Wall"
	for cf_opt in \
		wd1419 \
		wd1683 \
		wd1684 \
		wd193 \
		wd593 \
		wd279 \
		wd810 \
		wd869 \
		wd981
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"
elif test "$GCC" = yes && test "$GCC_VERSION" != "unknown"
then
	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	cf_warn_CONST=""
	test "$with_ext_const" = yes && cf_warn_CONST="Wwrite-strings"
	cf_gcc_warnings="Wignored-qualifiers Wlogical-op Wvarargs"
	test "x$CLANG_COMPILER" = xyes && cf_gcc_warnings=
	for cf_opt in W Wall \
		Wbad-function-cast \
		Wcast-align \
		Wcast-qual \
		Wdeclaration-after-statement \
		Wextra \
		Winline \
		Wmissing-declarations \
		Wmissing-prototypes \
		Wnested-externs \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes \
		Wundef Wno-inline $cf_gcc_warnings $cf_warn_CONST $1
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			case "$cf_opt" in
			(Winline)
				case "$GCC_VERSION" in
				([[34]].*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			(Wpointer-arith)
				case "$GCC_VERSION" in
				([[12]].*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			esac
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"
fi
rm -rf ./conftest*

AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GLOB_FULLPATH version: 2 updated: 2024/08/03 12:34:02
dnl ----------------
dnl Use this in case-statements to check for pathname syntax, i.e., absolute
dnl pathnames.  The "x" is assumed since we provide an alternate form for DOS.
AC_DEFUN([CF_GLOB_FULLPATH],[
AC_REQUIRE([CF_WITH_SYSTYPE])dnl
case "$cf_cv_system_name" in
(cygwin*|msys*|mingw32*|mingw64|os2*)
	GLOB_FULLPATH_POSIX='/*'
	GLOB_FULLPATH_OTHER='[[a-zA-Z]]:[[\\/]]*'
	;;
(*)
	GLOB_FULLPATH_POSIX='/*'
	GLOB_FULLPATH_OTHER=$GLOB_FULLPATH_POSIX
	;;
esac
AC_SUBST(GLOB_FULLPATH_POSIX)
AC_SUBST(GLOB_FULLPATH_OTHER)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GNU_SOURCE version: 10 updated: 2018/12/10 20:09:41
dnl -------------
dnl Check if we must define _GNU_SOURCE to get a reasonable value for
dnl _XOPEN_SOURCE, upon which many POSIX definitions depend.  This is a defect
dnl (or misfeature) of glibc2, which breaks portability of many applications,
dnl since it is interwoven with GNU extensions.
dnl
dnl Well, yes we could work around it...
dnl
dnl Parameters:
dnl	$1 is the nominal value for _XOPEN_SOURCE
AC_DEFUN([CF_GNU_SOURCE],
[
cf_gnu_xopen_source=ifelse($1,,500,$1)

AC_CACHE_CHECK(if this is the GNU C library,cf_cv_gnu_library,[
AC_TRY_COMPILE([#include <sys/types.h>],[
	#if __GLIBC__ > 0 && __GLIBC_MINOR__ >= 0
		return 0;
	#elif __NEWLIB__ > 0 && __NEWLIB_MINOR__ >= 0
		return 0;
	#else
	#	error not GNU C library
	#endif],
	[cf_cv_gnu_library=yes],
	[cf_cv_gnu_library=no])
])

if test x$cf_cv_gnu_library = xyes; then

	# With glibc 2.19 (13 years after this check was begun), _DEFAULT_SOURCE
	# was changed to help a little.  newlib incorporated the change about 4
	# years later.
	AC_CACHE_CHECK(if _DEFAULT_SOURCE can be used as a basis,cf_cv_gnu_library_219,[
		cf_save="$CPPFLAGS"
		CF_APPEND_TEXT(CPPFLAGS,-D_DEFAULT_SOURCE)
		AC_TRY_COMPILE([#include <sys/types.h>],[
			#if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 19) || (__GLIBC__ > 2)
				return 0;
			#elif (__NEWLIB__ == 2 && __NEWLIB_MINOR__ >= 4) || (__GLIBC__ > 3)
				return 0;
			#else
			#	error GNU C library __GLIBC__.__GLIBC_MINOR__ is too old
			#endif],
			[cf_cv_gnu_library_219=yes],
			[cf_cv_gnu_library_219=no])
		CPPFLAGS="$cf_save"
	])

	if test "x$cf_cv_gnu_library_219" = xyes; then
		cf_save="$CPPFLAGS"
		AC_CACHE_CHECK(if _XOPEN_SOURCE=$cf_gnu_xopen_source works with _DEFAULT_SOURCE,cf_cv_gnu_dftsrc_219,[
			CF_ADD_CFLAGS(-D_DEFAULT_SOURCE -D_XOPEN_SOURCE=$cf_gnu_xopen_source)
			AC_TRY_COMPILE([
				#include <limits.h>
				#include <sys/types.h>
				],[
				#if (_XOPEN_SOURCE >= $cf_gnu_xopen_source) && (MB_LEN_MAX > 1)
					return 0;
				#else
				#	error GNU C library is too old
				#endif],
				[cf_cv_gnu_dftsrc_219=yes],
				[cf_cv_gnu_dftsrc_219=no])
			])
		test "x$cf_cv_gnu_dftsrc_219" = "xyes" || CPPFLAGS="$cf_save"
	else
		cf_cv_gnu_dftsrc_219=maybe
	fi

	if test "x$cf_cv_gnu_dftsrc_219" != xyes; then

		AC_CACHE_CHECK(if we must define _GNU_SOURCE,cf_cv_gnu_source,[
		AC_TRY_COMPILE([#include <sys/types.h>],[
			#ifndef _XOPEN_SOURCE
			#error	expected _XOPEN_SOURCE to be defined
			#endif],
			[cf_cv_gnu_source=no],
			[cf_save="$CPPFLAGS"
			 CF_ADD_CFLAGS(-D_GNU_SOURCE)
			 AC_TRY_COMPILE([#include <sys/types.h>],[
				#ifdef _XOPEN_SOURCE
				#error	expected _XOPEN_SOURCE to be undefined
				#endif],
				[cf_cv_gnu_source=no],
				[cf_cv_gnu_source=yes])
			CPPFLAGS="$cf_save"
			])
		])

		if test "$cf_cv_gnu_source" = yes
		then
		AC_CACHE_CHECK(if we should also define _DEFAULT_SOURCE,cf_cv_default_source,[
			CF_APPEND_TEXT(CPPFLAGS,-D_GNU_SOURCE)
			AC_TRY_COMPILE([#include <sys/types.h>],[
				#ifdef _DEFAULT_SOURCE
				#error	expected _DEFAULT_SOURCE to be undefined
				#endif],
				[cf_cv_default_source=no],
				[cf_cv_default_source=yes])
			])
			if test "$cf_cv_default_source" = yes
			then
				CF_APPEND_TEXT(CPPFLAGS,-D_DEFAULT_SOURCE)
			fi
		fi
	fi

fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HELP_MESSAGE version: 4 updated: 2019/12/31 08:53:54
dnl ---------------
dnl Insert text into the help-message, for readability, from AC_ARG_WITH.
AC_DEFUN([CF_HELP_MESSAGE],
[CF_ACVERSION_CHECK(2.53,[],[
AC_DIVERT_HELP($1)])dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_IMAKE_CFLAGS version: 34 updated: 2020/12/31 18:40:20
dnl ---------------
dnl Use imake to obtain compiler flags.  We could, in principle, write tests to
dnl get these, but if imake is properly configured there is no point in doing
dnl this.
dnl
dnl Parameters (used in constructing a sample Imakefile):
dnl	$1 = optional value to append to $IMAKE_CFLAGS
dnl	$2 = optional value to append to $IMAKE_LOADFLAGS
AC_DEFUN([CF_IMAKE_CFLAGS],
[
AC_PATH_PROGS(IMAKE,xmkmf imake)

if test -n "$IMAKE" ; then

case $IMAKE in
(*/imake)
	cf_imake_opts="-DUseInstalled=YES"
	;;
(*/util/xmkmf)
	# A single parameter tells xmkmf where the config-files are:
	cf_imake_opts="`echo "$IMAKE"|sed -e s,/config/util/xmkmf,,`"
	;;
(*)
	cf_imake_opts=
	;;
esac

# If it's installed properly, imake (or its wrapper, xmkmf) will point to the
# config directory.
if mkdir conftestdir; then
	CDPATH=; export CDPATH
	cf_makefile=`cd "$srcdir" || exit;pwd`/Imakefile
	cd conftestdir

	cat >fix_cflags.sed <<'CF_EOF'
s/\\//g
s/[[ 	]][[ 	]]*/ /g
s/"//g
:pack
s/\(=[[^ ]][[^ ]]*\) \([[^-]]\)/\1	\2/g
t pack
s/\(-D[[a-zA-Z0-9_]][[a-zA-Z0-9_]]*\)=\([[^\'0-9 ]][[^ ]]*\)/\1='\\"\2\\"'/g
s/^IMAKE[[ ]]/IMAKE_CFLAGS="/
s/	/ /g
s/$/"/
CF_EOF

	cat >fix_lflags.sed <<'CF_EOF'
s/^IMAKE[[ 	]]*/IMAKE_LOADFLAGS="/
s/$/"/
CF_EOF

	echo >./Imakefile
	test -f "$cf_makefile" && cat "$cf_makefile" >>./Imakefile

	cat >> ./Imakefile <<'CF_EOF'
findstddefs:
	@echo IMAKE ${ALLDEFINES}ifelse([$1],,,[ $1])       | sed -f fix_cflags.sed
	@echo IMAKE ${EXTRA_LOAD_FLAGS}ifelse([$2],,,[ $2]) | sed -f fix_lflags.sed
CF_EOF

	if ( $IMAKE "$cf_imake_opts" 1>/dev/null 2>&AC_FD_CC && test -f Makefile)
	then
		CF_VERBOSE(Using $IMAKE $cf_imake_opts)
	else
		# sometimes imake doesn't have the config path compiled in.  Find it.
		cf_config=
		for cf_libpath in $X_LIBS $LIBS ; do
			case "$cf_libpath" in
			(-L*)
				cf_libpath=`echo ".$cf_libpath" | sed -e 's/^...//'`
				cf_libpath="$cf_libpath/X11/config"
				if test -d "$cf_libpath" ; then
					cf_config="$cf_libpath"
					break
				fi
				;;
			esac
		done
		if test -z "$cf_config" ; then
			AC_MSG_WARN(Could not find imake config-directory)
		else
			cf_imake_opts="$cf_imake_opts -I$cf_config"
			if ( "$IMAKE" -v "$cf_imake_opts" 2>&AC_FD_CC)
			then
				CF_VERBOSE(Using $IMAKE $cf_config)
			else
				AC_MSG_WARN(Cannot run $IMAKE)
			fi
		fi
	fi

	# GNU make sometimes prints "make[1]: Entering...", which
	# would confuse us.
	eval "`make findstddefs 2>/dev/null | grep -v make`"

	cd ..
	rm -rf conftestdir

	# We use ${ALLDEFINES} rather than ${STD_DEFINES} because the former
	# declares XTFUNCPROTO there.  However, some vendors (e.g., SGI) have
	# modified it to support site.cf, adding a kludge for the /usr/include
	# directory.  Try to filter that out, otherwise gcc won't find its
	# headers.
	if test -n "$GCC" ; then
	    if test -n "$IMAKE_CFLAGS" ; then
		cf_nostdinc=""
		cf_std_incl=""
		cf_cpp_opts=""
		for cf_opt in $IMAKE_CFLAGS
		do
		    case "$cf_opt" in
		    (-nostdinc)
			cf_nostdinc="$cf_opt"
			;;
		    (-I/usr/include)
			cf_std_incl="$cf_opt"
			;;
		    (*)
			cf_cpp_opts="$cf_cpp_opts $cf_opt"
			;;
		    esac
		done
		if test -z "$cf_nostdinc" ; then
		    IMAKE_CFLAGS="$cf_cpp_opts $cf_std_incl"
		elif test -z "$cf_std_incl" ; then
		    IMAKE_CFLAGS="$cf_cpp_opts $cf_nostdinc"
		else
		    CF_VERBOSE(suppressed \"$cf_nostdinc\" and \"$cf_std_incl\")
		    IMAKE_CFLAGS="$cf_cpp_opts"
		fi
	    fi
	fi
fi

# Some imake configurations define PROJECTROOT with an empty value.  Remove
# the empty definition.
case $IMAKE_CFLAGS in
(*-DPROJECTROOT=/*)
	;;
(*)
	IMAKE_CFLAGS=`echo "$IMAKE_CFLAGS" |sed -e "s,-DPROJECTROOT=[[ 	]], ,"`
	;;
esac

fi

CF_VERBOSE(IMAKE_CFLAGS $IMAKE_CFLAGS)
CF_VERBOSE(IMAKE_LOADFLAGS $IMAKE_LOADFLAGS)

AC_SUBST(IMAKE_CFLAGS)
AC_SUBST(IMAKE_LOADFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INPUT_METHOD version: 5 updated: 2021/06/07 17:39:17
dnl ---------------
dnl Check if the X libraries support input-method
AC_DEFUN([CF_INPUT_METHOD],
[
AC_CACHE_CHECK([if X libraries support input-method],cf_cv_input_method,[
AC_TRY_LINK([
#include <X11/IntrinsicP.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/Converters.h>
#include <X11/Xaw/XawImP.h>
],[
{
	XIM xim;
	XIMStyles *xim_styles = 0;
	XIMStyle input_style;
	Widget w = XtCreateWidget("none", (WidgetClass)0, None, (ArgList)0, 0);

	XSetLocaleModifiers("@im=none");
	xim = XOpenIM(XtDisplay(w), NULL, NULL, NULL);
	XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL);
	XCloseIM(xim);
	input_style = (XIMPreeditNothing | XIMStatusNothing);
	(void)xim_styles;
	(void)input_style;
}
],
[cf_cv_input_method=yes],
[cf_cv_input_method=no])])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INTEL_COMPILER version: 9 updated: 2023/02/18 17:41:25
dnl -----------------
dnl Check if the given compiler is really the Intel compiler for Linux.  It
dnl tries to imitate gcc, but does not return an error when it finds a mismatch
dnl between prototypes, e.g., as exercised by CF_MISSING_CHECK.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = INTEL_COMPILER (default) or INTEL_CPLUSPLUS
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_INTEL_COMPILER],[
AC_REQUIRE([AC_CANONICAL_HOST])
ifelse([$2],,INTEL_COMPILER,[$2])=no

if test "$ifelse([$1],,[$1],GCC)" = yes ; then
	case "$host_os" in
	(linux*|gnu*)
		AC_MSG_CHECKING(if this is really Intel ifelse([$1],GXX,C++,C) compiler)
		cf_save_CFLAGS="$ifelse([$3],,CFLAGS,[$3])"
		ifelse([$3],,CFLAGS,[$3])="$ifelse([$3],,CFLAGS,[$3]) -no-gcc"
		AC_TRY_COMPILE([],[
#ifdef __INTEL_COMPILER
#else
#error __INTEL_COMPILER is not defined
#endif
],[ifelse([$2],,INTEL_COMPILER,[$2])=yes
cf_save_CFLAGS="$cf_save_CFLAGS -we147"
],[])
		ifelse([$3],,CFLAGS,[$3])="$cf_save_CFLAGS"
		AC_MSG_RESULT($ifelse([$2],,INTEL_COMPILER,[$2]))
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LASTLOG version: 8 updated: 2023/12/01 17:22:50
dnl ----------
dnl Check for header defining _PATH_LASTLOG, or failing that, see if the lastlog
dnl file exists.
AC_DEFUN([CF_LASTLOG],
[
AC_CHECK_HEADERS(lastlog.h paths.h)
AC_CACHE_CHECK(for lastlog path,cf_cv_path_lastlog,[
AC_TRY_COMPILE([
#include <sys/types.h>
#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#else
#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
#endif],[static char path[] = _PATH_LASTLOG; (void)path],
	[cf_cv_path_lastlog="_PATH_LASTLOG"],
	[if test -f /usr/adm/lastlog ; then
	 	cf_cv_path_lastlog=/usr/adm/lastlog
	else
		cf_cv_path_lastlog=no
	fi])
])
test "$cf_cv_path_lastlog" != no && AC_DEFINE(USE_LASTLOG,1,[Define to 1 if we can define lastlog pathname])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LD_RPATH_OPT version: 9 updated: 2021/01/01 13:31:04
dnl ---------------
dnl For the given system and compiler, find the compiler flags to pass to the
dnl loader to use the "rpath" feature.
AC_DEFUN([CF_LD_RPATH_OPT],
[
AC_REQUIRE([CF_CHECK_CACHE])

LD_RPATH_OPT=
if test "x$cf_cv_enable_rpath" != xno
then
	AC_MSG_CHECKING(for an rpath option)
	case "$cf_cv_system_name" in
	(irix*)
		if test "$GCC" = yes; then
			LD_RPATH_OPT="-Wl,-rpath,"
		else
			LD_RPATH_OPT="-rpath "
		fi
		;;
	(linux*|gnu*|k*bsd*-gnu|freebsd*)
		LD_RPATH_OPT="-Wl,-rpath,"
		;;
	(openbsd[[2-9]].*|mirbsd*)
		LD_RPATH_OPT="-Wl,-rpath,"
		;;
	(dragonfly*)
		LD_RPATH_OPT="-rpath "
		;;
	(netbsd*)
		LD_RPATH_OPT="-Wl,-rpath,"
		;;
	(osf*|mls+*)
		LD_RPATH_OPT="-rpath "
		;;
	(solaris2*)
		LD_RPATH_OPT="-R"
		;;
	(*)
		;;
	esac
	AC_MSG_RESULT($LD_RPATH_OPT)

	case "x$LD_RPATH_OPT" in
	(x-R*)
		AC_MSG_CHECKING(if we need a space after rpath option)
		cf_save_LIBS="$LIBS"
		CF_ADD_LIBS(${LD_RPATH_OPT}$libdir)
		AC_TRY_LINK(, , cf_rpath_space=no, cf_rpath_space=yes)
		LIBS="$cf_save_LIBS"
		AC_MSG_RESULT($cf_rpath_space)
		test "$cf_rpath_space" = yes && LD_RPATH_OPT="$LD_RPATH_OPT "
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAKE_TAGS version: 6 updated: 2010/10/23 15:52:32
dnl ------------
dnl Generate tags/TAGS targets for makefiles.  Do not generate TAGS if we have
dnl a monocase filesystem.
AC_DEFUN([CF_MAKE_TAGS],[
AC_REQUIRE([CF_MIXEDCASE_FILENAMES])

AC_CHECK_PROGS(CTAGS, exctags ctags)
AC_CHECK_PROGS(ETAGS, exetags etags)

AC_CHECK_PROG(MAKE_LOWER_TAGS, ${CTAGS:-ctags}, yes, no)

if test "$cf_cv_mixedcase" = yes ; then
	AC_CHECK_PROG(MAKE_UPPER_TAGS, ${ETAGS:-etags}, yes, no)
else
	MAKE_UPPER_TAGS=no
fi

if test "$MAKE_UPPER_TAGS" = yes ; then
	MAKE_UPPER_TAGS=
else
	MAKE_UPPER_TAGS="#"
fi

if test "$MAKE_LOWER_TAGS" = yes ; then
	MAKE_LOWER_TAGS=
else
	MAKE_LOWER_TAGS="#"
fi

AC_SUBST(CTAGS)
AC_SUBST(ETAGS)

AC_SUBST(MAKE_UPPER_TAGS)
AC_SUBST(MAKE_LOWER_TAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MATH_LIB version: 11 updated: 2022/07/27 19:01:48
dnl -----------
dnl Checks for libraries.  At least one UNIX system, Apple Macintosh
dnl Rhapsody 5.5, does not have -lm.  We cannot use the simpler
dnl AC_CHECK_LIB(m,sin), because that fails for C++.
AC_DEFUN([CF_MATH_LIB],
[
AC_CACHE_CHECK(if -lm needed for math functions,
	cf_cv_need_libm,[
	AC_TRY_LINK([
		#include <stdio.h>
		#include <stdlib.h>
		#include <math.h>
	],
	[double x = rand(); printf("result = %g\\n", ]ifelse([$2],,sin(x),$2)[)],
	[cf_cv_need_libm=no],
	[cf_cv_need_libm=yes])])

if test "$cf_cv_need_libm" = yes
then

	cf_save_LIBS="$LIBS"
	LIBS="$LIBS -lm"
	AC_CACHE_CHECK(if -lm is available for math functions,
	cf_cv_have_libm,[
	AC_TRY_LINK([
		#include <stdio.h>
		#include <stdlib.h>
		#include <math.h>
	],
	[double x = rand(); printf("result = %g\\n", ]ifelse([$2],,sin(x),$2)[)],
	[cf_cv_have_libm=yes],
	[cf_cv_have_libm=no])])
	LIBS="$cf_save_LIBS"

	if test "$cf_cv_have_libm" = yes
	then
		ifelse($1,,[CF_ADD_LIB(m)],[$1=-lm])
	fi
else
	cf_cv_have_libm=yes
fi

if test "$cf_cv_have_libm" = yes
then
	AC_DEFINE(HAVE_MATH_FUNCS,1,[Define to 1 if math functions are available])
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_MIXEDCASE_FILENAMES version: 9 updated: 2021/01/01 16:53:59
dnl ----------------------
dnl Check if the file-system supports mixed-case filenames.  If we're able to
dnl create a lowercase name and see it as uppercase, it doesn't support that.
AC_DEFUN([CF_MIXEDCASE_FILENAMES],
[
AC_CACHE_CHECK(if filesystem supports mixed-case filenames,cf_cv_mixedcase,[
if test "$cross_compiling" = yes ; then
	case "$target_alias" in
	(*-os2-emx*|*-msdosdjgpp*|*-cygwin*|*-msys*|*-mingw*|*-uwin*|darwin*)
		cf_cv_mixedcase=no
		;;
	(*)
		cf_cv_mixedcase=yes
		;;
	esac
else
	rm -f conftest CONFTEST
	echo test >conftest
	if test -f CONFTEST ; then
		cf_cv_mixedcase=no
	else
		cf_cv_mixedcase=yes
	fi
	rm -f conftest CONFTEST
fi
])
test "$cf_cv_mixedcase" = yes && AC_DEFINE(MIXEDCASE_FILENAMES,1,[Define to 1 if filesystem supports mixed-case filenames.])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MKSTEMP version: 13 updated: 2023/12/01 17:22:50
dnl ----------
dnl Check for a working mkstemp.  This creates two files, checks that they are
dnl successfully created and distinct (AmigaOS apparently fails on the last).
AC_DEFUN([CF_MKSTEMP],[
AC_CHECK_HEADERS( \
unistd.h \
)
AC_CACHE_CHECK(for working mkstemp, cf_cv_func_mkstemp,[
rm -rf ./conftest*
AC_TRY_RUN([
$ac_includes_default

int main(void)
{
	static char tmpl[] = "conftestXXXXXX";
	char name[2][80];
	int n;
	int result = 0;
	int fd;
	struct stat sb;

	umask(077);
	for (n = 0; n < 2; ++n) {
		strcpy(name[n], tmpl);
		if ((fd = mkstemp(name[n])) >= 0) {
			if (!strcmp(name[n], tmpl)
			 || stat(name[n], &sb) != 0
			 || (sb.st_mode & S_IFMT) != S_IFREG
			 || (sb.st_mode & 077) != 0) {
				result = 1;
			}
			close(fd);
		}
	}
	if (result == 0
	 && !strcmp(name[0], name[1]))
		result = 1;
	${cf_cv_main_return:-return}(result);
}
],[cf_cv_func_mkstemp=yes
],[cf_cv_func_mkstemp=no
],[cf_cv_func_mkstemp=maybe])
])
if test "x$cf_cv_func_mkstemp" = xmaybe ; then
	AC_CHECK_FUNC(mkstemp)
fi
if test "x$cf_cv_func_mkstemp" = xyes || test "x$ac_cv_func_mkstemp" = xyes ; then
	AC_DEFINE(HAVE_MKSTEMP,1,[Define to 1 if mkstemp() is available and working.])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MSG_LOG version: 5 updated: 2010/10/23 15:52:32
dnl ----------
dnl Write a debug message to config.log, along with the line number in the
dnl configure script.
AC_DEFUN([CF_MSG_LOG],[
echo "${as_me:-configure}:__oline__: testing $* ..." 1>&AC_FD_CC
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NO_LEAKS_OPTION version: 9 updated: 2021/06/13 19:45:41
dnl ------------------
dnl see CF_WITH_NO_LEAKS
dnl
dnl $1 = option/name
dnl $2 = help-text
dnl $3 = symbol to define if the option is set
dnl $4 = additional actions to take if the option is set
AC_DEFUN([CF_NO_LEAKS_OPTION],[
AC_MSG_CHECKING(if you want to use $1 for testing)
AC_ARG_WITH($1,
	[$2],
	[case "x$withval" in
	(x|xno) ;;
	(*)
		: "${with_cflags:=-g}"
		: "${enable_leaks:=no}"
		with_$1=yes
		AC_DEFINE_UNQUOTED($3,1,"Define to 1 if you want to use $1 for testing.")ifelse([$4],,[
	 $4
])
		;;
	esac],
	[with_$1=])
AC_MSG_RESULT(${with_$1:-no})

case ".$with_cflags" in
(.*-g*)
	case .$CFLAGS in
	(.*-g*)
		;;
	(*)
		CF_ADD_CFLAGS([-g])
		;;
	esac
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATHSEP version: 8 updated: 2021/01/01 13:31:04
dnl ----------
dnl Provide a value for the $PATH and similar separator (or amend the value
dnl as provided in autoconf 2.5x).
AC_DEFUN([CF_PATHSEP],
[
	AC_MSG_CHECKING(for PATH separator)
	case "$cf_cv_system_name" in
	(os2*)	PATH_SEPARATOR=';'  ;;
	(*)	${PATH_SEPARATOR:=':'}  ;;
	esac
ifelse([$1],,,[$1=$PATH_SEPARATOR])
	AC_SUBST(PATH_SEPARATOR)
	AC_MSG_RESULT($PATH_SEPARATOR)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATH_PROG version: 12 updated: 2021/01/02 09:31:20
dnl ------------
dnl Check for a given program, defining corresponding symbol.
dnl	$1 = environment variable, which is suffixed by "_PATH" in the #define.
dnl	$2 = program name to find.
dnl	$3 = optional list of additional program names to test.
dnl $4 = $PATH
dnl
dnl If there is more than one token in the result, #define the remaining tokens
dnl to $1_ARGS.  We need this for 'install' in particular.
dnl
dnl FIXME: we should allow this to be overridden by environment variables
dnl
AC_DEFUN([CF_PATH_PROG],[
AC_REQUIRE([CF_PATHSEP])
test -z "[$]$1" && $1="$2"
AC_PATH_PROGS($1,[$]$1 $2 ifelse($3,,,$3),[$]$1, ifelse($4,,,$4))

cf_path_prog=""
cf_path_args=""
IFS="${IFS:- 	}"; cf_save_ifs="$IFS"; IFS="${IFS}$PATH_SEPARATOR"
for cf_temp in $ac_cv_path_$1
do
	if test -z "$cf_path_prog" ; then
		if test "$with_full_paths" = yes ; then
			CF_PATH_SYNTAX(cf_temp,break)
			cf_path_prog="$cf_temp"
		else
			cf_path_prog="`basename "$cf_temp"`"
		fi
	elif test -z "$cf_path_args" ; then
		cf_path_args="$cf_temp"
	else
		cf_path_args="$cf_path_args $cf_temp"
	fi
done
IFS="$cf_save_ifs"

if test -n "$cf_path_prog" ; then
	CF_MSG_LOG(defining path for ${cf_path_prog})
	AC_DEFINE_UNQUOTED($1_PATH,"$cf_path_prog",Define to pathname $1)
	test -n "$cf_path_args" && AC_DEFINE_UNQUOTED($1_ARGS,"$cf_path_args",Define to provide args for $1)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATH_SYNTAX version: 19 updated: 2024/08/03 13:08:58
dnl --------------
dnl Check the argument to see that it looks like a pathname.  Rewrite it if it
dnl begins with one of the prefix/exec_prefix variables, and then again if the
dnl result begins with 'NONE'.  This is necessary to work around autoconf's
dnl delayed evaluation of those symbols.
AC_DEFUN([CF_PATH_SYNTAX],[
AC_REQUIRE([CF_GLOB_FULLPATH])dnl

if test "x$prefix" != xNONE; then
	cf_path_syntax="$prefix"
else
	cf_path_syntax="$ac_default_prefix"
fi

case "x[$]$1" in
(x\[$]\(*\)*|x\'*\'*)
	;;
(x.|x$GLOB_FULLPATH_POSIX|x$GLOB_FULLPATH_OTHER)
	;;
(x\[$]\{*prefix\}*|x\[$]\{*dir\}*)
	eval $1="[$]$1"
	case "x[$]$1" in
	(xNONE/*)
		$1=`echo "[$]$1" | sed -e s%NONE%$cf_path_syntax%`
		;;
	esac
	;;
(xno|xNONE/*)
	$1=`echo "[$]$1" | sed -e s%NONE%$cf_path_syntax%`
	;;
(*)
	ifelse([$2],,[AC_MSG_ERROR([expected a pathname, not \"[$]$1\"])],$2)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PKG_CONFIG version: 13 updated: 2023/10/28 11:59:01
dnl -------------
dnl Check for the package-config program, unless disabled by command-line.
dnl
dnl Sets $PKG_CONFIG to the pathname of the pkg-config program.
AC_DEFUN([CF_PKG_CONFIG],
[
AC_MSG_CHECKING(if you want to use pkg-config)
AC_ARG_WITH(pkg-config,
	[[  --with-pkg-config[=CMD] enable/disable use of pkg-config and its name CMD]],
	[cf_pkg_config=$withval],
	[cf_pkg_config=yes])
AC_MSG_RESULT($cf_pkg_config)

case "$cf_pkg_config" in
(no)
	PKG_CONFIG=none
	;;
(yes)
	CF_ACVERSION_CHECK(2.52,
		[AC_PATH_TOOL(PKG_CONFIG, pkg-config, none)],
		[AC_PATH_PROG(PKG_CONFIG, pkg-config, none)])
	;;
(*)
	PKG_CONFIG=$withval
	;;
esac

test -z "$PKG_CONFIG" && PKG_CONFIG=none
if test "$PKG_CONFIG" != none ; then
	CF_PATH_SYNTAX(PKG_CONFIG)
elif test "x$cf_pkg_config" != xno ; then
	AC_MSG_WARN(pkg-config is not installed)
fi

AC_SUBST(PKG_CONFIG)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_C_SOURCE version: 12 updated: 2023/02/18 17:41:25
dnl -----------------
dnl Define _POSIX_C_SOURCE to the given level, and _POSIX_SOURCE if needed.
dnl
dnl	POSIX.1-1990				_POSIX_SOURCE
dnl	POSIX.1-1990 and			_POSIX_SOURCE and
dnl		POSIX.2-1992 C-Language			_POSIX_C_SOURCE=2
dnl		Bindings Option
dnl	POSIX.1b-1993				_POSIX_C_SOURCE=199309L
dnl	POSIX.1c-1996				_POSIX_C_SOURCE=199506L
dnl	X/Open 2000				_POSIX_C_SOURCE=200112L
dnl
dnl Parameters:
dnl	$1 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_POSIX_C_SOURCE],
[AC_REQUIRE([CF_POSIX_VISIBLE])dnl

if test "$cf_cv_posix_visible" = no; then

cf_POSIX_C_SOURCE=ifelse([$1],,199506L,[$1])

cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"

CF_REMOVE_DEFINE(cf_trim_CFLAGS,$cf_save_CFLAGS,_POSIX_C_SOURCE)
CF_REMOVE_DEFINE(cf_trim_CPPFLAGS,$cf_save_CPPFLAGS,_POSIX_C_SOURCE)

AC_CACHE_CHECK(if we should define _POSIX_C_SOURCE,cf_cv_posix_c_source,[
	CF_MSG_LOG(if the symbol is already defined go no further)
	AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
#error _POSIX_C_SOURCE is not defined
#endif],
	[cf_cv_posix_c_source=no],
	[cf_want_posix_source=no
	 case .$cf_POSIX_C_SOURCE in
	 (.[[12]]??*)
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		;;
	 (.2)
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		cf_want_posix_source=yes
		;;
	 (.*)
		cf_want_posix_source=yes
		;;
	 esac
	 if test "$cf_want_posix_source" = yes ; then
		AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _POSIX_SOURCE
#error _POSIX_SOURCE is defined
#endif],[],
		cf_cv_posix_c_source="$cf_cv_posix_c_source -D_POSIX_SOURCE")
	 fi
	 CF_MSG_LOG(ifdef from value $cf_POSIX_C_SOURCE)
	 CFLAGS="$cf_trim_CFLAGS"
	 CPPFLAGS="$cf_trim_CPPFLAGS"
	 CF_APPEND_TEXT(CPPFLAGS,$cf_cv_posix_c_source)
	 CF_MSG_LOG(if the second compile does not leave our definition intact error)
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
#error _POSIX_C_SOURCE is not defined
#endif],,
	 [cf_cv_posix_c_source=no])
	 CFLAGS="$cf_save_CFLAGS"
	 CPPFLAGS="$cf_save_CPPFLAGS"
	])
])

if test "$cf_cv_posix_c_source" != no ; then
	CFLAGS="$cf_trim_CFLAGS"
	CPPFLAGS="$cf_trim_CPPFLAGS"
	CF_ADD_CFLAGS($cf_cv_posix_c_source)
fi

fi # cf_cv_posix_visible

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_SAVED_IDS version: 12 updated: 2023/12/03 19:55:51
dnl ------------------
dnl
dnl Check first if saved-ids are always supported.  Some systems
dnl may require runtime checks.
AC_DEFUN([CF_POSIX_SAVED_IDS],
[
AC_CHECK_HEADERS( \
sys/param.h \
)

AC_CACHE_CHECK(if POSIX saved-ids are supported,cf_cv_posix_saved_ids,[
AC_TRY_LINK(
[
#include <unistd.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>		/* this may define "BSD" */
#endif
],[
#if defined(_POSIX_SAVED_IDS) && (_POSIX_SAVED_IDS > 0)
	int (*my_seteuid)(uid_t) = seteuid;
	int x = my_seteuid(geteuid());
	(void)x;
#elif defined(BSD) && (BSD >= 199103)
/* The BSD's may implement the runtime check - and it fails.
 * However, saved-ids work almost like POSIX (close enough for most uses).
 */
#else
#error no saved-ids found
#endif
],[cf_cv_posix_saved_ids=yes
],[
AC_TRY_RUN([
$ac_includes_default

int main(void)
{
	void *p = (void *) seteuid;
	long code = sysconf(_SC_SAVED_IDS);
	(void)p;
	${cf_cv_main_return:-return}  ((code > 0) ? 0 : 1);
}],
	cf_cv_posix_saved_ids=yes,
	cf_cv_posix_saved_ids=no,
	cf_cv_posix_saved_ids=unknown)
])
])

test "$cf_cv_posix_saved_ids" = yes && AC_DEFINE(HAVE_POSIX_SAVED_IDS,1,[Define to 1 if POSIX saved-ids are supported])
])
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_VISIBLE version: 1 updated: 2018/12/31 20:46:17
dnl ----------------
dnl POSIX documents test-macros which an application may set before any system
dnl headers are included to make features available.
dnl
dnl Some BSD platforms (originally FreeBSD, but copied by a few others)
dnl diverged from POSIX in 2002 by setting symbols which make all of the most
dnl recent features visible in the system header files unless the application
dnl overrides the corresponding test-macros.  Doing that introduces portability
dnl problems.
dnl
dnl This macro makes a special check for the symbols used for this, to avoid a
dnl conflicting definition.
AC_DEFUN([CF_POSIX_VISIBLE],
[
AC_CACHE_CHECK(if the POSIX test-macros are already defined,cf_cv_posix_visible,[
AC_TRY_COMPILE([#include <stdio.h>],[
#if defined(__POSIX_VISIBLE) && ((__POSIX_VISIBLE - 0L) > 0) \
	&& defined(__XSI_VISIBLE) && ((__XSI_VISIBLE - 0L) > 0) \
	&& defined(__BSD_VISIBLE) && ((__BSD_VISIBLE - 0L) > 0) \
	&& defined(__ISO_C_VISIBLE) && ((__ISO_C_VISIBLE - 0L) > 0)
#error conflicting symbols found
#endif
],[cf_cv_posix_visible=no],[cf_cv_posix_visible=yes])
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_WAIT version: 5 updated: 2023/01/05 19:26:07
dnl -------------
dnl Check for POSIX wait support
AC_DEFUN([CF_POSIX_WAIT],
[
AC_REQUIRE([AC_HEADER_SYS_WAIT])
AC_CACHE_CHECK(for POSIX wait functions,cf_cv_posix_wait,[
AC_TRY_LINK([
$ac_includes_default

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
],[
	int stat_loc;
	pid_t pid = waitpid(-1, &stat_loc, WNOHANG|WUNTRACED);
	pid_t pid2 = wait(&stat_loc);
	(void)pid;
	(void)pid2;
],
[cf_cv_posix_wait=yes],
[cf_cv_posix_wait=no])
])
test "$cf_cv_posix_wait" = yes && AC_DEFINE(USE_POSIX_WAIT,1,[Define to 1 if we have POSIX wait functions])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROCFS_CWD version: 4 updated: 2020/12/31 18:40:20
dnl -------------
dnl Find /proc tree (may be in a different place) which implements the "cwd"
dnl link.
AC_DEFUN([CF_PROCFS_CWD],[
AC_CACHE_CHECK(for proc tree with cwd-support,cf_cv_procfs_cwd,[
cf_cv_procfs_cwd=no
for cf_path in /proc /compat/linux/proc /usr/compat/linux/proc
do
	if test -d "$cf_path" && \
	   test -d "$cf_path"/$$ && \
	   { test -d "$cf_path"/$$/cwd || \
	     test -L "$cf_path"/$$/cwd; }; then
		cf_cv_procfs_cwd="$cf_path"
		break
	fi
done
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_CC version: 5 updated: 2019/12/31 08:53:54
dnl ----------
dnl standard check for CC, plus followup sanity checks
dnl $1 = optional parameter to pass to AC_PROG_CC to specify compiler name
AC_DEFUN([CF_PROG_CC],[
CF_ACVERSION_CHECK(2.53,
	[AC_MSG_WARN(this will incorrectly handle gnatgcc choice)
	 AC_REQUIRE([AC_PROG_CC])],
	[])
ifelse($1,,[AC_PROG_CC],[AC_PROG_CC($1)])
CF_GCC_VERSION
CF_ACVERSION_CHECK(2.52,
	[AC_PROG_CC_STDC],
	[CF_ANSI_CC_REQD])
CF_CC_ENV_FLAGS
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_GROFF version: 3 updated: 2018/01/07 13:16:19
dnl -------------
dnl Check if groff is available, for cases (such as html output) where nroff
dnl is not enough.
AC_DEFUN([CF_PROG_GROFF],[
AC_PATH_PROG(GROFF_PATH,groff,no)
AC_PATH_PROGS(NROFF_PATH,nroff mandoc,no)
AC_PATH_PROG(TBL_PATH,tbl,cat)
if test "x$GROFF_PATH" = xno
then
	NROFF_NOTE=
	GROFF_NOTE="#"
else
	NROFF_NOTE="#"
	GROFF_NOTE=
fi
AC_SUBST(GROFF_NOTE)
AC_SUBST(NROFF_NOTE)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_LINT version: 7 updated: 2024/11/30 14:37:45
dnl ------------
AC_DEFUN([CF_PROG_LINT],
[
AC_CHECK_PROGS(LINT, lint cppcheck splint)
case "x$LINT" in
(xlint|x*/lint) # NetBSD 10
	test -z "$LINT_OPTS" && LINT_OPTS="-chapbrxzgFS -v -Ac11"
	;;
(xcppcheck|x*/cppcheck)
	test -z "$LINT_OPTS" && LINT_OPTS="--enable=all -D__CPPCHECK__"
	;;
esac
AC_SUBST(LINT_OPTS)
AC_SUBST(LINT_LIBS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REGEX version: 19 updated: 2024/12/14 16:09:34
dnl --------
dnl Attempt to determine if we've got one of the flavors of regular-expression
dnl code that we can support.
AC_DEFUN([CF_REGEX],
[

cf_regex_func=no
cf_regex_libs=
case "$host_os" in
(mingw*)
	# -lsystre -ltre -lintl -liconv
	AC_CHECK_LIB(systre,regcomp,[
		AC_CHECK_LIB(iconv,libiconv_open,[CF_ADD_LIB(iconv)])
		AC_CHECK_LIB(intl,libintl_gettext,[CF_ADD_LIB(intl)])
		AC_CHECK_LIB(tre,tre_regcomp,[CF_ADD_LIB(tre)])
		CF_ADD_LIB(systre)
		cf_regex_func=regcomp
	],[
		AC_CHECK_LIB(gnurx,regcomp,[
			CF_ADD_LIB(gnurx)
			cf_regex_func=regcomp])
	])
	;;
(*)
	cf_regex_libs="regex re"
	AC_CHECK_FUNC(regcomp,[cf_regex_func=regcomp],[
		for cf_regex_lib in $cf_regex_libs
		do
			AC_CHECK_LIB($cf_regex_lib,regcomp,[
					CF_ADD_LIB($cf_regex_lib)
					cf_regex_func=regcomp
					break])
		done
	])
	;;
esac

if test "$cf_regex_func" = no ; then
	AC_CHECK_FUNC(compile,[cf_regex_func=compile],[
		AC_CHECK_LIB(gen,compile,[
				CF_ADD_LIB(gen)
				cf_regex_func=compile])])
fi

if test "$cf_regex_func" = no ; then
	AC_MSG_WARN(cannot find regular expression library)
fi

AC_CACHE_CHECK(for regular-expression headers,cf_cv_regex_hdrs,[

cf_cv_regex_hdrs=no
case "$cf_regex_func" in
(compile)
	for cf_regex_hdr in regexp.h regexpr.h
	do
		AC_TRY_LINK([#include <$cf_regex_hdr>],[
			char *p = compile("", "", "", 0);
			int x = step("", "");
			(void)p;
			(void)x;
		],[
			cf_cv_regex_hdrs=$cf_regex_hdr
			break
		])
	done
	;;
(*)
	for cf_regex_hdr in regex.h
	do
		AC_TRY_LINK([#include <sys/types.h>
#include <stdio.h>
#include <$cf_regex_hdr>],[
			regex_t *p = NULL;
			int x = regcomp(p, "", 0);
			int y = regexec(p, "", 0, 0, 0);
			(void)x;
			(void)y;
			regfree(p);
		],[
			cf_cv_regex_hdrs=$cf_regex_hdr
			break
		])
	done
	;;
esac

])

case "$cf_cv_regex_hdrs" in
	(no)		AC_MSG_WARN(no regular expression header found) ;;
	(regex.h)	AC_DEFINE(HAVE_REGEX_H_FUNCS,1,[Define to 1 to include regex.h for regular expressions]) ;;
	(regexp.h)	AC_DEFINE(HAVE_REGEXP_H_FUNCS,1,[Define to 1 to include regexp.h for regular expressions]) ;;
	(regexpr.h) AC_DEFINE(HAVE_REGEXPR_H_FUNCS,1,[Define to 1 to include regexpr.h for regular expressions]) ;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REMOVE_CFLAGS version: 3 updated: 2021/09/05 17:25:40
dnl ----------------
dnl Remove a given option from CFLAGS/CPPFLAGS
dnl $1 = option to remove
dnl $2 = variable to update
dnl $3 = nonempty to allow verbose message
define([CF_REMOVE_CFLAGS],
[
cf_tmp_cflag=`echo "x$1" | sed -e 's/^.//' -e 's/=.*//'`
while true
do
	cf_old_cflag=`echo "x[$]$2" | sed -e 's/^.//' -e 's/[[ 	]][[ 	]]*-/ -/g' -e "s%$cf_tmp_cflag\\(=[[^ 	]][[^ 	]]*\\)\?%%" -e 's/^[[ 	]]*//' -e 's%[[ ]][[ ]]*-D% -D%g' -e 's%[[ ]][[ ]]*-I% -I%g'`
	test "[$]$2" != "$cf_old_cflag" || break
	ifelse([$3],,,[CF_VERBOSE(removing old option $1 from $2)])
	$2="$cf_old_cflag"
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REMOVE_DEFINE version: 3 updated: 2010/01/09 11:05:50
dnl ----------------
dnl Remove all -U and -D options that refer to the given symbol from a list
dnl of C compiler options.  This works around the problem that not all
dnl compilers process -U and -D options from left-to-right, so a -U option
dnl cannot be used to cancel the effect of a preceding -D option.
dnl
dnl $1 = target (which could be the same as the source variable)
dnl $2 = source (including '$')
dnl $3 = symbol to remove
define([CF_REMOVE_DEFINE],
[
$1=`echo "$2" | \
	sed	-e 's/-[[UD]]'"$3"'\(=[[^ 	]]*\)\?[[ 	]]/ /g' \
		-e 's/-[[UD]]'"$3"'\(=[[^ 	]]*\)\?[$]//g'`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RESTORE_XTRA_FLAGS version: 1 updated: 2020/01/11 16:47:45
dnl ---------------------
dnl Restore flags saved in CF_SAVE_XTRA_FLAGS
dnl $1 = name of current macro
define([CF_RESTORE_XTRA_FLAGS],
[
LIBS="$cf_save_LIBS_$1"
CFLAGS="$cf_save_CFLAGS_$1"
CPPFLAGS="$cf_save_CPPFLAGS_$1"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REQUIRE_PKG version: 1 updated: 2025/01/10 19:55:54
dnl --------------
dnl Update $REQUIRE_PKG, which lists the known required packages for this
dnl program.
dnl
dnl $1 = package(s) to require, e.g., in the generated ".pc" file
define([CF_REQUIRE_PKG],
[
for cf_required in $1
do
	# check for duplicates
	for cf_require_pkg in $REQUIRE_PKG
	do
		if test "$cf_required" = "$cf_require_pkg"
		then
			cf_required=
			break
		fi
	done
	test -n "$cf_required" && REQUIRE_PKG="$REQUIRE_PKG $cf_required"
done
AC_SUBST(REQUIRE_PKG)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RPATH_HACK version: 13 updated: 2021/01/03 18:30:50
dnl -------------
AC_DEFUN([CF_RPATH_HACK],
[AC_REQUIRE([AC_PROG_FGREP])dnl
AC_REQUIRE([CF_LD_RPATH_OPT])dnl

AC_MSG_CHECKING(for updated LDFLAGS)
if test -n "$LD_RPATH_OPT" ; then
	AC_MSG_RESULT(maybe)

	AC_CHECK_PROGS(cf_ldd_prog,ldd,no)
	cf_rpath_list="/usr/lib /lib"
	if test "$cf_ldd_prog" != no
	then
		cf_rpath_oops=

AC_TRY_LINK([#include <stdio.h>],
		[printf("Hello");],
		[cf_rpath_oops=`"$cf_ldd_prog" "conftest$ac_exeext" | ${FGREP-fgrep} ' not found' | sed -e 's% =>.*$%%' |sort | uniq`
		 cf_rpath_list=`"$cf_ldd_prog" "conftest$ac_exeext" | ${FGREP-fgrep} / | sed -e 's%^.*[[ 	]]/%/%' -e 's%/[[^/]][[^/]]*$%%' |sort | uniq`])

		# If we passed the link-test, but get a "not found" on a given library,
		# this could be due to inept reconfiguration of gcc to make it only
		# partly honor /usr/local/lib (or whatever).  Sometimes this behavior
		# is intentional, e.g., installing gcc in /usr/bin and suppressing the
		# /usr/local libraries.
		if test -n "$cf_rpath_oops"
		then
			for cf_rpath_src in $cf_rpath_oops
			do
				for cf_rpath_dir in \
					/usr/local \
					/usr/pkg \
					/opt/sfw
				do
					if test -f "$cf_rpath_dir/lib/$cf_rpath_src"
					then
						CF_VERBOSE(...adding -L$cf_rpath_dir/lib to LDFLAGS for $cf_rpath_src)
						LDFLAGS="$LDFLAGS -L$cf_rpath_dir/lib"
						break
					fi
				done
			done
		fi
	fi

	CF_VERBOSE(...checking EXTRA_LDFLAGS $EXTRA_LDFLAGS)

	CF_RPATH_HACK_2(LDFLAGS)
	CF_RPATH_HACK_2(LIBS)

	CF_VERBOSE(...checked EXTRA_LDFLAGS $EXTRA_LDFLAGS)
else
	AC_MSG_RESULT(no)
fi
AC_SUBST(EXTRA_LDFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RPATH_HACK_2 version: 8 updated: 2021/01/01 13:31:04
dnl ---------------
dnl Do one set of substitutions for CF_RPATH_HACK, adding an rpath option to
dnl EXTRA_LDFLAGS for each -L option found.
dnl
dnl $cf_rpath_list contains a list of directories to ignore.
dnl
dnl $1 = variable name to update.  The LDFLAGS variable should be the only one,
dnl      but LIBS often has misplaced -L options.
AC_DEFUN([CF_RPATH_HACK_2],
[
CF_VERBOSE(...checking $1 [$]$1)

cf_rpath_dst=
for cf_rpath_src in [$]$1
do
	case "$cf_rpath_src" in
	(-L*)

		# check if this refers to a directory which we will ignore
		cf_rpath_skip=no
		if test -n "$cf_rpath_list"
		then
			for cf_rpath_item in $cf_rpath_list
			do
				if test "x$cf_rpath_src" = "x-L$cf_rpath_item"
				then
					cf_rpath_skip=yes
					break
				fi
			done
		fi

		if test "$cf_rpath_skip" = no
		then
			# transform the option
			if test "$LD_RPATH_OPT" = "-R " ; then
				cf_rpath_tmp=`echo "$cf_rpath_src" |sed -e "s%-L%-R %"`
			else
				cf_rpath_tmp=`echo "$cf_rpath_src" |sed -e "s%-L%$LD_RPATH_OPT%"`
			fi

			# if we have not already added this, add it now
			cf_rpath_tst=`echo "$EXTRA_LDFLAGS" | sed -e "s%$cf_rpath_tmp %%"`
			if test "x$cf_rpath_tst" = "x$EXTRA_LDFLAGS"
			then
				CF_VERBOSE(...Filter $cf_rpath_src ->$cf_rpath_tmp)
				EXTRA_LDFLAGS="$cf_rpath_tmp $EXTRA_LDFLAGS"
			fi
		fi
		;;
	esac
	cf_rpath_dst="$cf_rpath_dst $cf_rpath_src"
done
$1=$cf_rpath_dst

CF_VERBOSE(...checked $1 [$]$1)
AC_SUBST(EXTRA_LDFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SAVE_XTRA_FLAGS version: 1 updated: 2020/01/11 16:46:44
dnl ------------------
dnl Use this macro to save CFLAGS/CPPFLAGS/LIBS before checks against X headers
dnl and libraries which do not update those variables.
dnl
dnl $1 = name of current macro
define([CF_SAVE_XTRA_FLAGS],
[
cf_save_LIBS_$1="$LIBS"
cf_save_CFLAGS_$1="$CFLAGS"
cf_save_CPPFLAGS_$1="$CPPFLAGS"
LIBS="$LIBS ${X_PRE_LIBS} ${X_LIBS} ${X_EXTRA_LIBS}"
for cf_X_CFLAGS in $X_CFLAGS
do
	case "x$cf_X_CFLAGS" in
	x-[[IUD]]*)
		CPPFLAGS="$CPPFLAGS $cf_X_CFLAGS"
		;;
	*)
		CFLAGS="$CFLAGS $cf_X_CFLAGS"
		;;
	esac
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SETITIMER version: 1 updated: 2023/01/25 19:55:24
dnl ------------
AC_DEFUN([CF_SETITIMER],
[
AC_CACHE_CHECK(if setitimer is available,cf_cv_func_setitimer,[
AC_TRY_COMPILE([
$ac_includes_default
#include <sys/time.h>
],[
    struct itimerval it;
    memset(&it, 0, sizeof(struct itimerval));
    it.it_value.tv_sec = 1;
    setitimer(ITIMER_REAL, &it, NULL);
],
[cf_cv_func_setitimer=yes],
[cf_cv_func_setitimer=no])
])
test "$cf_cv_func_setitimer" != no && AC_DEFINE(HAVE_SETITIMER,1,[Define to 1 if setitimer is available])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SIGWINCH version: 7 updated: 2023/02/18 17:41:25
dnl -----------
dnl Use this macro after CF_XOPEN_SOURCE, but do not require it (not all
dnl programs need this test).
dnl
dnl This is really a Mac OS X 10.4.3 workaround.  Defining _POSIX_C_SOURCE
dnl forces SIGWINCH to be undefined (breaks xterm, ncurses).  Oddly, the struct
dnl winsize declaration is left alone - we may revisit this if Apple choose to
dnl break that part of the interface as well.
AC_DEFUN([CF_SIGWINCH],
[
AC_CACHE_CHECK(if SIGWINCH is defined,cf_cv_define_sigwinch,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/signal.h>
],[int x = SIGWINCH; (void)x],
	[cf_cv_define_sigwinch=yes],
	[AC_TRY_COMPILE([
#undef _XOPEN_SOURCE
#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <sys/signal.h>
],[int x = SIGWINCH; (void)x],
	[cf_cv_define_sigwinch=maybe],
	[cf_cv_define_sigwinch=no])
])
])

if test "$cf_cv_define_sigwinch" = maybe ; then
AC_CACHE_CHECK(for actual SIGWINCH definition,cf_cv_fixup_sigwinch,[
cf_cv_fixup_sigwinch=unknown
cf_sigwinch=32
while test "$cf_sigwinch" != 1
do
	AC_TRY_COMPILE([
#undef _XOPEN_SOURCE
#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <sys/signal.h>
],[
#if SIGWINCH != $cf_sigwinch
#error SIGWINCH is not $cf_sigwinch
#endif
int x = SIGWINCH; (void)x],
	[cf_cv_fixup_sigwinch=$cf_sigwinch
	 break])

cf_sigwinch="`expr "$cf_sigwinch" - 1`"
done
])

	if test "$cf_cv_fixup_sigwinch" != unknown ; then
		CPPFLAGS="$CPPFLAGS -DSIGWINCH=$cf_cv_fixup_sigwinch"
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SIG_ATOMIC_T version: 5 updated: 2020/03/10 18:53:47
dnl ---------------
dnl signal handler, but there are some gcc dependencies in that recommendation.
dnl Try anyway.
AC_DEFUN([CF_SIG_ATOMIC_T],
[
AC_MSG_CHECKING(for signal global datatype)
AC_CACHE_VAL(cf_cv_sig_atomic_t,[
	for cf_type in \
		"volatile sig_atomic_t" \
		"sig_atomic_t" \
		"int"
	do
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

extern $cf_type x;
$cf_type x;
static void handler(int sig)
{
	(void)sig;
	x = 5;
}],
		[signal(SIGINT, handler);
		 x = 1],
		[cf_cv_sig_atomic_t=$cf_type],
		[cf_cv_sig_atomic_t=no])
		test "$cf_cv_sig_atomic_t" != no && break
	done
	])
AC_MSG_RESULT($cf_cv_sig_atomic_t)
test "$cf_cv_sig_atomic_t" != no && AC_DEFINE_UNQUOTED(SIG_ATOMIC_T, $cf_cv_sig_atomic_t,[Define to signal global datatype])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STRUCT_LASTLOG version: 4 updated: 2023/01/05 17:56:31
dnl -----------------
dnl Check for header defining struct lastlog, ensure that its .ll_time member
dnl is compatible with time().
AC_DEFUN([CF_STRUCT_LASTLOG],
[
AC_CHECK_HEADERS(lastlog.h)
AC_CACHE_CHECK(for struct lastlog,cf_cv_struct_lastlog,[
AC_TRY_RUN([
$ac_includes_default

#include <time.h>
#include <lastlog.h>

int main(void)
{
	struct lastlog data;
	return (sizeof(data.ll_time) != sizeof(time_t));
}],[
cf_cv_struct_lastlog=yes],[
cf_cv_struct_lastlog=no],[
cf_cv_struct_lastlog=unknown])])

test $cf_cv_struct_lastlog != no && AC_DEFINE(USE_STRUCT_LASTLOG,1,[Define to 1 if we have struct lastlog])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SVR4 version: 6 updated: 2023/12/01 17:22:50
dnl -------
dnl Check if this is an SVR4 system.  We need the definition for xterm
AC_DEFUN([CF_SVR4],
[
AC_CHECK_LIB(elf, elf_begin,[
AC_CACHE_CHECK(if this is an SVR4 system, cf_cv_svr4,[
AC_TRY_COMPILE([
#if defined(__CYGWIN__)
#error Cygwin is not SVr4
#endif
#include <elf.h>
#include <sys/termio.h>
],[
static struct termio d_tio;
	d_tio.c_cc[VINTR] = 0;
	d_tio.c_cc[VQUIT] = 0;
	d_tio.c_cc[VERASE] = 0;
	d_tio.c_cc[VKILL] = 0;
	d_tio.c_cc[VEOF] = 0;
	d_tio.c_cc[VEOL] = 0;
	d_tio.c_cc[VMIN] = 0;
	d_tio.c_cc[VTIME] = 0;
	d_tio.c_cc[VLNEXT] = 0;
],
[cf_cv_svr4=yes],
[cf_cv_svr4=no])
])
])
test "$cf_cv_svr4" = yes && AC_DEFINE(SVR4,1,[Define to 1 if this is an SVR4 system])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYSV version: 16 updated: 2020/01/11 16:46:44
dnl -------
dnl Check if this is a SYSV platform, e.g., as used in <X11/Xos.h>, and whether
dnl defining it will be helpful.  The following features are used to check:
dnl
dnl a) bona-fide SVSV doesn't use const for sys_errlist[].  Since this is a
dnl legacy (pre-ANSI) feature, const should not apply.  Modern systems only
dnl declare strerror().  Xos.h declares the legacy form of str_errlist[], and
dnl a compile-time error will result from trying to assign to a const array.
dnl
dnl b) compile with headers that exist on SYSV hosts.
dnl
dnl c) compile with type definitions that differ on SYSV hosts from standard C.
AC_DEFUN([CF_SYSV],
[
AC_REQUIRE([AC_PATH_XTRA])

CF_SAVE_XTRA_FLAGS([CF_SYSV])

AC_CHECK_HEADERS( \
termios.h \
stdlib.h \
X11/Intrinsic.h \
)

AC_REQUIRE([CF_SYS_ERRLIST])

AC_CACHE_CHECK(if we should define SYSV,cf_cv_sysv,[
AC_TRY_COMPILE([
#undef  SYSV
#define SYSV 1			/* get Xos.h to declare sys_errlist[] */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>		/* look for wchar_t */
#endif
#ifdef HAVE_X11_INTRINSIC_H
#include <X11/Intrinsic.h>	/* Intrinsic.h has other traps... */
#endif
#ifdef HAVE_TERMIOS_H		/* needed for HPUX 10.20 */
#include <termios.h>
#define STRUCT_TERMIOS struct termios
#else
#define STRUCT_TERMIOS struct termio
#endif
#include <curses.h>
#include <term.h>		/* eliminate most BSD hacks */
#include <errno.h>		/* declare sys_errlist on older systems */
#include <sys/termio.h>		/* eliminate most of the remaining ones */
],[
static STRUCT_TERMIOS d_tio;
	d_tio.c_cc[VINTR] = 0;
	d_tio.c_cc[VQUIT] = 0;
	d_tio.c_cc[VERASE] = 0;
	d_tio.c_cc[VKILL] = 0;
	d_tio.c_cc[VEOF] = 0;
	d_tio.c_cc[VEOL] = 0;
	d_tio.c_cc[VMIN] = 0;
	d_tio.c_cc[VTIME] = 0;
#if defined(HAVE_SYS_ERRLIST) && !defined(DECL_SYS_ERRLIST)
sys_errlist[0] = "";		/* Cygwin mis-declares this */
#endif
],
[cf_cv_sysv=yes],
[cf_cv_sysv=no])
])
test "$cf_cv_sysv" = yes && AC_DEFINE(SYSV,1,[Define to 1 if this is an SYSV system])

CF_RESTORE_XTRA_FLAGS([CF_SYSV])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYSV_UTMP version: 7 updated: 2021/01/02 09:31:20
dnl ------------
dnl Check if this is a SYSV flavor of UTMP
AC_DEFUN([CF_SYSV_UTMP],
[
AC_CACHE_CHECK(if $cf_cv_have_utmp is SYSV flavor,cf_cv_sysv_utmp,[
test "$cf_cv_have_utmp" = "utmp" && cf_prefix="ut" || cf_prefix="utx"
AC_TRY_LINK([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],[
struct $cf_cv_have_utmp x;
	set${cf_prefix}ent ();
	get${cf_prefix}id(&x);
	put${cf_prefix}line(&x);
	end${cf_prefix}ent();],
	[cf_cv_sysv_utmp=yes],
	[cf_cv_sysv_utmp=no])
])
test "$cf_cv_sysv_utmp" = yes && AC_DEFINE(USE_SYSV_UTMP,1,[Define to 1 if utmp is SYSV flavor])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYS_ERRLIST version: 6 updated: 2001/12/30 13:03:23
dnl --------------
dnl Check for declaration of sys_nerr and sys_errlist in one of stdio.h and
dnl errno.h.  Declaration of sys_errlist on BSD4.4 interferes with our
dnl declaration.  Reported by Keith Bostic.
AC_DEFUN([CF_SYS_ERRLIST],
[
    CF_CHECK_ERRNO(sys_nerr)
    CF_CHECK_ERRNO(sys_errlist)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TERMIOS_TYPES version: 2 updated: 2020/03/10 18:53:47
dnl ----------------
dnl https://pubs.opengroup.org/onlinepubs/009695399/basedefs/termios.h.html
dnl says that tcflag_t, speed_t and cc_t are typedef'd.  If they are not,
dnl fallback to historical values.
AC_DEFUN([CF_TERMIOS_TYPES],[

AC_CACHE_CHECK(for termios type tcflag_t, cf_cv_havetype_tcflag_t,[
	AC_TRY_COMPILE([#include <termios.h>],[
		tcflag_t x = 0; (void)x],
		[cf_cv_havetype_tcflag_t=yes],
		[cf_cv_havetype_tcflag_t=no])
])
test "$cf_cv_havetype_tcflag_t" = no && AC_DEFINE(tcflag_t,unsigned long,[Define usable value of tcflag_t if not declared])

AC_CACHE_CHECK(for termios type speed_t, cf_cv_havetype_speed_t,[
	AC_TRY_COMPILE([#include <termios.h>],[
		speed_t x = 0; (void)x],
		[cf_cv_havetype_speed_t=yes],
		[cf_cv_havetype_speed_t=no])
])
test "$cf_cv_havetype_speed_t" = no && AC_DEFINE(speed_t,unsigned short,[Define usable value of speed_t if not declared])

AC_CACHE_CHECK(for termios type cc_t, cf_cv_havetype_cc_t,[
	AC_TRY_COMPILE([#include <termios.h>],[
		cc_t x = 0; (void)x],
		[cf_cv_havetype_cc_t=yes],
		[cf_cv_havetype_cc_t=no])
])
test "$cf_cv_havetype_cc_t" = no && AC_DEFINE(cc_t,unsigned char,[Define usable value of cc_t if not declared])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TERMIO_C_ISPEED version: 4 updated: 2020/03/10 18:53:47
dnl ------------------
dnl Check for SGI's broken redefinition of baud rates introduced in IRIX 6.5
dnl (there doesn't appear to be a useful predefined symbol).
AC_DEFUN([CF_TERMIO_C_ISPEED],
[
AC_CACHE_CHECK(for IRIX 6.5 baud-rate redefinitions,cf_cv_termio_c_ispeed,[
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/termio.h>],[
struct termio foo;
foo.c_ispeed = B38400;
foo.c_ospeed = B9600;
(void)foo;
],[cf_cv_termio_c_ispeed=yes
],[cf_cv_termio_c_ispeed=no])
])
test "$cf_cv_termio_c_ispeed" = yes && AC_DEFINE(HAVE_TERMIO_C_ISPEED,1,[define 1 if we have IRIX 6.5 baud-rate redefinitions])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TRIM_X_LIBS version: 3 updated: 2015/04/12 15:39:00
dnl --------------
dnl Trim extra base X libraries added as a workaround for inconsistent library
dnl dependencies returned by "new" pkg-config files.
AC_DEFUN([CF_TRIM_X_LIBS],[
	for cf_trim_lib in Xmu Xt X11
	do
		case "$LIBS" in
		(*-l$cf_trim_lib\ *-l$cf_trim_lib*)
			LIBS=`echo "$LIBS " | sed -e 's/  / /g' -e 's%-l'"$cf_trim_lib"' %%' -e 's/ $//'`
			CF_VERBOSE(..trimmed $LIBS)
			;;
		esac
	done
])
dnl ---------------------------------------------------------------------------
dnl CF_TRY_PKG_CONFIG version: 7 updated: 2025/01/10 19:55:54
dnl -----------------
dnl This is a simple wrapper to use for pkg-config, for libraries which may be
dnl available in that form.
dnl
dnl $1 = package name, which may be a shell variable
dnl $2 = extra logic to use, if any, after updating CFLAGS and LIBS
dnl $3 = logic to use if pkg-config does not have the package
AC_DEFUN([CF_TRY_PKG_CONFIG],[
AC_REQUIRE([CF_PKG_CONFIG])

if test "$PKG_CONFIG" != none && "$PKG_CONFIG" --exists "$1"; then
	CF_VERBOSE(found package $1)
	cf_pkgconfig_incs="`$PKG_CONFIG --cflags "$1" 2>/dev/null`"
	cf_pkgconfig_libs="`$PKG_CONFIG --libs   "$1" 2>/dev/null`"
	CF_VERBOSE(package $1 CFLAGS: $cf_pkgconfig_incs)
	CF_VERBOSE(package $1 LIBS: $cf_pkgconfig_libs)
	CF_REQUIRE_PKG($1)
	CF_ADD_CFLAGS($cf_pkgconfig_incs)
	CF_ADD_LIBS($cf_pkgconfig_libs)
	ifelse([$2],,:,[$2])
else
	cf_pkgconfig_incs=
	cf_pkgconfig_libs=
	ifelse([$3],,:,[$3])
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_TRY_XOPEN_SOURCE version: 4 updated: 2022/09/10 15:16:16
dnl -------------------
dnl If _XOPEN_SOURCE is not defined in the compile environment, check if we
dnl can define it successfully.
AC_DEFUN([CF_TRY_XOPEN_SOURCE],[
AC_CACHE_CHECK(if we should define _XOPEN_SOURCE,cf_cv_xopen_source,[
	AC_TRY_COMPILE(CF__XOPEN_SOURCE_HEAD,CF__XOPEN_SOURCE_BODY,
	[cf_cv_xopen_source=no],
	[cf_save="$CPPFLAGS"
	 CF_APPEND_TEXT(CPPFLAGS,-D_XOPEN_SOURCE=$cf_XOPEN_SOURCE)
	 AC_TRY_COMPILE(CF__XOPEN_SOURCE_HEAD,CF__XOPEN_SOURCE_BODY,
		[cf_cv_xopen_source=no],
		[cf_cv_xopen_source=$cf_XOPEN_SOURCE])
		CPPFLAGS="$cf_save"
	])
])

if test "$cf_cv_xopen_source" != no ; then
	CF_REMOVE_DEFINE(CFLAGS,$CFLAGS,_XOPEN_SOURCE)
	CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,_XOPEN_SOURCE)
	cf_temp_xopen_source="-D_XOPEN_SOURCE=$cf_cv_xopen_source"
	CF_APPEND_CFLAGS($cf_temp_xopen_source)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_TTY_GROUP version: 17 updated: 2023/12/01 17:22:50
dnl ------------
dnl Check if the system has a tty-group defined.  This is used in xterm when
dnl setting pty ownership.
dnl
dnl The check relies upon running a test-program, which calls ttyname.  If this
dnl is run in batch mode, or if autoconf uses shell functions, the extra layer
dnl of eval/call may close stdin, making calls to ttyname fail.  To work around
dnl that, fall back to "/dev/tty".
AC_DEFUN([CF_TTY_GROUP],
[AC_REQUIRE([AC_PROG_EGREP])dnl
AC_MSG_CHECKING(for explicit tty group name)
AC_ARG_WITH(tty-group,
	[[  --with-tty-group[=XXX]  use XXX for the tty-group]],
	[cf_tty_group=$withval],
	[cf_tty_group=auto...])
test -z "$cf_tty_group"    && cf_tty_group=auto...
test "$cf_tty_group" = yes && cf_tty_group=auto...
AC_MSG_RESULT($cf_tty_group)

if test "$cf_tty_group" = "auto..." ; then
AC_CACHE_CHECK(for tty group name,cf_cv_tty_group_name,[

# If we are configuring as root, it is hard to get a clue about the tty group.
# But we'll guess based on how our connection is set up - assuming it is done
# properly.

cf_uid="`id | sed -e 's/^[^=]*=//' -e 's/(.*$//'`"
# )vi
if test "$cf_uid" != 0 ; then
cf_cv_tty_group_name=
cf_tty_name="`tty`"
test "$cf_tty_name" = "not a tty" && cf_tty_name=/dev/tty
test -z "$cf_tty_name" && cf_tty_name=/dev/tty
if test -c "$cf_tty_name"
then
	cf_option="-lL"

	# Expect listing to have fields like this:
	#-rwxrwxrwx   1 user      group       34293 Jul 18 16:29 pathname
	ls $cf_option "$cf_tty_name" >conftest.out
	read cf_mode cf_links cf_usr cf_grp cf_size cf_date1 cf_date2 cf_date3 cf_rest <conftest.out
	if test -z "$cf_rest" ; then
		cf_option="${cf_option}g"
		ls "$cf_option" "$cf_tty_name" >conftest.out
		read cf_mode cf_links cf_usr cf_grp cf_size cf_date1 cf_date2 cf_date3 cf_rest <conftest.out
	fi
	rm -f conftest.out
	cf_cv_tty_group_name=$cf_grp
fi
fi

# If we cannot deduce the tty group, fall back on hardcoded cases

if test -z "$cf_cv_tty_group_name"
then
case $host_os in
(osf*)
	cf_cv_tty_group_name="terminal"
	;;
(*)
	cf_cv_tty_group_name="unknown"
	if ( ${EGREP-egrep} '^tty:' /etc/group 2>/dev/null 1>/dev/null ) then
		cf_cv_tty_group_name="tty"
	fi
	;;
esac
fi
])
cf_tty_group="$cf_cv_tty_group_name"
else
	# if configure option, always do this
	AC_DEFINE(USE_TTY_GROUP,1,[Define to 1 if we have a tty groupname])
fi

AC_DEFINE_UNQUOTED(TTY_GROUP_NAME,"$cf_tty_group",[Define to the name use for tty group])

# This is only a double-check that the group-name we obtained above really
# does apply to the device.  We cannot perform this test if we are in batch
# mode, or if we are cross-compiling.

AC_CACHE_CHECK(if we may use the $cf_tty_group group,cf_cv_tty_group,[
cf_tty_name="`tty`"
if test "$cf_tty_name" != "not a tty"
then
AC_TRY_RUN([
$ac_includes_default

#include <grp.h>

int main(void)
{
	static char default_tty[] = "/dev/tty";
	struct stat sb;
	struct group *ttygrp;
	int fd;
	char *name;

	for (fd = 0; fd < 3; ++fd) {
		if ((name = ttyname(fd)) != 0)
			break;
	}
	if (name == 0)
		name = default_tty;

	ttygrp = getgrnam(TTY_GROUP_NAME);
	endgrent();

	if (ttygrp != 0
	 && name != 0
	 && stat(name, &sb) == 0
	 && sb.st_gid != getgid()
	 && sb.st_gid == ttygrp->gr_gid) {
		${cf_cv_main_return:-return} (0);
	}
	${cf_cv_main_return:-return} (1);
}
	],
	[cf_cv_tty_group=yes],
	[cf_cv_tty_group=no],
	[cf_cv_tty_group=unknown])
elif test "$cross_compiling" = yes; then
	cf_cv_tty_group=unknown
else
	cf_cv_tty_group=yes
fi
])

if test $cf_cv_tty_group = no ; then
	AC_MSG_WARN(Cannot use $cf_tty_group group)
else
	AC_DEFINE(USE_TTY_GROUP)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TYPE_CC_T version: 3 updated: 2020/03/10 18:53:47
dnl ------------
dnl	Check for cc_t type, used in termio.
AC_DEFUN([CF_TYPE_CC_T],
[
AC_MSG_CHECKING(for cc_t in <termios.h> or <termio.h>)
AC_CACHE_VAL(cf_cv_type_cc_t,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#if defined(HAVE_TERMIOS_H)
#include <termios.h>
#else
#include <termio.h>
#include <sys/ioctl.h>
#endif
],
		[cc_t x; (void)x],
		[cf_cv_type_cc_t=yes],
		[cf_cv_type_cc_t=no])
	])
AC_MSG_RESULT($cf_cv_type_cc_t)
test $cf_cv_type_cc_t = no && AC_DEFINE(cc_t, unsigned char,[Define to cc_t type used in termio])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TYPE_FD_MASK version: 3 updated: 2012/10/04 06:57:36
dnl ---------------
dnl Check for the declaration of fd_mask, which is like fd_set, associated
dnl with select().  The check for fd_set should have pulled in this as well,
dnl but there is a special case for Mac OS X, possibly other BSD-derived
dnl platforms.
AC_DEFUN([CF_TYPE_FD_MASK],
[
AC_REQUIRE([CF_TYPE_FD_SET])

AC_CACHE_CHECK(for declaration of fd_mask,cf_cv_type_fd_mask,[
    if test x$cf_cv_type_fd_set = xX11/Xpoll.h ; then
        AC_TRY_COMPILE([
#include <X11/Xpoll.h>],[fd_mask x],,
        [CF_MSG_LOG(if we must define CSRG_BASED)
# Xosdefs.h on Mac OS X may not define this (but it should).
            AC_TRY_COMPILE([
#define CSRG_BASED
#include <X11/Xpoll.h>],[fd_mask x],
        cf_cv_type_fd_mask=CSRG_BASED)])
    else
        cf_cv_type_fd_mask=$cf_cv_type_fd_set
    fi
])
if test x$cf_cv_type_fd_mask = xCSRG_BASED ; then
    AC_DEFINE(CSRG_BASED,1,[Define to 1 if needed for declaring fd_mask()])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TYPE_FD_SET version: 6 updated: 2020/03/10 18:53:47
dnl --------------
dnl Check for the declaration of fd_set.  Some platforms declare it in
dnl <sys/types.h>, and some in <sys/select.h>, which requires <sys/types.h>.
dnl Finally, if we are using this for an X application, Xpoll.h may include
dnl <sys/select.h>, so we don't want to do it twice.
AC_DEFUN([CF_TYPE_FD_SET],
[
AC_CHECK_HEADERS(X11/Xpoll.h)

AC_CACHE_CHECK(for declaration of fd_set,cf_cv_type_fd_set,
	[CF_MSG_LOG(sys/types alone)
AC_TRY_COMPILE([
#include <sys/types.h>],
	[fd_set x; (void)x],
	[cf_cv_type_fd_set=sys/types.h],
	[CF_MSG_LOG(X11/Xpoll.h)
AC_TRY_COMPILE([
#ifdef HAVE_X11_XPOLL_H
#include <X11/Xpoll.h>
#endif],
	[fd_set x; (void)x],
	[cf_cv_type_fd_set=X11/Xpoll.h],
	[CF_MSG_LOG(sys/select.h)
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/select.h>],
	[fd_set x; (void)x],
	[cf_cv_type_fd_set=sys/select.h],
	[cf_cv_type_fd_set=unknown])])])])
if test $cf_cv_type_fd_set = sys/select.h ; then
	AC_DEFINE(USE_SYS_SELECT_H,1,[Define to 1 to include sys/select.h to declare fd_set])
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_TYPE_NFDS_T version: 1 updated: 2023/02/26 20:20:10
dnl --------------
dnl Check for nfds_t type, used in poll.
AC_DEFUN([CF_TYPE_NFDS_T],
[
AC_MSG_CHECKING(for nfds_t in <poll.h>)
AC_CACHE_VAL(cf_cv_type_nfds_t,[
	AC_TRY_COMPILE([
$ac_includes_default
#include <poll.h>
],
		[nfds_t x; (void)x],
		[cf_cv_type_nfds_t=yes],
		[cf_cv_type_nfds_t=no])
	])
AC_MSG_RESULT($cf_cv_type_nfds_t)
test $cf_cv_type_nfds_t = no && AC_DEFINE(nfds_t, unsigned,[Define to nfds_t type used in poll])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UNDO_CFLAGS version: 1 updated: 2011/07/02 09:27:51
dnl --------------
dnl Remove flags from $CFLAGS or similar shell variable using sed.
dnl $1 = variable
dnl $2 = message
dnl $3 = pattern to remove
AC_DEFUN([CF_UNDO_CFLAGS],
[
	CF_VERBOSE(removing $2 flags from $1)
	$1=`echo "[$]$1" | sed -e 's/$3//'`
	CF_VERBOSE(...result [$]$1)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UPPER version: 5 updated: 2001/01/29 23:40:59
dnl --------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
$1=`echo "$2" | sed y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTEMPTER version: 4 updated: 2012/10/04 20:12:20
dnl -----------
dnl Try to link with utempter library
AC_DEFUN([CF_UTEMPTER],
[
AC_CACHE_CHECK(if we can link with utempter library,cf_cv_have_utempter,[
cf_save_LIBS="$LIBS"
CF_ADD_LIB(utempter)
AC_TRY_LINK([
#include <utempter.h>
],[
	addToUtmp("/dev/tty", 0, 1);
	removeFromUtmp();
],[
	cf_cv_have_utempter=yes],[
	cf_cv_have_utempter=no])
LIBS="$cf_save_LIBS"
])
if test "$cf_cv_have_utempter" = yes ; then
	AC_DEFINE(USE_UTEMPTER,1,[Define to 1 if we can/should link with utempter])
	CF_ADD_LIB(utempter)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP version: 12 updated: 2021/01/02 09:31:20
dnl -------
dnl Check for UTMP/UTMPX headers
AC_DEFUN([CF_UTMP],
[
AC_REQUIRE([CF_LASTLOG])

AC_CACHE_CHECK(for utmp implementation,cf_cv_have_utmp,[
	cf_cv_have_utmp=no
for cf_header in utmpx utmp ; do
cf_utmp_includes="
#include <sys/types.h>
#include <${cf_header}.h>
#define getutent getutxent
#ifdef USE_LASTLOG
#include <lastlog.h>	/* may conflict with utmpx.h on Linux */
#endif
"
	AC_TRY_COMPILE([$cf_utmp_includes],
	[struct $cf_header x;
	 char *name = x.ut_name; /* utmp.h and compatible definitions */
	 (void)x;
	 (void)name;
	],
	[cf_cv_have_utmp=$cf_header
	 break],
	[
	AC_TRY_COMPILE([$cf_utmp_includes],
	[struct $cf_header x;
	 char *name = x.ut_user; /* utmpx.h must declare this */
	 (void)x;
	 (void)name;
	],
	[cf_cv_have_utmp=$cf_header
	 break
	])])
done
])

if test "$cf_cv_have_utmp" != no ; then
	AC_DEFINE(HAVE_UTMP,1,[Define to 1 if the utmp interface is available])
	test "$cf_cv_have_utmp" = utmpx && AC_DEFINE(UTMPX_FOR_UTMP,1,[Define if we have utmpx interface])
	CF_UTMP_UT_HOST
	CF_UTMP_UT_SYSLEN
	CF_UTMP_UT_NAME
	CF_UTMP_UT_XSTATUS
	CF_UTMP_UT_XTIME
	CF_UTMP_UT_SESSION
	CF_SYSV_UTMP
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_GROUP version: 2 updated: 2020/12/31 10:54:15
dnl -------------
dnl Find the utmp/utmpx file and determine its group to allow setgid programs
dnl to manipulate it, e.g., when there is no intermediary.
AC_DEFUN([CF_UTMP_GROUP],[
AC_REQUIRE([CF_UTMP])
if test $cf_cv_have_utmp != no ; then
AC_CACHE_CHECK(for utmp/utmpx group,cf_cv_utmp_group,[
for cf_utmp_path in /var/adm /var/run
do
	for cf_utmp_file in utmpx utmp
	do
		if test -f $cf_utmp_path/$cf_utmp_file
		then
			cf_cv_utmp_group=root

			cf_option="-lL"

			# Expect listing to have fields like this:
			#-r--r--r--   1 user      group       34293 Jul 18 16:29 pathname
			ls "$cf_option" "$cf_utmp_path/$cf_utmp_file" >conftest
			read cf_mode cf_links cf_usr cf_grp cf_size cf_date1 cf_date2 cf_date3 cf_rest <conftest
			if test -z "$cf_rest" ; then
				cf_option="${cf_option}g"
				ls "$cf_option" "$cf_utmp_path/$cf_utmp_file" >conftest
				read cf_mode cf_links cf_usr cf_grp cf_size cf_date1 cf_date2 cf_date3 cf_rest <conftest
			fi
			rm -f conftest

			# If we have a pathname, and the date fields look right, assume we've
			# captured the group as well.
			if test -n "$cf_rest" ; then
				cf_test=`echo "${cf_date2}${cf_date3}" | sed -e 's/[[0-9:]]//g'`
				if test -z "$cf_test" ; then
					cf_cv_utmp_group=$cf_grp;
				fi
			fi
			break
		fi
	done
	test -n "$cf_cv_utmp_group" && break
done
])
else
	AC_MSG_ERROR(cannot find utmp group)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_HOST version: 10 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Check if UTMP/UTMPX struct defines ut_host member
AC_DEFUN([CF_UTMP_UT_HOST],
[
if test "$cf_cv_have_utmp" != no ; then
AC_MSG_CHECKING(if ${cf_cv_have_utmp}.ut_host is declared)
AC_CACHE_VAL(cf_cv_have_utmp_ut_host,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[struct $cf_cv_have_utmp x;
	 char *y = &x.ut_host[0];
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_host=yes],
	[cf_cv_have_utmp_ut_host=no])
	])
AC_MSG_RESULT($cf_cv_have_utmp_ut_host)
test "$cf_cv_have_utmp_ut_host" != no && AC_DEFINE(HAVE_UTMP_UT_HOST,1,[Define to 1 if UTMP/UTMPX struct defines ut_host member])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_NAME version: 8 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Check if UTMP/UTMPX struct defines ut_name member
AC_DEFUN([CF_UTMP_UT_NAME],
[
if test "$cf_cv_have_utmp" != no ; then
AC_CACHE_CHECK(if ${cf_cv_have_utmp}.ut_name is declared,cf_cv_have_utmp_ut_name,[
	cf_cv_have_utmp_ut_name=no
cf_utmp_includes="
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>
#define getutent getutxent
#ifdef USE_LASTLOG
#include <lastlog.h>		/* may conflict with utmpx.h on Linux */
#endif
"
for cf_header in ut_name ut_user ; do
	AC_TRY_COMPILE([$cf_utmp_includes],
	[struct $cf_cv_have_utmp x;
	 char *name = x.$cf_header;
	 (void)x;
	 (void)name;
	],
	[cf_cv_have_utmp_ut_name=$cf_header
	 break])
done
])

case "$cf_cv_have_utmp_ut_name" in
(no)
	AC_MSG_ERROR(Cannot find declaration for ut.ut_name)
	;;
(ut_user)
	AC_DEFINE(ut_name,ut_user,[Define to rename UTMP/UTMPX struct ut_name member])
	;;
esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_SESSION version: 9 updated: 2021/06/07 17:39:17
dnl ------------------
dnl Check if UTMP/UTMPX struct defines ut_session member
AC_DEFUN([CF_UTMP_UT_SESSION],
[
if test "$cf_cv_have_utmp" != no ; then
AC_CACHE_CHECK(if ${cf_cv_have_utmp}.ut_session is declared, cf_cv_have_utmp_ut_session,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[static struct $cf_cv_have_utmp x;
	 long y = x.ut_session;
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_session=yes],
	[cf_cv_have_utmp_ut_session=no])
])
if test "$cf_cv_have_utmp_ut_session" != no ; then
	AC_DEFINE(HAVE_UTMP_UT_SESSION,1,[Define to 1 if UTMP/UTMPX struct defines ut_session member])
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_SYSLEN version: 4 updated: 2021/01/02 09:31:20
dnl -----------------
dnl Check if UTMP/UTMPX struct defines ut_syslen member
AC_DEFUN([CF_UTMP_UT_SYSLEN],
[
if test "$cf_cv_have_utmp" != no ; then
AC_MSG_CHECKING(if ${cf_cv_have_utmp}.ut_syslen is declared)
AC_CACHE_VAL(cf_cv_have_utmp_ut_syslen,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[struct $cf_cv_have_utmp x;
	 int y = x.ut_syslen;
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_syslen=yes],
	[cf_cv_have_utmp_ut_syslen=no])
	])
AC_MSG_RESULT($cf_cv_have_utmp_ut_syslen)
test "$cf_cv_have_utmp_ut_syslen" != no && AC_DEFINE(HAVE_UTMP_UT_SYSLEN,1,[Define to 1 if UTMP/UTMPX struct defines ut_syslen member])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_XSTATUS version: 6 updated: 2021/01/02 09:31:20
dnl ------------------
dnl Check for known variants on the UTMP/UTMPX struct's exit-status as reported
dnl by various people:
dnl
dnl	ut_exit.__e_exit (HPUX 11 - David Ellement, also in glibc2)
dnl	ut_exit.e_exit (SVR4)
dnl	ut_exit.ut_e_exit (os390 - Greg Smith)
dnl	ut_exit.ut_exit (Tru64 4.0f - Jeremie Petit, 4.0e - Tomas Vanhala)
dnl
dnl Note: utmp_xstatus is not a conventional compatibility definition in the
dnl system header files.
AC_DEFUN([CF_UTMP_UT_XSTATUS],
[
if test "$cf_cv_have_utmp" != no ; then
AC_CACHE_CHECK(for exit-status in $cf_cv_have_utmp,cf_cv_have_utmp_ut_xstatus,[
for cf_result in \
	ut_exit.__e_exit \
	ut_exit.e_exit \
	ut_exit.ut_e_exit \
	ut_exit.ut_exit
do
AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[struct $cf_cv_have_utmp x;
	 long y = x.$cf_result = 0;
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_xstatus=$cf_result
	 break],
	[cf_cv_have_utmp_ut_xstatus=no])
done
])
if test "$cf_cv_have_utmp_ut_xstatus" != no ; then
	AC_DEFINE(HAVE_UTMP_UT_XSTATUS,1,[Define to 1 if UTMP/UTMPX has exit-status member])
	AC_DEFINE_UNQUOTED(ut_xstatus,$cf_cv_have_utmp_ut_xstatus,[Define if needed to rename member ut_xstatus of UTMP/UTMPX])
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_XTIME version: 11 updated: 2021/01/02 09:31:20
dnl ----------------
dnl Check if UTMP/UTMPX struct defines ut_xtime member
AC_DEFUN([CF_UTMP_UT_XTIME],
[
if test "$cf_cv_have_utmp" != no ; then
AC_CACHE_CHECK(if ${cf_cv_have_utmp}.ut_xtime is declared, cf_cv_have_utmp_ut_xtime,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[struct $cf_cv_have_utmp x;
	 long y = x.ut_xtime = 0;
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_xtime=yes],
	[AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[struct $cf_cv_have_utmp x;
	 long y = x.ut_tv.tv_sec;
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_xtime=define],
	[cf_cv_have_utmp_ut_xtime=no])
	])
])
if test "$cf_cv_have_utmp_ut_xtime" != no ; then
	AC_DEFINE(HAVE_UTMP_UT_XTIME,1,[Define to 1 if UTMP/UTMPX struct defines ut_xtime member])
	if test "$cf_cv_have_utmp_ut_xtime" = define ; then
		AC_DEFINE(ut_xtime,ut_tv.tv_sec,[Define if needed to alternate name for utmpx.ut_xtime member])
	fi
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_VERBOSE version: 3 updated: 2007/07/29 09:55:12
dnl ----------
dnl Use AC_VERBOSE w/o the warnings
AC_DEFUN([CF_VERBOSE],
[test -n "$verbose" && echo "	$1" 1>&AC_FD_MSG
CF_MSG_LOG([$1])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_APP_CLASS version: 3 updated: 2015/04/12 15:39:00
dnl -----------------
dnl Handle configure option "--with-app-class", setting the $APP_CLASS
dnl variable, used for X resources.
dnl
dnl $1 = default value.
AC_DEFUN([CF_WITH_APP_CLASS],[
AC_MSG_CHECKING(for X applications class)
AC_ARG_WITH(app-class,
	[  --with-app-class=XXX    override X applications class (default $1)],
	[APP_CLASS=$withval],
	[APP_CLASS=$1])

case x$APP_CLASS in
(*[[/@,%]]*)
	AC_MSG_WARN(X applications class cannot contain punctuation)
	APP_CLASS=$1
	;;
(x[[A-Z]]*)
	;;
(*)
	AC_MSG_WARN([X applications class must start with capital, ignoring $APP_CLASS])
	APP_CLASS=$1
	;;
esac

AC_MSG_RESULT($APP_CLASS)

AC_SUBST(APP_CLASS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_APP_DEFAULTS version: 6 updated: 2015/01/02 09:05:50
dnl --------------------
dnl Handle configure option "--with-app-defaults", setting these shell
dnl variables:
dnl
dnl $APPSDIR is the option value, used for installing app-defaults files.
dnl $no_appsdir is a "#" (comment) if "--without-app-defaults" is given.
dnl
dnl Most Linux's use this:
dnl 	/usr/share/X11/app-defaults
dnl Debian uses this:
dnl 	/etc/X11/app-defaults
dnl DragonFlyBSD ports uses this:
dnl 	/usr/pkg/lib/X11/app-defaults
dnl FreeBSD ports use these:
dnl 	/usr/local/lib/X11/app-defaults
dnl 	/usr/local/share/X11/app-defaults
dnl Mandriva has these:
dnl 	/usr/lib/X11/app-defaults
dnl 	/usr/lib64/X11/app-defaults
dnl NetBSD has these
dnl 	/usr/X11R7/lib/X11/app-defaults
dnl OpenSolaris uses
dnl 	32-bit:
dnl 	/usr/X11/etc/X11/app-defaults
dnl 	/usr/X11/share/X11/app-defaults
dnl 	/usr/X11/lib/X11/app-defaults
dnl OSX uses
dnl		/opt/local/share/X11/app-defaults (MacPorts)
dnl		/opt/X11/share/X11/app-defaults (non-ports)
dnl	64-bit:
dnl 	/usr/X11/etc/X11/app-defaults
dnl 	/usr/X11/share/X11/app-defaults (I mkdir'd this)
dnl 	/usr/X11/lib/amd64/X11/app-defaults
dnl Solaris10 uses (in this order):
dnl 	/usr/openwin/lib/X11/app-defaults
dnl 	/usr/X11/lib/X11/app-defaults
AC_DEFUN([CF_WITH_APP_DEFAULTS],[
AC_MSG_CHECKING(for directory to install resource files)
AC_ARG_WITH(app-defaults,
	[  --with-app-defaults=DIR directory in which to install resource files (EPREFIX/lib/X11/app-defaults)],
	[APPSDIR=$withval],
	[APPSDIR='${exec_prefix}/lib/X11/app-defaults'])

if test "x[$]APPSDIR" = xauto
then
	APPSDIR='${exec_prefix}/lib/X11/app-defaults'
	for cf_path in \
		/opt/local/share/X11/app-defaults \
		/opt/X11/share/X11/app-defaults \
		/usr/share/X11/app-defaults \
		/usr/X11/share/X11/app-defaults \
		/usr/X11/lib/X11/app-defaults \
		/usr/lib/X11/app-defaults \
		/etc/X11/app-defaults \
		/usr/pkg/lib/X11/app-defaults \
		/usr/X11R7/lib/X11/app-defaults \
		/usr/X11R6/lib/X11/app-defaults \
		/usr/X11R5/lib/X11/app-defaults \
		/usr/X11R4/lib/X11/app-defaults \
		/usr/local/lib/X11/app-defaults \
		/usr/local/share/X11/app-defaults \
		/usr/lib64/X11/app-defaults
	do
		if test -d "$cf_path" ; then
			APPSDIR="$cf_path"
			break
		fi
	done
else
	cf_path=$APPSDIR
	CF_PATH_SYNTAX(cf_path)
fi

AC_MSG_RESULT($APPSDIR)
AC_SUBST(APPSDIR)

no_appsdir=
if test "$APPSDIR" = no
then
	no_appsdir="#"
else
	EXTRA_INSTALL_DIRS="$EXTRA_INSTALL_DIRS \$(APPSDIR)"
fi
AC_SUBST(no_appsdir)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DBMALLOC version: 7 updated: 2010/06/21 17:26:47
dnl ----------------
dnl Configure-option for dbmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DBMALLOC],[
CF_NO_LEAKS_OPTION(dbmalloc,
	[  --with-dbmalloc         test: use Conor Cahill's dbmalloc library],
	[USE_DBMALLOC])

if test "$with_dbmalloc" = yes ; then
	AC_CHECK_HEADER(dbmalloc.h,
		[AC_CHECK_LIB(dbmalloc,[debug_malloc]ifelse([$1],,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DESKTOP_CATEGORY version: 10 updated: 2023/12/02 06:55:40
dnl ------------------------
dnl Taking into account the absence of standardization of desktop categories
dnl take a look to see whether other applications on the current system are
dnl assigned any/all of a set of suggested categories.
dnl
dnl $1 = program name
dnl $2 = case-pattern to match comparable desktop files to obtain category
dnl      This pattern may contain wildcards.
dnl $3 = suggested categories, also a case-pattern but without wildcards,
dnl      since it doubles as a default value for a shell case-statement.
dnl $4 = categories to use if no match is found on the build-machine for the
dnl      --with-desktop-category "auto" setting.
dnl
dnl The macro tells the configure script to substitute the $DESKTOP_CATEGORY
dnl value.
AC_DEFUN([CF_WITH_DESKTOP_CATEGORY],[
AC_REQUIRE([AC_PROG_EGREP])dnl
AC_REQUIRE([CF_DISABLE_DESKTOP])dnl
if test -z "$desktop_utils"
then
	AC_MSG_CHECKING(for requested desktop-category)
	AC_ARG_WITH(desktop-category,
		[  --with-desktop-category=XXX  one or more desktop categories or auto],
		[cf_desktop_want=$withval],
		[cf_desktop_want=auto])
	AC_MSG_RESULT($cf_desktop_want)

	if test "$cf_desktop_want" = auto
	then
		rm -rf conftest*
		cf_desktop_also=
		for cf_desktop_dir in  \
			/usr/share/app-install \
			/usr/share/applications
		do
			if test -d $cf_desktop_dir
			then
				find $cf_desktop_dir -name '*.desktop' | \
				while true
				do
					read cf_desktop_path
					test -z "$cf_desktop_path" && break
					cf_desktop_name=`basename "$cf_desktop_path" .desktop`
					case $cf_desktop_name in
					($1|*-$1|$2)
						CF_VERBOSE(inspect $cf_desktop_path)
						${EGREP-egrep} '^Categories=' "$cf_desktop_path" | \
							tr ';' '\n' | \
							sed -e 's%^.*=%%' -e '/^$/d' >>conftest.1
						;;
					esac
				done
			fi
		done
		if test -s conftest.1
		then
			cf_desktop_last=
			sort conftest.1 | \
			while true
			do
				read cf_desktop_this
				test -z "$cf_desktop_this" && break
				if test -s conftest.2
				then
					grep -w "$cf_desktop_this" conftest.2 >/dev/null && continue
				elif test -s conftest.3
				then
					grep -w "$cf_desktop_this" conftest.3 >/dev/null && continue
				fi
				case "$cf_desktop_this" in
				(-*)
					;;
				(Qt*|*Xfce*|*[[ABCDEFGHIJKLMNOPQRSTUVWXYZ]][[ABCDEFGHIJKLMNOPQRSTUVWXYZ]]*)
					CF_VERBOSE(ignored $cf_desktop_this)
					echo "$cf_desktop_this" >> conftest.3
					;;
				($3)
					CF_VERBOSE(applied $cf_desktop_this)
					test "x$cf_desktop_last" != "x$cf_desktop_this" && echo "$cf_desktop_this" >>conftest.2
					;;
				esac
				cf_desktop_last=$cf_desktop_this
			done
			cf_desktop_want="`tr '\n' ';' < conftest.2`"
		fi
		if test -n "$cf_desktop_want"
		then
			if test "$cf_desktop_want" = auto
			then
				cf_desktop_want=
			else
				# do a sanity check on the semicolon-separated list, ignore on failure
				cf_desktop_test=`echo "$cf_desktop_want" | sed -e 's/[[^;]]//g'`
				test -z "$cf_desktop_test" && cf_desktop_want=
				cf_desktop_test=`echo "$cf_desktop_want" | sed -e 's/^.*;$/./g'`
				test -z "$cf_desktop_test" && cf_desktop_want=
			fi
		fi
		if test -z "$cf_desktop_want"
		then
			cf_desktop_want="ifelse([$4],,ifelse([$3],,[Application;],[`echo "$3" | sed -e 's/\*//g' -e 's/|/;/g' -e 's/[[;]]*$/;/g'`]),[$4])"
			CF_VERBOSE(no usable value found for desktop category, using $cf_desktop_want)
		fi
	fi
	DESKTOP_CATEGORY=`echo "$cf_desktop_want" | sed -e 's/[[ ,]]/;/g'`
	CF_VERBOSE(will use Categories=$DESKTOP_CATEGORY)
	AC_SUBST(DESKTOP_CATEGORY)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DMALLOC version: 7 updated: 2010/06/21 17:26:47
dnl ---------------
dnl Configure-option for dmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DMALLOC],[
CF_NO_LEAKS_OPTION(dmalloc,
	[  --with-dmalloc          test: use Gray Watson's dmalloc library],
	[USE_DMALLOC])

if test "$with_dmalloc" = yes ; then
	AC_CHECK_HEADER(dmalloc.h,
		[AC_CHECK_LIB(dmalloc,[dmalloc_debug]ifelse([$1],,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_ICONDIR version: 5 updated: 2012/07/22 09:18:02
dnl ---------------
dnl Handle configure option "--with-icondir", setting these shell variables:
dnl
dnl $ICONDIR is the option value, used for installing icon files.
dnl $no_icondir is a "#" (comment) if "--without-icondir" is given.
AC_DEFUN([CF_WITH_ICONDIR],[
AC_MSG_CHECKING(for directory to install icons)
AC_ARG_WITH(icondir,
	[  --with-icondir=DIR      directory in which to install icons for desktop],
	[ICONDIR=$withval],
	[test -z "$ICONDIR" && ICONDIR=no])

if test "x[$]ICONDIR" = xauto
then
	ICONDIR='${datadir}/icons'
	for cf_path in \
		/usr/share/icons \
		/usr/X11R6/share/icons
	do
		if test -d "$cf_path" ; then
			ICONDIR="$cf_path"
			break
		fi
	done
else
	cf_path=$ICONDIR
	CF_PATH_SYNTAX(cf_path)
fi
AC_MSG_RESULT($ICONDIR)
AC_SUBST(ICONDIR)

no_icondir=
if test "$ICONDIR" = no
then
	no_icondir="#"
else
	EXTRA_INSTALL_DIRS="$EXTRA_INSTALL_DIRS \$(ICONDIR)"
fi
AC_SUBST(no_icondir)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_ICON_NAME version: 4 updated: 2023/11/23 06:40:35
dnl -----------------
dnl Allow a default icon-name to be overridden.
dnl $1 = default icon name
AC_DEFUN([CF_WITH_ICON_NAME],[
AC_MSG_CHECKING(for the icon name)
AC_ARG_WITH(icon-name,
	[[  --with-icon-name[=XXX]  override icon name (default: $1)]],
	[ICON_NAME="$withval"],
	[ICON_NAME=$1])
case "x$ICON_NAME" in
(xyes|xno|x)
	ICON_NAME=$1
	;;
esac
AC_SUBST(ICON_NAME)
AC_MSG_RESULT($ICON_NAME)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_ICON_SYMLINK version: 3 updated: 2023/11/23 06:40:35
dnl --------------------
dnl Workaround for systems which are (mis)configured to map all icon references
dnl for xterm into "xterm" name.  For instance, recent (2013) KDE ignores both
dnl the name given in the .desktop file (xterm-color) and the application name
dnl (xterm-dev).
dnl
dnl $1 = default icon name to use if symlink is wanted
AC_DEFUN([CF_WITH_ICON_SYMLINK],[
AC_MSG_CHECKING(for icon symlink to use)
AC_ARG_WITH(icon-symlink,
	[[  --with-icon-symlink[=XXX] make symbolic link for icon name (default: $1)]],
	[ICON_SYMLINK="$withval"],
	[ICON_SYMLINK=NONE])
case "x$ICON_SYMLINK" in
(xyes)
	ICON_SYMLINK=$1
	;;
(xno|x)
	ICON_SYMLINK=NONE
	;;
esac
AC_SUBST(ICON_SYMLINK)
AC_MSG_RESULT($ICON_SYMLINK)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_ICON_THEME version: 14 updated: 2023/11/23 06:40:35
dnl ------------------
dnl If asked, check for prerequisites and setup symbols to permit installing
dnl one or more application icons in the Red Hat icon-theme directory
dnl hierarchy.
dnl
dnl If the prerequisites are missing, give a warning and revert to the long-
dnl standing pixmaps directory.
dnl
dnl Parameters:
dnl
dnl $1 = application icon.  This can be a list, and is not optional.
dnl $2 = default theme (defaults to hicolor)
dnl $3 = formats (defaults to list [.svg .png .xpm])
dnl $4 = alternate icon if no theme is used (defaults to $1).
dnl
dnl Result:
dnl ICON_NAME = basename of first item in $1, unless already set
dnl ICON_LIST = reprocessed $1
dnl ICON_THEME = reprocessed $2
dnl ICON_FORMAT = reprocessed $3
AC_DEFUN([CF_WITH_ICON_THEME],
[
ifelse([$1],,[
	AC_MSG_ERROR([macro [CF_WITH_ICON_THEME] requires application-icon name])
],[

CF_WITH_PIXMAPDIR
CF_WITH_ICONDIR

AC_MSG_CHECKING(if icon theme should be used)
AC_ARG_WITH(icon-theme,
	[[  --with-icon-theme[=XXX] install icons into desktop theme (hicolor)]],
	[ICON_THEME=$withval],
	[ICON_THEME=no])

case "x$ICON_THEME" in
(xno)
	;;
(x|xyes)
	ICON_THEME=ifelse([$2],,hicolor,$2)
	;;
esac
AC_MSG_RESULT($ICON_THEME)

if test "x$ICON_THEME" = xno
then
	if test "x$ICONDIR" != xno
	then
		CF_VERBOSE(ignoring icondir without theme)
		no_icondir="#"
	fi
else
	if test "x$ICONDIR" = xno
	then
		AC_MSG_ERROR(icondir must be set for icon theme)
	fi
fi

: ${ICON_FORMAT:=ifelse([$3],,[".svg .png .xpm"],[$3])}

ICON_LIST=

ifelse([$4],,[cf_icon_list=$1],[
if test "x$ICON_THEME" != xno
then
	cf_icon_list="$1"
else
	cf_icon_list="$4"
fi
])

AC_MSG_CHECKING([for icon(s) to install])
for cf_name in $cf_icon_list
do
	CF_VERBOSE(using $ICON_FORMAT)
	for cf_suffix in $ICON_FORMAT
	do
		cf_icon="${cf_name}${cf_suffix}"
		cf_left=`echo "$cf_icon" | sed -e 's/:.*//'`
		if test ! -f "${cf_left}"
		then
			if test "x$srcdir" != "x."
			then
				cf_icon="${srcdir}/${cf_left}"
				cf_left=`echo "$cf_icon" | sed -e 's/:.*//'`
				if test ! -f "${cf_left}"
				then
					continue
				fi
			else
				continue
			fi
		fi
		if test "x$ICON_THEME" != xno
		then
			cf_base=`basename "$cf_left"`
			cf_trim=`echo "$cf_base" | sed -e 's/_[[0-9]][[0-9]]x[[0-9]][[0-9]]\././'`
			case "x${cf_base}" in
			(*:*)
				cf_next=$cf_base
				# user-defined mapping
				;;
			(*.png)
				cf_size=`file "$cf_left"|sed -e 's/^[[^:]]*://' -e 's/^.*[[^0-9]]\([[0-9]][[0-9]]* x [[0-9]][[0-9]]*\)[[^0-9]].*$/\1/' -e 's/ //g'`
				if test -z "$cf_size"
				then
					AC_MSG_WARN(cannot determine size of $cf_left)
					continue
				fi
				cf_next="$cf_size/apps/$cf_trim"
				;;
			(*.svg)
				cf_next="scalable/apps/$cf_trim"
				;;
			(*.xpm)
				CF_VERBOSE(ignored XPM file in icon theme)
				continue
				;;
			(*_[[0-9]][[0-9]]*x[[0-9]][[0-9]]*.*)
				cf_size=`echo "$cf_left"|sed -e 's/^.*_\([[0-9]][[0-9]]*x[[0-9]][[0-9]]*\)\..*$/\1/'`
				cf_left=`echo "$cf_left"|sed -e 's/^\(.*\)_\([[0-9]][[0-9]]*x[[0-9]][[0-9]]*\)\(\..*\)$/\1\3/'`
				cf_next="$cf_size/apps/$cf_base"
				;;
			esac
			CF_VERBOSE(adding $cf_next)
			cf_icon="${cf_icon}:${cf_next}"
		fi
		test -n "$ICON_LIST" && ICON_LIST="$ICON_LIST "
		ICON_LIST="$ICON_LIST${cf_icon}"
		if test -z "$ICON_NAME"
		then
			ICON_NAME=`basename "$cf_icon" | sed -e 's/[[.:]].*//'`
		fi
	done
done

if test -n "$verbose"
then
	AC_MSG_CHECKING(result)
fi
AC_MSG_RESULT($ICON_LIST)

if test -z "$ICON_LIST"
then
	AC_MSG_ERROR(no icons found)
fi
])

AC_MSG_CHECKING(for icon name)
AC_MSG_RESULT($ICON_NAME)

AC_SUBST(ICON_FORMAT)
AC_SUBST(ICON_THEME)
AC_SUBST(ICON_LIST)
AC_SUBST(ICON_NAME)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_IMAKE_CFLAGS version: 13 updated: 2024/07/05 19:34:53
dnl --------------------
dnl xterm and similar programs build more readily when propped up with imake's
dnl hand-tuned definitions.  If we do not use imake, provide fallbacks for the
dnl most common definitions that we're not likely to do by autoconf tests.
AC_DEFUN([CF_WITH_IMAKE_CFLAGS],[
AC_REQUIRE([CF_ENABLE_NARROWPROTO])

AC_MSG_CHECKING(if we should use imake to help)
CF_ARG_ENABLE(imake,
	[  --enable-imake          enable use of imake for definitions],
	[enable_imake=yes],
	[enable_imake=no])
AC_MSG_RESULT($enable_imake)

if test "$enable_imake" = yes ; then
	CF_IMAKE_CFLAGS(ifelse([$1],,,[$1]))
fi

if test -n "$IMAKE" && test -n "$IMAKE_CFLAGS" ; then
	CF_ADD_CFLAGS($IMAKE_CFLAGS)
else
	IMAKE_CFLAGS=
	IMAKE_LOADFLAGS=
	CF_VERBOSE(make fallback definitions)

	# We prefer config.guess' values when we can get them, to avoid
	# inconsistent results with uname (AIX for instance).  However,
	# config.guess is not always consistent either.
	case $host_os in
	(*[[0-9]].[[0-9]]*)
		UNAME_RELEASE="$host_os"
		;;
	(*)
		UNAME_RELEASE=`(uname -r) 2>/dev/null` || UNAME_RELEASE=unknown
		;;
	esac

	case .$UNAME_RELEASE in
	(*[[0-9]].[[0-9]]*)
		OSMAJORVERSION=`echo "$UNAME_RELEASE" |sed -e 's/^[[^0-9]]*//' -e 's/\..*//'`
		OSMINORVERSION=`echo "$UNAME_RELEASE" |sed -e 's/^[[^0-9]]*//' -e 's/^[[^.]]*\.//' -e 's/\..*//' -e 's/[[^0-9]].*//' `
		test -z "$OSMAJORVERSION" && OSMAJORVERSION=1
		test -z "$OSMINORVERSION" && OSMINORVERSION=0
		IMAKE_CFLAGS="-DOSMAJORVERSION=$OSMAJORVERSION -DOSMINORVERSION=$OSMINORVERSION $IMAKE_CFLAGS"
		;;
	esac

	# FUNCPROTO is standard with X11R6, but XFree86 drops it, leaving some
	# fallback/fragments for NeedPrototypes, etc.
	IMAKE_CFLAGS="-DFUNCPROTO=15 $IMAKE_CFLAGS"

	# If this is not set properly, Xaw's scrollbars will not work
	if test "$enable_narrowproto" = yes ; then
		IMAKE_CFLAGS="-DNARROWPROTO=1 $IMAKE_CFLAGS"
	fi

	# Other special definitions:
	case $host_os in
	(aix*)
		# imake on AIX 5.1 defines AIXV3.  really.
		IMAKE_CFLAGS="-DAIXV3 -DAIXV4 $IMAKE_CFLAGS"
		;;
	(irix[[56]].*)
		# these are needed to make SIGWINCH work in xterm
		IMAKE_CFLAGS="-DSYSV -DSVR4 $IMAKE_CFLAGS"
		;;
	esac

	# "modern" systems install X applications in /usr/bin.  Other systems may
	# use one of the X release-based directories.
	case "$CFLAGS $CPPFLAGS $IMAKE_CFLAGS" in
	(*-DPROJECTROOT*)
		;;
	(*)
		for cf_dir in /usr/X11R7 /usr/X11R6 /usr/X11R5
		do
			if test -d "$cf_dir/bin"
			then
				IMAKE_CFLAGS="$IMAKE_CFLAGS -DPROJECTROOT=\\\"$cf_dir\\\""
				break
			fi
		done
		;;
	esac

	CF_ADD_CFLAGS($IMAKE_CFLAGS)

	AC_SUBST(IMAKE_CFLAGS)
	AC_SUBST(IMAKE_LOADFLAGS)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_MAN2HTML version: 14 updated: 2024/09/09 17:17:46
dnl ----------------
dnl Check for man2html and groff.  Prefer man2html over groff, but use groff
dnl as a fallback.  See
dnl
dnl		https://invisible-island.net/scripts/man2html.html
dnl
dnl Generate a shell script which hides the differences between the two.
dnl
dnl We name that "man2html.tmp".
dnl
dnl The shell script can be removed later, e.g., using "make distclean".
AC_DEFUN([CF_WITH_MAN2HTML],[
AC_REQUIRE([CF_PROG_GROFF])dnl
AC_REQUIRE([AC_PROG_FGREP])dnl

case "x${with_man2html}" in
(xno)
	cf_man2html=no
	;;
(x|xyes)
	AC_PATH_PROG(cf_man2html,man2html,no)
	case "x$cf_man2html" in
	(x/*)
		AC_MSG_CHECKING(for the modified Earl Hood script)
		if ( $cf_man2html -help 2>&1 | grep 'Make an index of headers at the end' >/dev/null )
		then
			cf_man2html_ok=yes
		else
			cf_man2html=no
			cf_man2html_ok=no
		fi
		AC_MSG_RESULT($cf_man2html_ok)
		;;
	(*)
		cf_man2html=no
		;;
	esac
esac

AC_MSG_CHECKING(for program to convert manpage to html)
AC_ARG_WITH(man2html,
	[[  --with-man2html[=XXX]   use XXX rather than groff]],
	[cf_man2html=$withval],
	[cf_man2html=$cf_man2html])

cf_with_groff=no

case $cf_man2html in
(yes)
	AC_MSG_RESULT(man2html)
	AC_PATH_PROG(cf_man2html,man2html,no)
	;;
(no|groff|*/groff*)
	cf_with_groff=yes
	cf_man2html=$GROFF_PATH
	AC_MSG_RESULT($cf_man2html)
	;;
(*)
	AC_MSG_RESULT($cf_man2html)
	;;
esac

MAN2HTML_TEMP="man2html.tmp"
	cat >$MAN2HTML_TEMP <<CF_EOF
#!$SHELL
# Temporary script generated by CF_WITH_MAN2HTML
# Convert inputs to html, sending result to standard output.
#
# Parameters:
# \${1} = rootname of file to convert
# \${2} = suffix of file to convert, e.g., "1"
# \${3} = macros to use, e.g., "man"
#
ROOT=\[$]1
TYPE=\[$]2
MACS=\[$]3

unset LANG
unset LC_ALL
unset LC_CTYPE
unset LANGUAGE
GROFF_NO_SGR=stupid
export GROFF_NO_SGR

CF_EOF

NROFF_OPTS=
if test "x$cf_with_groff" = xyes
then
	MAN2HTML_NOTE="$GROFF_NOTE"
	MAN2HTML_PATH="$GROFF_PATH"
	cat >>$MAN2HTML_TEMP <<CF_EOF
$SHELL -c "$TBL_PATH \${ROOT}.\${TYPE} | $GROFF_PATH -P -o0 -I\${ROOT}_ -Thtml -\${MACS}"
CF_EOF
else
	# disable hyphenation if this is groff
	if test "x$GROFF_PATH" != xno
	then
		AC_MSG_CHECKING(if nroff is really groff)
		cf_check_groff="`$NROFF_PATH --version 2>/dev/null | grep groff`"
		test -n "$cf_check_groff" && cf_check_groff=yes
		test -n "$cf_check_groff" || cf_check_groff=no
		AC_MSG_RESULT($cf_check_groff)
		test "x$cf_check_groff" = xyes && NROFF_OPTS="-rHY=0"
	fi
	MAN2HTML_NOTE=""
	CF_PATH_SYNTAX(cf_man2html)
	MAN2HTML_PATH="$cf_man2html"
	AC_MSG_CHECKING(for $cf_man2html top/bottom margins)

	# for this example, expect 3 lines of content, the remainder is head/foot
	cat >conftest.in <<CF_EOF
.TH HEAD1 HEAD2 HEAD3 HEAD4 HEAD5
.SH SECTION
MARKER
CF_EOF

	LC_ALL=C LC_CTYPE=C LANG=C LANGUAGE=C $NROFF_PATH -man conftest.in >conftest.out

	cf_man2html_1st="`${FGREP-fgrep} -n MARKER conftest.out |sed -e 's/^[[^0-9]]*://' -e 's/:.*//'`"
	cf_man2html_top=`expr "$cf_man2html_1st" - 2`
	cf_man2html_bot="`wc -l conftest.out |sed -e 's/[[^0-9]]//g'`"
	cf_man2html_bot=`expr "$cf_man2html_bot" - 2 - "$cf_man2html_top"`
	cf_man2html_top_bot="-topm=$cf_man2html_top -botm=$cf_man2html_bot"

	AC_MSG_RESULT($cf_man2html_top_bot)

	AC_MSG_CHECKING(for pagesize to use)
	for cf_block in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
	do
	cat >>conftest.in <<CF_EOF
.nf
0
1
2
3
4
5
6
7
8
9
CF_EOF
	done

	LC_ALL=C LC_CTYPE=C LANG=C LANGUAGE=C $NROFF_PATH -man conftest.in >conftest.out
	cf_man2html_page="`${FGREP-fgrep} -n HEAD1 conftest.out |sed -n '$p' |sed -e 's/^[[^0-9]]*://' -e 's/:.*//'`"
	test -z "$cf_man2html_page" && cf_man2html_page=99999
	test "$cf_man2html_page" -gt 100 && cf_man2html_page=99999

	rm -rf conftest*
	AC_MSG_RESULT($cf_man2html_page)

	cat >>$MAN2HTML_TEMP <<CF_EOF
: \${MAN2HTML_PATH=$MAN2HTML_PATH}
MAN2HTML_OPTS="\$MAN2HTML_OPTS -index -title=\"\$ROOT(\$TYPE)\" -compress -pgsize $cf_man2html_page"
case \${TYPE} in
(ms)
	$TBL_PATH \${ROOT}.\${TYPE} | $NROFF_PATH $NROFF_OPTS -\${MACS} | \$MAN2HTML_PATH -topm=0 -botm=0 \$MAN2HTML_OPTS
	;;
(*)
	$TBL_PATH \${ROOT}.\${TYPE} | $NROFF_PATH $NROFF_OPTS -\${MACS} | \$MAN2HTML_PATH $cf_man2html_top_bot \$MAN2HTML_OPTS
	;;
esac
CF_EOF
fi

chmod 700 $MAN2HTML_TEMP

AC_SUBST(MAN2HTML_NOTE)
AC_SUBST(MAN2HTML_PATH)
AC_SUBST(MAN2HTML_TEMP)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_PCRE version: 12 updated: 2017/07/29 22:57:34
dnl ------------
dnl Add PCRE (Perl-compatible regular expressions) to the build if it is
dnl available and the user requests it.  Assume the application will otherwise
dnl use the POSIX interface.
dnl
dnl TODO allow $withval to specify package location
AC_DEFUN([CF_WITH_PCRE],
[
AC_REQUIRE([CF_PKG_CONFIG])

AC_MSG_CHECKING(if you want to use PCRE for regular-expressions)
AC_ARG_WITH(pcre,
	[  --with-pcre             use PCRE for regular-expressions])
test -z "$with_pcre" && with_pcre=no
AC_MSG_RESULT($with_pcre)

if test "$with_pcre" != no ; then
	CF_TRY_PKG_CONFIG(libpcre,,[
		AC_CHECK_LIB(pcre,pcre_compile,,
			AC_MSG_ERROR(Cannot find PCRE library))])

	AC_DEFINE(HAVE_LIB_PCRE,1,[Define to 1 if we can/should compile with the PCRE library])

	case $LIBS in
	(*pcreposix*)
		;;
	(*)
		AC_CHECK_LIB(pcreposix,pcreposix_regcomp,
			[AC_DEFINE(HAVE_PCREPOSIX_H,1,[Define to 1 if we should include pcreposix.h])
				CF_ADD_LIB(pcreposix)],
			[AC_CHECK_LIB(pcreposix,regcomp,[
				AC_DEFINE(HAVE_PCREPOSIX_H,1,[Define to 1 if we should include pcreposix.h])
				CF_ADD_LIB(pcreposix)],
				AC_MSG_ERROR(Cannot find PCRE POSIX library)]))
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_PCRE2 version: 6 updated: 2021/08/11 20:35:34
dnl -------------
dnl Add PCRE2 (Perl-compatible regular expressions v2) to the build if it is
dnl available and the user requests it.  Assume the application will otherwise
dnl use the POSIX interface.
dnl
dnl TODO allow $withval to specify package location
AC_DEFUN([CF_WITH_PCRE2],
[
AC_REQUIRE([CF_PKG_CONFIG])

AC_MSG_CHECKING(if you want to use PCRE2 for regular-expressions)
AC_ARG_WITH(pcre2,
	[  --with-pcre2            use PCRE2 for regular-expressions])
test -z "$with_pcre2" && with_pcre2=no
AC_MSG_RESULT($with_pcre2)

if test "x$with_pcre2" != xno ; then
	cf_with_pcre2_ok=no
	for cf_with_pcre2 in libpcre2 libpcre2-posix libpcre
	do
		CF_TRY_PKG_CONFIG($cf_with_pcre2,[cf_with_pcre2_ok=yes; break])
	done
	cf_with_pcre2_ok=yes || AC_MSG_ERROR(Cannot find PCRE2 library)

	AC_DEFINE(HAVE_LIB_PCRE2,1,[Define to 1 if we can/should compile with the PCRE2 library])

	# if pkgconfig gave no results, look for the libraries directly
	case "$LIBS" in
	(*pcre2-posix*|*pcreposix*)
		;;
	(*)
		AC_CHECK_LIB(pcre2-posix,regcomp,[
			CF_ADD_LIB(pcre2-posix)],
			[AC_CHECK_LIB(pcreposix,regcomp,[
			 CF_ADD_LIB(pcreposix)
			],[AC_MSG_ERROR(Cannot find PCRE2 POSIX library)])])
		;;
	esac

	# either way, check for the library header files
	AC_CHECK_HEADERS(pcre2posix.h pcreposix.h)
	AC_CHECK_FUNCS(PCRE2regcomp)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_PIXMAPDIR version: 3 updated: 2012/07/22 09:18:02
dnl -----------------
dnl Handle configure option "--with-pixmapdir", setting these shell variables:
dnl
dnl $PIXMAPDIR is the option value, used for installing pixmap files.
dnl $no_pixmapdir is a "#" (comment) if "--without-pixmapdir" is given.
AC_DEFUN([CF_WITH_PIXMAPDIR],[
AC_MSG_CHECKING(for directory to install pixmaps)
AC_ARG_WITH(pixmapdir,
	[  --with-pixmapdir=DIR    directory in which to install pixmaps (DATADIR/pixmaps)],
	[PIXMAPDIR=$withval],
	[test -z "$PIXMAPDIR" && PIXMAPDIR='${datadir}/pixmaps'])

if test "x[$]PIXMAPDIR" = xauto
then
	PIXMAPDIR='${datadir}/pixmaps'
	for cf_path in \
		/usr/share/pixmaps \
		/usr/X11R6/share/pixmaps
	do
		if test -d "$cf_path" ; then
			PIXMAPDIR="$cf_path"
			break
		fi
	done
else
	cf_path=$PIXMAPDIR
	CF_PATH_SYNTAX(cf_path)
fi
AC_MSG_RESULT($PIXMAPDIR)
AC_SUBST(PIXMAPDIR)

no_pixmapdir=
if test "$PIXMAPDIR" = no
then
	no_pixmapdir="#"
else
	EXTRA_INSTALL_DIRS="$EXTRA_INSTALL_DIRS \$(PIXMAPDIR)"
fi
AC_SUBST(no_pixmapdir)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_SYSTYPE version: 1 updated: 2013/01/26 16:26:12
dnl ---------------
dnl For testing, override the derived host system-type which is used to decide
dnl things such as the linker commands used to build shared libraries.  This is
dnl normally chosen automatically based on the type of system which you are
dnl building on.  We use it for testing the configure script.
dnl
dnl This is different from the --host option: it is used only for testing parts
dnl of the configure script which would not be reachable with --host since that
dnl relies on the build environment being real, rather than mocked up.
AC_DEFUN([CF_WITH_SYSTYPE],[
CF_CHECK_CACHE([AC_CANONICAL_SYSTEM])
AC_ARG_WITH(system-type,
	[  --with-system-type=XXX  test: override derived host system-type],
[AC_MSG_WARN(overriding system type to $withval)
	cf_cv_system_name=$withval
	host_os=$withval
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_UTMP_PATH version: 1 updated: 2023/09/04 16:05:17
dnl -----------------
dnl utmp and wtmp have different pathnames on different systems, but there
dnl are only a few common choices.  Note that they may not necessarily appear
dnl in the same directories.  Prefer utmpx/wtmpx to utmp/wtmp, since that's
dnl the way the configure script is designed.
AC_DEFUN([CF_WITH_UTMP_PATH],[
AC_ARG_WITH(utmp-path,
	[  --with-utmp-path=XXX    use XXX rather than auto for utmp path],
	[cf_utmp_path=$withval],
	[cf_utmp_path=auto])
if test "$cf_utmp_path" = auto ; then
	for cf_utmp_path in /etc/utmp /var/adm/utmp /var/log/utmp /var/run/utmp
	do
		if test -f ${cf_utmp_path}x ; then
			cf_utmp_path=${cf_utmp_path}x
			break
		elif test -f $cf_utmp_path ; then
			break
		fi
	done
else
	CF_PATH_SYNTAX(cf_utmp_path)
fi
UTMP_PATH=$cf_utmp_path
UTMP_NAME=`echo "$cf_utmp_path" | sed -e 's,^.*/,,'`
AC_SUBST(UTMP_NAME)
AC_SUBST(UTMP_PATH)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_VALGRIND version: 1 updated: 2006/12/14 18:00:21
dnl ----------------
AC_DEFUN([CF_WITH_VALGRIND],[
CF_NO_LEAKS_OPTION(valgrind,
	[  --with-valgrind         test: use valgrind],
	[USE_VALGRIND])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_WTMP_PATH version: 2 updated: 2023/10/02 17:22:48
dnl -----------------
dnl Like CF_WITH_UTMP_PATH, but for the wtmp file.
AC_DEFUN([CF_WITH_WTMP_PATH],[
AC_ARG_WITH(wtmp-path,
	[  --with-wtmp-path=XXX    use XXX rather than auto for wtmp path],
	[cf_wtmp_path=$withval],
	[cf_wtmp_path=auto])
if test "$cf_wtmp_path" = auto ; then
	for cf_wtmp_path in /etc/wtmp /var/adm/wtmp /var/run/wtmp /var/log/wtmp
	do
		if test -f ${cf_wtmp_path}x ; then
			cf_wtmp_path=${cf_wtmp_path}x
			break
		elif test -f $cf_wtmp_path/wtmp ; then
			break
		fi
	done
else
	CF_PATH_SYNTAX(cf_wtmp_path)
fi
WTMP_PATH=$cf_wtmp_path
WTMP_NAME=`echo "$cf_wtmp_path" | sed -e 's,^.*/,,'`
AC_SUBST(WTMP_NAME)
AC_SUBST(WTMP_PATH)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_XINERAMA version: 1 updated: 2016/05/28 14:41:12
dnl ----------------
AC_DEFUN([CF_WITH_XINERAMA],
[
AC_MSG_CHECKING(if you want to use the Xinerama extension)
AC_ARG_WITH(xinerama,
[  --without-xinerama      do not use Xinerama extension for multiscreen support],
	[cf_with_xinerama="$withval"],
	[cf_with_xinerama=yes])
AC_MSG_RESULT($cf_with_xinerama)
if test "$cf_with_xinerama" = yes; then
	CF_XINERAMA
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_XPM version: 4 updated: 2023/11/23 06:40:35
dnl -----------
dnl Test for Xpm library, update compiler/loader flags if it is wanted and
dnl found.
dnl
dnl Also sets ICON_SUFFIX
AC_DEFUN([CF_WITH_XPM],
[
ICON_SUFFIX=.xbm

cf_save_cppflags="${CPPFLAGS}"
cf_save_ldflags="${LDFLAGS}"

AC_MSG_CHECKING(if you want to use the Xpm library for colored icon)
AC_ARG_WITH(xpm,
[[  --with-xpm[=DIR]        use Xpm library for colored icon, may specify path]],
	[cf_Xpm_library="$withval"],
	[cf_Xpm_library=yes])
AC_MSG_RESULT($cf_Xpm_library)

if test "$cf_Xpm_library" != no ; then
    if test "$cf_Xpm_library" != yes ; then
	CPPFLAGS="$CPPFLAGS -I$withval/include"
	LDFLAGS="$LDFLAGS -L$withval/lib"
    fi
    AC_CHECK_HEADER(X11/xpm.h,[
	AC_CHECK_LIB(Xpm, XpmCreatePixmapFromData,[
	    AC_DEFINE(HAVE_LIBXPM,1,[Define to 1 if we should use Xpm library])
	    ICON_SUFFIX=.xpm
	    LIBS="-lXpm $LIBS"],
	    [CPPFLAGS="${cf_save_cppflags}" LDFLAGS="${cf_save_ldflags}"],
	    [-lX11 $X_LIBS])],
	[CPPFLAGS="${cf_save_cppflags}" LDFLAGS="${cf_save_ldflags}"])
fi

AC_SUBST(ICON_SUFFIX)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_XBOOL_RESULT version: 3 updated: 2015/04/12 15:39:00
dnl ---------------
dnl Translate an autoconf boolean yes/no into X11's booleans, e.g., True/False.
dnl Allow for more than two values, e.g., "maybe", still using the same leading
dnl capital convention.
dnl
dnl $1 = symbol to define
dnl $2 = symbol holding value
dnl $3 = description
define([CF_XBOOL_RESULT],[
AC_MSG_RESULT([$]$2)
case [$]$2 in
(yes)
	$2=true
	;;
(no)
	$2=false
	;;
esac
cf_xbool1=`echo "[$]$2"|sed -e 's/^\(.\).*/\1/'`
CF_UPPER(cf_xbool1,$cf_xbool1)
cf_xbool2=`echo "[$]$2"|sed -e 's/^.//'`
$2=${cf_xbool1}${cf_xbool2}
AC_DEFINE_UNQUOTED($1,[$]$2,$3)
AC_SUBST($2)
])
dnl ---------------------------------------------------------------------------
dnl CF_XINERAMA version: 2 updated: 2015/02/15 15:18:41
dnl -----------
AC_DEFUN([CF_XINERAMA],[
CF_TRY_PKG_CONFIG(xinerama,[
	AC_DEFINE(HAVE_X11_EXTENSIONS_XINERAMA_H)],[
	AC_CHECK_LIB(Xinerama,XineramaQueryScreens,
		[CF_ADD_LIB(Xinerama)
		 AC_CHECK_HEADERS( \
			X11/extensions/Xinerama.h \
			)
		])
	])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_XKB_BELL_EXT version: 7 updated: 2023/12/01 17:22:50
dnl ---------------
dnl Check for XKB bell extension
AC_DEFUN([CF_XKB_BELL_EXT],[
AC_REQUIRE([AC_PATH_XTRA])

CF_SAVE_XTRA_FLAGS([CF_XKB_BELL_EXT])

AC_CACHE_CHECK(for XKB Bell extension, cf_cv_xkb_bell_ext,[
AC_TRY_LINK([
#include <X11/Intrinsic.h>
#include <X11/XKBlib.h>		/* has the prototype */
#include <X11/extensions/XKBbells.h>	/* has the XkbBI_xxx definitions */
],[
	int x = (XkbBI_Info |XkbBI_MinorError |XkbBI_MajorError |XkbBI_TerminalBell |XkbBI_MarginBell);
	Atom y = 0;
	(void)x;
	XkbBell((Display *)0, (Window)0, 0, y);
],[cf_cv_xkb_bell_ext=yes],[cf_cv_xkb_bell_ext=no])
])
test "$cf_cv_xkb_bell_ext" = yes && AC_DEFINE(HAVE_XKB_BELL_EXT,1,[Define 1 if we have XKB Bell extension])

CF_RESTORE_XTRA_FLAGS([CF_XKB_BELL_EXT])
])
dnl ---------------------------------------------------------------------------
dnl CF_XKB_KEYCODE_TO_KEYSYM version: 3 updated: 2020/03/10 18:53:47
dnl ------------------------
dnl Some older vendor-unix systems made a practice of delivering fragments of
dnl Xkb, requiring test-compiles.
AC_DEFUN([CF_XKB_KEYCODE_TO_KEYSYM],[
AC_CACHE_CHECK(if we can use XkbKeycodeToKeysym, cf_cv_xkb_keycode_to_keysym,[
AC_TRY_COMPILE([
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
],[
    KeySym keysym = XkbKeycodeToKeysym((Display *)0, 0, 0, 0);
	(void)keysym;
],[
cf_cv_xkb_keycode_to_keysym=yes
],[
cf_cv_xkb_keycode_to_keysym=no
])
])

if test $cf_cv_xkb_keycode_to_keysym = yes
then
	AC_CHECK_FUNCS(XkbKeycodeToKeysym)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_XKB_QUERY_EXTENSION version: 2 updated: 2012/09/28 20:23:46
dnl ----------------------
dnl see ifdef in scrollbar.c - iron out here
AC_DEFUN([CF_XKB_QUERY_EXTENSION],[
AC_CACHE_CHECK(if we can use XkbQueryExtension, cf_cv_xkb_query_extension,[
AC_TRY_COMPILE([
#include <X11/Xlib.h>
#include <X11/extensions/XKB.h>
#include <X11/XKBlib.h>
],[
	int xkbmajor = XkbMajorVersion;
	int xkbminor = XkbMinorVersion;
	int xkbopcode, xkbevent, xkberror;

	if (XkbLibraryVersion(&xkbmajor, &xkbminor)
	    && XkbQueryExtension((Display *)0,
				 &xkbopcode,
				 &xkbevent,
				 &xkberror,
				 &xkbmajor,
				 &xkbminor))
		 return 0;
],[
cf_cv_xkb_query_extension=yes
],[
cf_cv_xkb_query_extension=no
])
])

if test $cf_cv_xkb_query_extension = yes
then
	AC_CHECK_FUNCS(XkbQueryExtension)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_XOPEN_SOURCE version: 68 updated: 2024/11/09 18:07:29
dnl ---------------
dnl Try to get _XOPEN_SOURCE defined properly that we can use POSIX functions,
dnl or adapt to the vendor's definitions to get equivalent functionality,
dnl without losing the common non-POSIX features.
dnl
dnl Parameters:
dnl	$1 is the nominal value for _XOPEN_SOURCE
dnl	$2 is the nominal value for _POSIX_C_SOURCE
dnl
dnl The default case prefers _XOPEN_SOURCE over _POSIX_C_SOURCE if the
dnl implementation predefines it, because X/Open and most implementations agree
dnl that the latter is a legacy or "aligned" value.
dnl
dnl Because _XOPEN_SOURCE is preferred, if defining _POSIX_C_SOURCE turns
dnl that off, then refrain from setting _POSIX_C_SOURCE explicitly.
dnl
dnl References:
dnl https://pubs.opengroup.org/onlinepubs/007904975/functions/xsh_chap02_02.html
dnl https://docs.oracle.com/cd/E19253-01/816-5175/standards-5/index.html
dnl https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
AC_DEFUN([CF_XOPEN_SOURCE],[
AC_REQUIRE([AC_CANONICAL_HOST])
AC_REQUIRE([CF_POSIX_VISIBLE])

if test "$cf_cv_posix_visible" = no; then

cf_XOPEN_SOURCE=ifelse([$1],,500,[$1])
cf_POSIX_C_SOURCE=ifelse([$2],,199506L,[$2])
cf_xopen_source=

case "$host_os" in
(aix[[4-7]]*)
	cf_xopen_source="-D_ALL_SOURCE"
	;;
(darwin[[0-8]].*)
	cf_xopen_source="-D_APPLE_C_SOURCE"
	;;
(darwin*)
	cf_xopen_source="-D_DARWIN_C_SOURCE"
	cf_XOPEN_SOURCE=
	;;
(freebsd*|dragonfly*|midnightbsd*)
	# 5.x headers associate
	#	_XOPEN_SOURCE=600 with _POSIX_C_SOURCE=200112L
	#	_XOPEN_SOURCE=500 with _POSIX_C_SOURCE=199506L
	cf_POSIX_C_SOURCE=200112L
	cf_XOPEN_SOURCE=600
	cf_xopen_source="-D_BSD_TYPES -D__BSD_VISIBLE -D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE -D_XOPEN_SOURCE=$cf_XOPEN_SOURCE"
	;;
(hpux11*)
	cf_xopen_source="-D_HPUX_SOURCE -D_XOPEN_SOURCE=500"
	;;
(hpux*)
	cf_xopen_source="-D_HPUX_SOURCE"
	;;
(irix[[56]].*)
	cf_xopen_source="-D_SGI_SOURCE"
	cf_XOPEN_SOURCE=
	;;
(linux*gnu|linux*gnuabi64|linux*gnuabin32|linux*gnueabi|linux*gnueabihf|linux*gnux32|uclinux*|gnu*|mint*|k*bsd*-gnu|cygwin|msys|mingw*|linux*uclibc)
	CF_GNU_SOURCE($cf_XOPEN_SOURCE)
	;;
linux*musl)
	cf_xopen_source="-D_BSD_SOURCE"
	;;
(minix*)
	cf_xopen_source="-D_NETBSD_SOURCE" # POSIX.1-2001 features are ifdef'd with this...
	;;
(mirbsd*)
	# setting _XOPEN_SOURCE or _POSIX_SOURCE breaks <sys/select.h> and other headers which use u_int / u_short types
	cf_XOPEN_SOURCE=
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	;;
(netbsd*)
	cf_xopen_source="-D_NETBSD_SOURCE" # setting _XOPEN_SOURCE breaks IPv6 for lynx on NetBSD 1.6, breaks xterm, is not needed for ncursesw
	;;
(openbsd[[6-9]]*)
	# OpenBSD 6.x has broken locale support, both compile-time and runtime.
	# see https://www.mail-archive.com/bugs@openbsd.org/msg13200.html
	# Abusing the conformance level is a workaround.
	AC_MSG_WARN(this system does not provide usable locale support)
	cf_xopen_source="-D_BSD_SOURCE"
	cf_XOPEN_SOURCE=700
	;;
(openbsd[[4-5]]*)
	# setting _XOPEN_SOURCE lower than 500 breaks g++ compile with wchar.h, needed for ncursesw
	cf_xopen_source="-D_BSD_SOURCE"
	cf_XOPEN_SOURCE=600
	;;
(openbsd*)
	# setting _XOPEN_SOURCE breaks xterm on OpenBSD 2.8, is not needed for ncursesw
	;;
(osf[[45]]*)
	cf_xopen_source="-D_OSF_SOURCE"
	;;
(nto-qnx*)
	cf_xopen_source="-D_QNX_SOURCE"
	;;
(sco*)
	# setting _XOPEN_SOURCE breaks Lynx on SCO Unix / OpenServer
	;;
(solaris2.*)
	cf_xopen_source="-D__EXTENSIONS__"
	cf_cv_xopen_source=broken
	;;
(sysv4.2uw2.*) # Novell/SCO UnixWare 2.x (tested on 2.1.2)
	cf_XOPEN_SOURCE=
	cf_POSIX_C_SOURCE=
	;;
(*)
	CF_TRY_XOPEN_SOURCE
	cf_save_xopen_cppflags="$CPPFLAGS"
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	# Some of these niche implementations use copy/paste, double-check...
	if test "$cf_cv_xopen_source" = no ; then
		CF_VERBOSE(checking if _POSIX_C_SOURCE interferes with _XOPEN_SOURCE)
		AC_TRY_COMPILE(CF__XOPEN_SOURCE_HEAD,CF__XOPEN_SOURCE_BODY,,[
			AC_MSG_WARN(_POSIX_C_SOURCE definition is not usable)
			CPPFLAGS="$cf_save_xopen_cppflags"])
	fi
	;;
esac

if test -n "$cf_xopen_source" ; then
	CF_APPEND_CFLAGS($cf_xopen_source,true)
fi

dnl In anything but the default case, we may have system-specific setting
dnl which is still not guaranteed to provide all of the entrypoints that
dnl _XOPEN_SOURCE would yield.
if test -n "$cf_XOPEN_SOURCE" && test -z "$cf_cv_xopen_source" ; then
	AC_MSG_CHECKING(if _XOPEN_SOURCE really is set)
	AC_TRY_COMPILE([#include <stdlib.h>],[
#ifndef _XOPEN_SOURCE
#error _XOPEN_SOURCE is not defined
#endif],
	[cf_XOPEN_SOURCE_set=yes],
	[cf_XOPEN_SOURCE_set=no])
	AC_MSG_RESULT($cf_XOPEN_SOURCE_set)
	if test "$cf_XOPEN_SOURCE_set" = yes
	then
		AC_TRY_COMPILE([#include <stdlib.h>],[
#if (_XOPEN_SOURCE - 0) < $cf_XOPEN_SOURCE
#error (_XOPEN_SOURCE - 0) < $cf_XOPEN_SOURCE
#endif],
		[cf_XOPEN_SOURCE_set_ok=yes],
		[cf_XOPEN_SOURCE_set_ok=no])
		if test "$cf_XOPEN_SOURCE_set_ok" = no
		then
			AC_MSG_WARN(_XOPEN_SOURCE is lower than requested)
		fi
	else
		CF_TRY_XOPEN_SOURCE
	fi
fi
fi # cf_cv_posix_visible
])
dnl ---------------------------------------------------------------------------
dnl CF_X_ATHENA version: 26 updated: 2025/06/14 06:46:23
dnl -----------
dnl Check for Xaw (Athena) libraries
dnl
dnl Sets $cf_x_athena according to the flavor of Xaw which is used.
AC_DEFUN([CF_X_ATHENA],
[
cf_x_athena=${cf_x_athena:-Xaw}

AC_MSG_CHECKING(if you want to link with Xaw 3d library)
withval=
AC_ARG_WITH(Xaw3d,
	[  --with-Xaw3d            link with Xaw 3d library])
if test "$withval" = yes ; then
	cf_x_athena=Xaw3d
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if you want to link with Xaw 3d xft library)
withval=
AC_ARG_WITH(Xaw3dxft,
	[  --with-Xaw3dxft         link with Xaw 3d xft library])
if test "$withval" = yes ; then
	cf_x_athena=Xaw3dxft
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if you want to link with neXT Athena library)
withval=
AC_ARG_WITH(neXtaw,
	[  --with-neXtaw           link with neXT Athena library])
if test "$withval" = yes ; then
	cf_x_athena=neXtaw
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if you want to link with Athena-Plus library)
withval=
AC_ARG_WITH(XawPlus,
	[  --with-XawPlus          link with Athena-Plus library])
if test "$withval" = yes ; then
	cf_x_athena=XawPlus
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

cf_x_athena_lib=""

if test "$PKG_CONFIG" != none ; then
	cf_athena_list=
	test "$cf_x_athena" = Xaw && cf_athena_list="xaw8 xaw7 xaw6"
	for cf_athena_pkg in \
		$cf_athena_list \
		${cf_x_athena} \
		${cf_x_athena}-devel \
		lib${cf_x_athena} \
		lib${cf_x_athena}-devel
	do
		CF_TRY_PKG_CONFIG($cf_athena_pkg,[
			cf_x_athena_lib="$cf_pkgconfig_libs"
			CF_UPPER(cf_x_athena_LIBS,HAVE_LIB_$cf_x_athena)
			AC_DEFINE_UNQUOTED($cf_x_athena_LIBS)

AC_CACHE_CHECK(for usable $cf_x_athena/Xmu package,cf_cv_xaw_compat,[
AC_TRY_LINK([
$ac_includes_default
#include <X11/Xmu/CharSet.h>
],[
int check = XmuCompareISOLatin1("big", "small");
(void)check;
],[cf_cv_xaw_compat=yes],[cf_cv_xaw_compat=no])])

			if test "$cf_cv_xaw_compat" = no
			then
				# workaround for broken ".pc" files...
				case "$cf_x_athena_lib" in
				(*-lXmu*)
					;;
				(*)
					CF_VERBOSE(work around broken package)
					cf_save_xmu="$LIBS"
					cf_first_lib=`echo "$cf_save_xmu" | sed -e 's/^[ ][ ]*//' -e 's/ .*//'`
					CF_TRY_PKG_CONFIG(xmu,[
							LIBS="$cf_save_xmu"
							CF_ADD_LIB_AFTER($cf_first_lib,$cf_pkgconfig_libs)
						],[
							CF_ADD_LIB_AFTER($cf_first_lib,-lXmu)
						])
					;;
				esac
			fi

			break])
	done
fi

if test -z "$cf_x_athena_lib" ; then
	CF_X_EXT
	CF_X_TOOLKIT
	CF_X_ATHENA_CPPFLAGS($cf_x_athena)
	CF_X_ATHENA_LIBS($cf_x_athena)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_X_ATHENA_CPPFLAGS version: 9 updated: 2020/12/31 10:54:15
dnl --------------------
dnl Normally invoked by CF_X_ATHENA, with $1 set to the appropriate flavor of
dnl the Athena widgets, e.g., Xaw, Xaw3d, neXtaw.
AC_DEFUN([CF_X_ATHENA_CPPFLAGS],
[
AC_REQUIRE([AC_PATH_XTRA])
cf_x_athena_root=ifelse([$1],,Xaw,[$1])
cf_x_athena_inc=""

for cf_path in default \
	/usr/contrib/X11R6 \
	/usr/contrib/X11R5 \
	/usr/lib/X11R5 \
	/usr/local
do
	if test -z "$cf_x_athena_inc" ; then
		CF_SAVE_XTRA_FLAGS([CF_X_ATHENA_CPPFLAGS])
		cf_test=X11/$cf_x_athena_root/SimpleMenu.h
		if test "$cf_path" != default ; then
			CF_APPEND_TEXT(CPPFLAGS,-I$cf_path/include)
			AC_MSG_CHECKING(for $cf_test in $cf_path)
		else
			AC_MSG_CHECKING(for $cf_test)
		fi
		AC_TRY_COMPILE([
#include <X11/Intrinsic.h>
#include <$cf_test>],[],
			[cf_result=yes],
			[cf_result=no])
		AC_MSG_RESULT($cf_result)
		CF_RESTORE_XTRA_FLAGS([CF_X_ATHENA_CPPFLAGS])
		if test "$cf_result" = yes ; then
			test "$cf_path"  = default && cf_x_athena_inc=default
			test "$cf_path" != default && cf_x_athena_inc="$cf_path/include"
			break
		fi
	fi
done

if test -z "$cf_x_athena_inc" ; then
	AC_MSG_WARN([Unable to find Athena header files])
elif test "$cf_x_athena_inc" != default ; then
	CF_APPEND_TEXT(CPPFLAGS,-I$cf_x_athena_inc)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_X_ATHENA_LIBS version: 14 updated: 2023/01/11 04:05:23
dnl ----------------
dnl Normally invoked by CF_X_ATHENA, with $1 set to the appropriate flavor of
dnl the Athena widgets, e.g., Xaw, Xaw3d, neXtaw.
AC_DEFUN([CF_X_ATHENA_LIBS],
[AC_REQUIRE([CF_X_TOOLKIT])
cf_x_athena_root=ifelse([$1],,Xaw,[$1])
cf_x_athena_lib=""

for cf_path in default \
	/usr/contrib/X11R6 \
	/usr/contrib/X11R5 \
	/usr/lib/X11R5 \
	/usr/local
do
	for cf_lib in \
		${cf_x_athena_root} \
		${cf_x_athena_root}7 \
		${cf_x_athena_root}6
	do
	for cf_libs in \
		"-l$cf_lib -lXmu" \
		"-l$cf_lib -lXpm -lXmu" \
		"-l${cf_lib}_s -lXmu_s"
	do
		test -n "$cf_x_athena_lib" && break

		CF_SAVE_XTRA_FLAGS([CF_X_ATHENA_LIBS])
		cf_test=XawSimpleMenuAddGlobalActions
		test "$cf_path" != default && cf_libs="-L$cf_path/lib $cf_libs"
		CF_ADD_LIBS($cf_libs)
		AC_MSG_CHECKING(for $cf_test in $cf_libs)
		AC_TRY_LINK([
$ac_includes_default
#include <X11/Intrinsic.h>
#include <X11/$cf_x_athena_root/SimpleMenu.h>
],[
$cf_test((XtAppContext) 0)],
			[cf_result=yes],
			[cf_result=no])
		AC_MSG_RESULT($cf_result)
		CF_RESTORE_XTRA_FLAGS([CF_X_ATHENA_LIBS])

		if test "$cf_result" = yes ; then
			cf_x_athena_lib="$cf_libs"
			break
		fi
	done # cf_libs
		test -n "$cf_x_athena_lib" && break
	done # cf_lib
done

if test -z "$cf_x_athena_lib" ; then
	AC_MSG_ERROR(
[Unable to successfully link Athena library (-l$cf_x_athena_root) with test program])
fi

CF_ADD_LIBS($cf_x_athena_lib)
CF_UPPER(cf_x_athena_LIBS,HAVE_LIB_$cf_x_athena)
AC_DEFINE_UNQUOTED($cf_x_athena_LIBS)
])
dnl ---------------------------------------------------------------------------
dnl CF_X_EXT version: 3 updated: 2010/06/02 05:03:05
dnl --------
AC_DEFUN([CF_X_EXT],[
CF_TRY_PKG_CONFIG(Xext,,[
	AC_CHECK_LIB(Xext,XextCreateExtension,
		[CF_ADD_LIB(Xext)])])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_X_EXT_DOUBLE_BUFFER version: 1 updated: 2019/07/18 21:09:40
dnl ----------------------
AC_DEFUN([CF_X_EXT_DOUBLE_BUFFER],[
AC_REQUIRE([CF_X_EXT])
AC_CHECK_HEADER(X11/extensions/Xdbe.h,
	AC_DEFINE(HAVE_X11_EXTENSIONS_XDBE_H,1,[Define to 1 if we have X11/extensions/Xdbe.h])
	AC_CHECK_FUNC(XdbeSwapBuffers,
				  [AC_DEFINE(HAVE_XDBESWAPBUFFERS,1,[Define to 1 if we have XdbeSwapBuffers])
				   cf_x_ext_double_buffer=yes]))
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_X_FONTCONFIG version: 9 updated: 2025/04/18 20:15:05
dnl ---------------
dnl Check for fontconfig library, a dependency of the X FreeType library.
AC_DEFUN([CF_X_FONTCONFIG],
[
AC_REQUIRE([CF_X_XFT])

if test "$cf_cv_found_freetype" = yes ; then
AC_CACHE_CHECK(for usable Xft/fontconfig package,cf_cv_xft_compat,[
AC_TRY_LINK([
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
],[
	XftPattern *pat = 0;
	XftPatternBuild(pat,
					XFT_FAMILY, XftTypeString, "mono",
					(void *) 0);
],[cf_cv_xft_compat=yes],[cf_cv_xft_compat=no])
])

if test "$cf_cv_xft_compat" = no
then
	# workaround for broken ".pc" files used for Xft.
	case "$cf_cv_x_freetype_libs" in
	(*-lfontconfig*)
		;;
	(*)
		CF_VERBOSE(work around broken package)
		cf_save_fontconfig="$LIBS"
		CF_TRY_PKG_CONFIG(fontconfig,[
				CF_ADD_CFLAGS($cf_pkgconfig_incs)
				LIBS="$cf_save_fontconfig"
				CF_ADD_LIB_AFTER(-lXft,$cf_pkgconfig_libs)
			],[
				CF_ADD_LIB_AFTER(-lXft,-lfontconfig)
			])
		;;
	esac
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_X_FREETYPE version: 30 updated: 2025/04/20 17:58:12
dnl -------------
dnl Check for X FreeType headers and libraries (XFree86 4.x, etc).
dnl
dnl First check for the appropriate config program, since the developers for
dnl these libraries change their configuration (and config program) more or
dnl less randomly.  If we cannot find the config program, do not bother trying
dnl to guess the latest variation of include/lib directories.
dnl
dnl If either or both of these configure-script options are not given, rely on
dnl the output of the config program to provide the cflags/libs options:
dnl	--with-freetype-cflags
dnl	--with-freetype-libs
AC_DEFUN([CF_X_FREETYPE],
[
AC_REQUIRE([CF_PKG_CONFIG])

cf_cv_x_freetype_incs=no
cf_cv_x_freetype_libs=no
cf_extra_freetype_libs=
FREETYPE_CONFIG=none
FREETYPE_PARAMS=

AC_MSG_CHECKING(for FreeType configuration script)
AC_ARG_WITH(freetype-config,
	[  --with-freetype-config  configure script to use for FreeType],
	[cf_cv_x_freetype_cfgs="$withval"],
	[cf_cv_x_freetype_cfgs=auto])
test -z $cf_cv_x_freetype_cfgs && cf_cv_x_freetype_cfgs=auto
test $cf_cv_x_freetype_cfgs = no && cf_cv_x_freetype_cfgs=none
AC_MSG_RESULT($cf_cv_x_freetype_cfgs)

case $cf_cv_x_freetype_cfgs in
(none)
	AC_MSG_CHECKING(if you specified -D/-I options for FreeType)
	AC_ARG_WITH(freetype-cflags,
		[  --with-freetype-cflags  -D/-I options for compiling with FreeType],
		[cf_cv_x_freetype_incs="$with_freetype_cflags"],
		[cf_cv_x_freetype_incs=no])
	AC_MSG_RESULT($cf_cv_x_freetype_incs)

	AC_MSG_CHECKING(if you specified -L/-l options for FreeType)
	AC_ARG_WITH(freetype-libs,
		[  --with-freetype-libs    -L/-l options to link FreeType],
		[cf_cv_x_freetype_libs="$with_freetype_libs"],
		[cf_cv_x_freetype_libs=no])
	AC_MSG_RESULT($cf_cv_x_freetype_libs)
	;;
(auto)
	if test "$PKG_CONFIG" != none && "$PKG_CONFIG" --exists freetype2; then
		FREETYPE_CONFIG=$PKG_CONFIG
		FREETYPE_PARAMS=freetype2
	else
		AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, none)
		if test "$FREETYPE_CONFIG" != none; then
			cf_extra_freetype_libs="-lfreetype2"
		else
			AC_PATH_PROG(FREETYPE_OLD_CONFIG, xft-config, none)
			if test "$FREETYPE_OLD_CONFIG" != none; then
				FREETYPE_CONFIG=$FREETYPE_OLD_CONFIG
			fi
		fi
	fi
	;;
(pkg*)
	if test "$PKG_CONFIG" != none && "$PKG_CONFIG" --exists freetype2; then
		FREETYPE_CONFIG=$cf_cv_x_freetype_cfgs
		FREETYPE_PARAMS=freetype2
	else
		AC_MSG_WARN(cannot find pkg-config for freetype2)
	fi
	;;
(*)
	AC_PATH_PROG(FREETYPE_XFT_CONFIG, $cf_cv_x_freetype_cfgs, none)
	if test "$FREETYPE_XFT_CONFIG" != none; then
		FREETYPE_CONFIG=$FREETYPE_XFT_CONFIG
	else
		AC_MSG_WARN(cannot find config script for freetype2)
	fi
	;;
esac

if test "$FREETYPE_CONFIG" != none ; then
	AC_MSG_CHECKING(for FreeType config)
	AC_MSG_RESULT($FREETYPE_CONFIG $FREETYPE_PARAMS)

	if test "$cf_cv_x_freetype_incs" = no ; then
		AC_MSG_CHECKING(for $FREETYPE_CONFIG cflags)
		cf_cv_x_freetype_incs="`$FREETYPE_CONFIG $FREETYPE_PARAMS --cflags 2>/dev/null`"
		AC_MSG_RESULT($cf_cv_x_freetype_incs)
	fi

	if test "$cf_cv_x_freetype_libs" = no ; then
		AC_MSG_CHECKING(for $FREETYPE_CONFIG libs)
		cf_cv_x_freetype_libs="$cf_extra_freetype_libs `$FREETYPE_CONFIG $FREETYPE_PARAMS --libs 2>/dev/null`"
		AC_MSG_RESULT($cf_cv_x_freetype_libs)
	fi
fi

if test "$cf_cv_x_freetype_incs" = no ; then
	cf_cv_x_freetype_incs=
fi

if test "$cf_cv_x_freetype_libs" = no ; then
	cf_cv_x_freetype_libs=-lfreetype2
fi

AC_MSG_CHECKING(if we can link with FreeType library)

cf_save_ft_LIBS="$LIBS"
cf_save_ft_INCS="$CPPFLAGS"

CF_ADD_LIBS($cf_cv_x_freetype_libs)
CF_ADD_CFLAGS($cf_cv_x_freetype_incs)

AC_TRY_LINK([
#include <ft2build.h>
#include FT_FREETYPE_H
],[
	FT_Library alibrary;
	FT_Init_FreeType( &alibrary );],
	[cf_cv_found_freetype=yes],
	[cf_cv_found_freetype=no])
AC_MSG_RESULT($cf_cv_found_freetype)

if test "$cf_cv_found_freetype" = no ; then
	AC_MSG_WARN(No libraries found for FreeType)
	LIBS="$cf_save_ft_LIBS"
	CPPFLAGS="$cf_save_ft_INCS"
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_X_TOOLKIT version: 27 updated: 2023/01/11 04:05:23
dnl ------------
dnl Check for X Toolkit libraries
AC_DEFUN([CF_X_TOOLKIT],
[
AC_REQUIRE([AC_PATH_XTRA])
AC_REQUIRE([CF_CHECK_CACHE])

# OSX is schizoid about who owns /usr/X11 (old) versus /opt/X11 (new) (and
# in some cases has installed dummy files in the former, other cases replaced
# it with a link to the new location).  This complicates the configure script.
# Check for that pitfall, and recover using pkg-config
#
# If none of these are set, the configuration is almost certainly broken.
if test -z "${X_CFLAGS}${X_PRE_LIBS}${X_LIBS}${X_EXTRA_LIBS}"
then
	CF_TRY_PKG_CONFIG(x11,,[AC_MSG_WARN(unable to find X11 library)])
	CF_TRY_PKG_CONFIG(ice,,[AC_MSG_WARN(unable to find ICE library)])
	CF_TRY_PKG_CONFIG(sm,,[AC_MSG_WARN(unable to find SM library)])
	CF_TRY_PKG_CONFIG(xt,,[AC_MSG_WARN(unable to find Xt library)])
else
	LIBS="$X_PRE_LIBS $LIBS $X_EXTRA_LIBS"
fi

cf_have_X_LIBS=no

CF_TRY_PKG_CONFIG(xt,[

	case "x$LIBS" in
	(*-lX11*)
		;;
	(*)
# we have an "xt" package, but it may omit Xt's dependency on X11
AC_CACHE_CHECK(for usable X dependency,cf_cv_xt_x11_compat,[
AC_TRY_LINK([
$ac_includes_default
#include <X11/Xlib.h>
],[
	int rc1 = XDrawLine((Display*) 0, (Drawable) 0, (GC) 0, 0, 0, 0, 0);
	int rc2 = XClearWindow((Display*) 0, (Window) 0);
	int rc3 = XMoveWindow((Display*) 0, (Window) 0, 0, 0);
	int rc4 = XMoveResizeWindow((Display*)0, (Window)0, 0, 0, 0, 0);
],[cf_cv_xt_x11_compat=yes],[cf_cv_xt_x11_compat=no])])
		if test "$cf_cv_xt_x11_compat" = no
		then
			CF_VERBOSE(work around broken X11 dependency)
			# 2010/11/19 - good enough until a working Xt on Xcb is delivered.
			CF_TRY_PKG_CONFIG(x11,,[CF_ADD_LIB_AFTER(-lXt,-lX11)])
		fi
		;;
	esac

AC_CACHE_CHECK(for usable X Toolkit package,cf_cv_xt_ice_compat,[
AC_TRY_LINK([
$ac_includes_default
#include <X11/Shell.h>
],[int num = IceConnectionNumber(0); (void) num
],[cf_cv_xt_ice_compat=yes],[cf_cv_xt_ice_compat=no])])

	if test "$cf_cv_xt_ice_compat" = no
	then
		# workaround for broken ".pc" files used for X Toolkit.
		case "x$X_PRE_LIBS" in
		(*-lICE*)
			case "x$LIBS" in
			(*-lICE*)
				;;
			(*)
				CF_VERBOSE(work around broken ICE dependency)
				CF_TRY_PKG_CONFIG(ice,
					[CF_TRY_PKG_CONFIG(sm)],
					[CF_ADD_LIB_AFTER(-lXt,$X_PRE_LIBS)])
				;;
			esac
			;;
		esac
	fi

	cf_have_X_LIBS=yes
],[

	LDFLAGS="$X_LIBS $LDFLAGS"
	CF_CHECK_CFLAGS($X_CFLAGS)

	AC_CHECK_FUNC(XOpenDisplay,,[
	AC_CHECK_LIB(X11,XOpenDisplay,
		[CF_ADD_LIB(X11)])])

	AC_CHECK_FUNC(XtAppInitialize,,[
	AC_CHECK_LIB(Xt, XtAppInitialize,
		[AC_DEFINE(HAVE_LIBXT,1,[Define to 1 if we can compile with the Xt library])
		 cf_have_X_LIBS=Xt
		 LIBS="-lXt $LIBS"])])
])

if test "$cf_have_X_LIBS" = no ; then
	AC_MSG_WARN(
[Unable to successfully link X Toolkit library (-lXt) with
test program.  You will have to check and add the proper libraries by hand
to makefile.])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_X_XFT version: 2 updated: 2025/04/20 17:58:12
dnl --------
AC_DEFUN([CF_X_XFT],
[
AC_REQUIRE([CF_PKG_CONFIG])
AC_REQUIRE([CF_X_FREETYPE])

if test "$cf_cv_found_freetype" = yes
then

	cf_save_xft_LIBS="$LIBS"
	cf_save_xft_INCS="$CPPFLAGS"
	CF_TRY_PKG_CONFIG(xft,,[AC_MSG_WARN(unable to find Xft library with $PKG_CONFIG)])

	if test -z "$cf_pkgconfig_libs"
	then
		AC_CHECK_LIB(Xft, XftNameParse, LIBS="-lXft $LIBS")
	fi

	AC_MSG_CHECKING(if we can link with Xft and FreeType libraries)
	AC_TRY_LINK([
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xft/Xft.h>],[
		XftPattern  *pat = XftNameParse ("name"); (void)pat],
		[cf_cv_found_xft_libraries=yes],
		[cf_cv_found_xft_libraries=no])
	AC_MSG_RESULT($cf_cv_found_xft_libraries)

	if test "$cf_cv_found_xft_libraries" = yes ; then
		AC_DEFINE(XRENDERFONT,1,[Define to 1 if we can/should link with FreeType libraries])

		AC_CHECK_FUNCS( \
			XftDrawCharSpec \
			XftDrawSetClip \
			XftDrawSetClipRectangles )

	else
		LIBS="$cf_save_xft_LIBS"
		CPPFLAGS="$cf_save_xft_INCS"
	fi

# FIXME: revisit this if needed
AC_SUBST(HAVE_TYPE_FCCHAR32)
AC_SUBST(HAVE_TYPE_XFTCHARSPEC)

fi
])
dnl ---------------------------------------------------------------------------
dnl CF__GRANTPT_BODY version: 6 updated: 2021/06/07 17:39:17
dnl ----------------
dnl Body for workability check of grantpt.
define([CF__GRANTPT_BODY],[
	int code = 0;
	int pty;
	int tty;
	char *slave;
	struct termios tio;

	(void)tio;
	signal(SIGALRM, my_timeout);

	if (alarm(2) == 9)
		failed(9);
	else if ((pty = posix_openpt(O_RDWR)) < 0)
		failed(1);
	else if (grantpt(pty) < 0)
		failed(2);
	else if (unlockpt(pty) < 0)
		failed(3);
	else if ((slave = ptsname(pty)) == 0)
		failed(4);
#if (CONFTEST == 3) || defined(CONFTEST_isatty)
	else if (!isatty(pty))
		failed(4);
#endif
#if CONFTEST >= 4
    else if (tcgetattr(pty, &tio) < 0)
		failed(20);
    else if (tcsetattr(pty, TCSAFLUSH, &tio) < 0)
		failed(21);
#endif
	/* BSD posix_openpt does not treat pty as a terminal until slave is opened.
	 * Linux does treat it that way.
	 */
	else if ((tty = open(slave, O_RDWR)) < 0)
		failed(5);
#ifdef CONFTEST
#ifdef I_PUSH
#if (CONFTEST == 0) || defined(CONFTEST_ptem)
    else if (ioctl(tty, I_PUSH, "ptem") < 0)
		failed(10);
#endif
#if (CONFTEST == 1) || defined(CONFTEST_ldterm)
    else if (ioctl(tty, I_PUSH, "ldterm") < 0)
		failed(11);
#endif
#if (CONFTEST == 2) || defined(CONFTEST_ttcompat)
    else if (ioctl(tty, I_PUSH, "ttcompat") < 0)
		failed(12);
#endif
#endif /* I_PUSH */
#if CONFTEST >= 5
    else if (tcgetattr(tty, &tio) < 0)
		failed(30);
    else if (tcsetattr(tty, TCSAFLUSH, &tio) < 0)
		failed(31);
#endif
#endif /* CONFTEST */
    (void) tty;
	${cf_cv_main_return:-return}(code);
])
dnl ---------------------------------------------------------------------------
dnl CF__GRANTPT_HEAD version: 4 updated: 2020/03/10 18:53:47
dnl ----------------
dnl Headers for workability check of grantpt.
define([CF__GRANTPT_HEAD],[
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#ifndef HAVE_POSIX_OPENPT
#undef posix_openpt
#define posix_openpt(mode) open("/dev/ptmx", mode)
#endif

#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif

static void failed(int code)
{
	perror("conftest");
	exit(code);
}

static void my_timeout(int sig)
{
	(void)sig;
	exit(99);
}
])
dnl ---------------------------------------------------------------------------
dnl CF__XOPEN_SOURCE_BODY version: 2 updated: 2023/02/18 17:41:25
dnl ---------------------
dnl body of test when test-compiling for _XOPEN_SOURCE check
define([CF__XOPEN_SOURCE_BODY],
[
#ifndef _XOPEN_SOURCE
#error _XOPEN_SOURCE is not defined
#endif
])
dnl ---------------------------------------------------------------------------
dnl CF__XOPEN_SOURCE_HEAD version: 2 updated: 2023/02/18 17:41:25
dnl ---------------------
dnl headers to include when test-compiling for _XOPEN_SOURCE check
define([CF__XOPEN_SOURCE_HEAD],
[
$ac_includes_default
])
