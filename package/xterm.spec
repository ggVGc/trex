# $XTermId: xterm.spec,v 1.181 2025/06/01 14:41:00 tom Exp $
Summary: X terminal emulator (development version)
%global my_middle xterm
%global my_suffix -dev
%global fullname %{my_middle}%{my_suffix}
%global my_class XTermDev
Name: %{fullname}
Version: 400
Release: 1
License: X11
Group: User Interface/X
Source: https://invisible-island.net/archives/xterm/xterm-%{version}.tgz
URL: https://invisible-island.net/xterm/
Provides: x-terminal-emulator >= %{version}

# Using "find-xterm-fonts" gives these package names:
#
# fedora:
#     xorg-x11-fonts-misc
# mageia:
#     x11-font-adobe-75dpi
#     x11-font-misc-misc
# opensuse:
#     efont-unicode-bitmap-fonts
#     xorg-x11-fonts-core
#     xorg-x11-fonts-legacy

# Library dependencies (chiefly Xft and Xpm) are similar across my test
# machines for mageia/fedora/opensuse, but package names vary:
#
# fedora:
#     libXft            libXft-devel
#     libXpm            libXpm-devel
#     libutempter       libutempter-devel
#     ncurses-libs      ncurses-devel
# mageia
#     lib64xft2         lib64xft-devel
#     lib64xpm4         lib64xpm-devel
#     lib64utempter0    lib64utempter-devel
#     lib64ncurses6     lib64ncurses-devel
# opensuse
#     libXft2           libXft-devel
#     libXpm4           libXpm-devel
#     libutempter0      utempter-devel
#     libncurses6       ncurses-devel

# This part (the build-requires) would be useful if the various distributions
# had provided stable package-naming, or virtual packages to cover transitions. 
# However, they have not done this in the past.
%define use_x_manpage %(test "x$_use_x_manpage" = xyes && echo 1 || echo 0)
%if "%{use_x_manpage}"

%global is_mandriva %(test -f /etc/mandriva-release && echo %{use_x_manpage} || echo 0)
%global is_redhat   %(test -f /etc/redhat-release && echo %{use_x_manpage} || echo 0)
%global is_suse     %(test -f /etc/SuSE-release && echo %{use_x_manpage} || echo 0)

%if %{is_mandriva}
BuildRequires: x11-docs
%else
%if %{is_redhat}
# missing in Fedora 37:
## BuildRequires: xorg-x11-docs
BuildRequires: ncurses-devel
%else
%if %{is_suse}
BuildRequires: xorg-docs
%endif
%endif
%endif

%endif

%description
xterm is the standard terminal emulator for the X Window System.
It provides DEC VT102 and Tektronix 4014 compatible terminals for
programs that cannot use the window system directly.  This version
implements ISO/ANSI colors, Unicode, and most of the control sequences
used by DEC VT220 terminals.

This package provides four commands:
 a) %{fullname}, which is the actual terminal emulator
 b) u%{fullname}, which is a wrapper around %{fullname}
    which sets %{fullname} to use UTF-8 encoding when
    the user's locale supports this,
 c) koi8r%{fullname}, a wrapper similar to u%{fullname}
    for locales that use the KOI8-R character set, and
 d) resize%{my_suffix}.

A complete list of control sequences supported by the X terminal emulator
is provided in /usr/share/doc/%{fullname}.

The %{fullname} program uses bitmap images provided by the xbitmaps package.

Those interested in using koi8r%{fullname} will likely want to install the
xfonts-cyrillic package as well.

This package is configured to use "%{fullname}" and "%{my_class}"
for the program and its resource class, to avoid conflict with other packages.

%prep

%global target_appdata %{fullname}.appdata.xml

%define desktop_utils   %(if which desktop-file-install 2>&1 >/dev/null ; then echo 1 || echo 0 ; fi)
%define icon_theme  %(test -d /usr/share/icons/hicolor && echo 1 || echo 0)
%define apps_x11r6  %(test -d /usr/X11R6/lib/X11/app-defaults && echo 1 || echo 0)
%define apps_shared %(test -d /usr/share/X11/app-defaults && echo 1 || echo 0)
%define apps_syscnf %(test -d /etc/X11/app-defaults && echo 1 || echo 0)

%if %{apps_x11r6}
%define _xresdir    %{_prefix}/X11R6/lib/X11/app-defaults
%else
%if %{apps_shared}
%define _xresdir    %{_datadir}/X11/app-defaults
%else
%define _xresdir    %{_sysconfdir}/X11/app-defaults
%endif
%endif

%define _iconsdir   %{_datadir}/icons
%define _pixmapsdir %{_datadir}/pixmaps
%define my_docdir   %{_datadir}/doc/%{fullname}

# no need for debugging symbols...
%define debug_package %{nil}

%setup -q -n xterm-%{version}

%build
CPPFLAGS="-DMISC_EXP -DEXP_HTTP_HEADERS" \
%configure \
        --target %{_target_platform} \
        --prefix=%{_prefix} \
        --bindir=%{_bindir} \
        --datadir=%{_datadir} \
        --mandir=%{_mandir} \
%if "%{my_suffix}" != ""
        --program-suffix=%{my_suffix} \
        --without-xterm-symlink \
%endif
%if "%{icon_theme}"
        --with-icon-symlink=%{fullname} \
        --with-icon-theme \
        --with-icondir=%{_iconsdir} \
%endif
        --with-app-class=%{my_class} \
        --disable-imake \
        --enable-dabbrev \
        --enable-dec-locator \
        --enable-exec-xterm \
        --enable-hp-fkeys \
        --enable-load-vt-fonts \
        --enable-logfile-exec \
        --enable-logging \
        --enable-mini-luit \
        --enable-regis-graphics \
        --enable-sco-fkeys \
        --enable-toolbar \
        --enable-xmc-glitch \
        --with-app-defaults=%{_xresdir} \
        --with-pixmapdir=%{_pixmapsdir} \
        --with-own-terminfo=%{_datadir}/terminfo \
        --with-terminal-type=xterm-new \
        --with-utempter \
        --with-icon-name=mini.xterm \
        --with-xpm
make

chmod u+w XTerm.ad
cat >>XTerm.ad <<EOF
*backarrowKeyIsErase: true
*ptyInitialErase: true
EOF
ls -l *.ad

%install
rm -rf $RPM_BUILD_ROOT

# Usually do not use install-ti, since that will conflict with ncurses.
make install-bin install-man install-app install-icon \
%if "%{install_ti}" == "yes"
        install-ti \
%endif
        DESTDIR=$RPM_BUILD_ROOT \
        TERMINFO=%{_datadir}/terminfo

        mkdir -p $RPM_BUILD_ROOT%{my_docdir}
        cp \
                ctlseqs.txt \
                README.i18n \
                THANKS \
                xterm.log.html \
        $RPM_BUILD_ROOT%{my_docdir}/

        cp -r vttests \
        $RPM_BUILD_ROOT%{my_docdir}/

        # The scripts are readable, but not executable, to let find-requires
        # know that they do not depend on Perl packages.
        chmod 644 $RPM_BUILD_ROOT%{my_docdir}/vttests/*.pl

%if "%{desktop_utils}"
make install-desktop \
        DESKTOP_FLAGS="--dir $RPM_BUILD_ROOT%{_datadir}/applications"

test -n "%{my_suffix}" && \
( cd $RPM_BUILD_ROOT%{_datadir}/applications
        for p in *.desktop
        do
                mv $p `basename $p .desktop`%{my_suffix}.desktop
        done
)

mkdir -p $RPM_BUILD_ROOT%{_datadir}/appdata && \
install -m 644 xterm.appdata.xml $RPM_BUILD_ROOT%{_datadir}/appdata/%{target_appdata} && \
( cd $RPM_BUILD_ROOT%{_datadir}/appdata
  sed -i \
      -e 's/>xterm\.desktop</>%{fullname}.desktop</' \
      -e 's/>XTerm</>%{my_class}</' \
      %{target_appdata}
)
diff -u xterm.appdata.xml $RPM_BUILD_ROOT%{_datadir}/appdata/%{target_appdata} && \
%endif

%post
%if "%{icon_theme}"
touch --no-create %{_iconsdir}/hicolor
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
  %{_bindir}/gtk-update-icon-cache %{_iconsdir}/hicolor || :
fi
%endif
# find-requires does not care about at this point
chmod +x %{my_docdir}/vttests/*.*

%postun
%if "%{icon_theme}"
touch --no-create %{_iconsdir}/hicolor
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
  %{_bindir}/gtk-update-icon-cache %{_iconsdir}/hicolor || :
fi
%endif

%files
%defattr(-,root,root,-)
%{_bindir}/koi8r%{fullname}
%{_bindir}/%{fullname}
%{_bindir}/u%{fullname}
%{_bindir}/resize%{my_suffix}
%{_mandir}/*/*
%{my_docdir}/*
%{_xresdir}/*XTerm*

%if "%{install_ti}" == "yes"
%{_datadir}/terminfo/*
%endif

%if "%{desktop_utils}"
%config(missingok) %{_datadir}/appdata/%{target_appdata}
%config(missingok) %{_datadir}/applications/%{fullname}.desktop
%config(missingok) %{_datadir}/applications/u%{fullname}.desktop
%endif

%if "%{icon_theme}"
%{_iconsdir}/hicolor/*.png
%{_iconsdir}/hicolor/*.svg
%{_iconsdir}/hicolor/48x48/apps/*.png
%{_iconsdir}/hicolor/scalable/apps/*.svg
%endif
%{_pixmapsdir}/*.xpm

%changelog

* Fri Nov 25 2022 Thomas E. Dickey
- Fedora 37 has no xorg-x11-docs

* Thu Feb 24 2022 Thomas E. Dickey
- double-buffer is not enabled by default

* Sat Jul 25 2020 Thomas E. Dickey
- sixels are enabled by default

* Sun Mar 08 2020 Thomas E. Dickey
- remove "--vendor" option from desktop-file-install

* Sun Nov 17 2019 Thomas E. Dickey
- install appdata.xml file

* Wed May 02 2018 Thomas E. Dickey
- install all icons

* Fri Jan 29 2016 Thomas E. Dickey
- use --enable-screen-dumps

* Mon May 27 2013 Thomas E. Dickey
- use --with-icon-symlink

* Mon Oct 08 2012 Thomas E. Dickey
- added to pixmapsdir

* Fri Jun 15 2012 Thomas E. Dickey
- modify to support icon theme

* Fri Oct 22 2010 Thomas E. Dickey
- initial version.
