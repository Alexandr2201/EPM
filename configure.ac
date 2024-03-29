dnl
dnl Configuration script for the ESP Package Manager (EPM).
dnl
dnl Copyright 1999-2019 by Michael R Sweet
dnl Copyright 1999-2007 by Easy Software Products, all rights reserved.
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2, or (at your option)
dnl any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl

AC_INIT([EPM],[4.4.1],[https://github.com/michaelrsweet/epm/issues],[epm],[https://michaelrsweet.github.io/epm])
AC_CONFIG_HEADER(config.h)

dnl Version number...

VERSION="AC_PACKAGE_VERSION"
VERNUMBER=441
AC_SUBST(VERSION)
AC_SUBST(VERNUMBER)
AC_DEFINE_UNQUOTED(EPM_VERSION, "ESP Package Manager v$VERSION")

dnl Get the operating system...

uname=`uname`
uversion=`uname -r | sed -e '1,$s/[[^0-9]]//g'`

if test "$uname" = "IRIX64"; then
    uname="IRIX"
fi

dnl Clear default debugging options and set normal optimization by
dnl default unless the user asks for debugging specifically.

CFLAGS="${CFLAGS:=}"
CXXFLAGS="${CXXFLAGS:=}"
LDFLAGS="${LDFLAGS:=}"
AC_SUBST(LDFLAGS)

AC_ARG_WITH(archflags, [  --with-archflags=flags  specify architecture flags [[default=none]]],
	ARCHFLAGS="$withval",
	ARCHFLAGS="${ARCHFLAGS:=}")
AC_SUBST(ARCHFLAGS)

AC_ARG_WITH(optim, [  --with-optim=flags      specify optimization flags [[default=auto]]],
	OPTIM="$withval",
	OPTIM="${OPTIM:=}")
AC_SUBST(OPTIM)

AC_ARG_ENABLE(debug, [  --enable-debug          turn on debugging [[default=no]]],
if eval "test x$enable_debug = xyes"; then
 	OPTIM="-g $OPTIM"
else
	LDFLAGS="$LDFLAGS -s"
fi)

AC_ARG_ENABLE(gui, [  --enable-gui            turn on GUI utilities [[default=auto]]])

AC_ARG_WITH(docdir, [  --with-docdir=/dir      set directory for documentation
                          [[default=${datadir}/doc/epm]]],
	docdir="$withval",
	docdir="\${datadir}/doc/epm")

AC_SUBST(docdir)

AC_ARG_WITH(softwaredir, [  --with-softwaredir=/dir set directory for EPM uninstall files and init scripts
                          [[default=/etc/software]]],
	softwaredir="$withval",
	softwaredir="")

dnl Checks for programs...
AC_PROG_CC(clang cc gcc)
AC_PROG_CXX(clang++ c++ g++)
AC_PROG_RANLIB
AC_PATH_PROG(AR,ar)
AC_PATH_PROG(CHMOD,chmod)
AC_PATH_PROG(CP,cp)
AC_PATH_PROG(GZIP,gzip)
if test x$GZIP = x; then
	AC_MSG_ERROR(GNU zip not found but required!)
fi
AC_PATH_PROG(HTMLDOC,htmldoc)
AC_PATH_PROG(MKDIR,mkdir)
AC_PATH_PROG(NROFF,nroff)
if test x$NROFF = x; then
	AC_PATH_PROG(GROFF,groff)
	if test x$GROFF = x; then
        	NROFF="echo"
	else
        	NROFF="$GROFF -T ascii"
	fi
fi
AC_PATH_PROG(RM,rm)
AC_PATH_PROG(RPM,rpm)
AC_PATH_PROG(RPMBUILD,rpmbuild)
AC_PATH_PROG(STRIP,strip)

dnl Architecture checks...
AC_C_CONST
AC_C_CHAR_UNSIGNED

if test "$uname" = "IRIX"; then
	if test `uname -r` = 6.5; then
		STRIP="$STRIP -f -s -k -l -h"
	fi
fi

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADER(strings.h,AC_DEFINE(HAVE_STRINGS_H))
AC_CHECK_HEADER(sys/mount.h,AC_DEFINE(HAVE_SYS_MOUNT_H))
AC_CHECK_HEADER(sys/param.h,AC_DEFINE(HAVE_SYS_PARAM_H))
AC_CHECK_HEADER(sys/statfs.h,AC_DEFINE(HAVE_SYS_STATFS_H))
AC_CHECK_HEADER(sys/vfs.h,AC_DEFINE(HAVE_SYS_VFS_H))

dnl Checks for string functions.
AC_CHECK_FUNCS(strcasecmp strdup strlcat strlcpy strncasecmp)
if test "$uname" = "HP-UX" -a "$uversion" = "1020"; then
	AC_MSG_WARN(Forcing snprintf emulation for HP-UX.)
else
	AC_CHECK_FUNCS(snprintf vsnprintf)
fi
AC_SEARCH_LIBS(gethostname, socket)

if test "x$enable_gui" != xno; then
	AC_PATH_PROG(FLTKCONFIG,fltk-config)
else
	FLTKCONFIG=""
fi

if test "x$FLTKCONFIG" = x; then
	GUIS=""
	GUILIBS=""
	INSTALL_GUIS=""
	INSTALL_OSX=""

	AC_MSG_WARN([Sorry, setup GUI requires FLTK 1.1.x or later.])
else
	CXXFLAGS="`$FLTKCONFIG --cflags` ${CXXFLAGS}"
	GUIS="setup uninst"
	GUILIBS="`$FLTKCONFIG --use-images --ldstaticflags`"
	INSTALL_GUIS="install-guis"

	if test "$uname" = Darwin; then
		GUILIBS="$GUILIBS -framework Security"
		INSTALL_OSX="install-macos"
	else
		INSTALL_OSX=""
	fi
fi

AC_SUBST(INSTALL_GUIS)
AC_SUBST(INSTALL_OSX)
AC_SUBST(GUIS)
AC_SUBST(GUILIBS)

dnl Set the default compression value...
case "$uname" in
	Darwin* | *BSD | Linux* | SunOS*)
		# macOS, *BSD, Linux, and Solaris come with gzip standard
		AC_DEFINE(EPM_COMPRESS, 1)
		;;

	*)
		# All other OS's do not.
		AC_DEFINE(EPM_COMPRESS, 0)
		;;
esac

dnl Update compiler options...
if test -n "$GCC"; then
	# Add standard optimization options...
	if test -z "$OPTIM"; then
        	OPTIM="-Os"
	fi

	# Add useful warning options...
	CFLAGS="-Wall -Wformat -Wno-format-y2k -Wunused $CFLAGS"
	CXXFLAGS="-Wall -Wformat -Wno-format-y2k -Wunused $CXXFLAGS"

	# See if GCC supports -Wno-format-truncation...
	AC_MSG_CHECKING(if GCC supports -Wno-format-truncation)
	OLDCFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -Werror -Wno-format-truncation"
	AC_TRY_COMPILE(,,[
		CFLAGS="$OLDCFLAGS -Wno-format-truncation"
		CXXFLAGS="$CXXFLAGS -Wno-format-truncation"
		AC_MSG_RESULT(yes)],[
		AC_MSG_RESULT(no)
		CFLAGS="$OLDCFLAGS"])

	# See if GCC supports -fno-rtti...
	AC_MSG_CHECKING(if GCC supports -fno-rtti)
	OLDCFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -fno-rtti"
	AC_TRY_COMPILE(,,
		CXXFLAGS="$CXXFLAGS -fno-rtti"
		AC_MSG_RESULT(yes),
		AC_MSG_RESULT(no))
	CFLAGS="$OLDCFLAGS"

	# See if GCC supports -fno-exceptions...
	AC_MSG_CHECKING(if GCC supports -fno-exceptions)
	OLDCFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -fno-exceptions"
	AC_TRY_COMPILE(,,
		CXXFLAGS="$CXXFLAGS -fno-exceptions"
		AC_MSG_RESULT(yes),
		AC_MSG_RESULT(no))
	CFLAGS="$OLDCFLAGS"

	# See if we are running HP-UX or Solaris; if so, try the
	# -fpermissive option...
	case $uname in
		SunOS* | HP-UX*)
			AC_MSG_CHECKING(if GCC supports -fpermissive)

			OLDCFLAGS="$CFLAGS"
			CFLAGS="$CFLAGS -fpermissive"
			AC_TRY_COMPILE(,,
				CXXFLAGS="$CXXFLAGS -fpermissive"
				AC_MSG_RESULT(yes),
				AC_MSG_RESULT(no))
			CFLAGS="$OLDCFLAGS"
			;;

		*)
			;;
	esac
else
	case $uname in
		AIX*)
			if test -z "$OPTIM"; then
				OPTIM="-O2"
			fi
			;;
		HP-UX*)
			if test -z "$OPTIM"; then
				OPTIM="+O2"
			fi
			OPTIM="$OPTIM +DAportable"
			;;
        	IRIX*)
			if test -z "$OPTIM"; then
        			OPTIM="-O2"
			fi
			if test $uversion -ge 62; then
				OPTIM="$OPTIM -n32 -mips3"
				OPTIM="$OPTIM -OPT:Olimit=3000"
				OPTIM="-fullwarn -woff 1209,1506,1692 $OPTIM"
			else
				OPTIM="-fullwarn $OPTIM"
			fi
			;;
		SunOS*)
			# Solaris
			if test -z "$OPTIM"; then
				OPTIM="-xO4"
			fi
			;;
		*)
			# Running some other operating system...
			echo "Building EPM with default compiler optimizations.  Set the CFLAGS and CXXFLAGS"
			echo "environment variable before running configure to override them."
			;;
	esac
fi

dnl Flags for "ar" command...
case $uname in
        Darwin* | *BSD*)
                ARFLAGS="-rc"
                ;;
        *)
                ARFLAGS="crs"
                ;;
esac

AC_SUBST(ARFLAGS)

dnl Fix "prefix" variable if it hasn't been specified...
if test "$prefix" = "NONE"; then
	prefix="/usr/local"
fi

dnl Fix "exec_prefix" variable if it hasn't been specified...
if test "$exec_prefix" = "NONE"; then
	exec_prefix="$prefix"
fi

dnl Fix "libdir" variable if it hasn't been specified...
if test "$libdir" = "\${exec_prefix}/lib"; then
	libdir="$exec_prefix/lib"
fi

dnl Fix "datarootdir" variable if it hasn't been specified...
if test "$datarootdir" = "\${prefix}/share"; then
	if test "$prefix" = "/"; then
		datarootdir="/usr/share"
	else
		datarootdir="$prefix/share"
	fi
fi

dnl Fix "datadir" variable if it hasn't been specified...
if test "$datadir" = "\${prefix}/share"; then
	if test "$prefix" = "/"; then
		datadir="/usr/share"
	else
		datadir="$prefix/share"
	fi
elif test "$datadir" = "\${datarootdir}"; then
	datadir="$datarootdir"
fi

dnl Fix "mandir" variable if it hasn't been specified...
if test "$mandir" = "\${prefix}/man" -a "$prefix" = "/usr"; then
	case "$uname" in
        	*BSD* | Darwin* | Linux*)
        		# BSD, Darwin (macOS), and Linux
        		mandir="/usr/share/man"
        		;;
        	IRIX*)
        		# SGI IRIX
        		mandir="/usr/share/catman/u_man"
        		;;
        	*)
        		# All others
        		mandir="/usr/man"
        		;;
	esac
fi

dnl EPM_SOFTWARE needs special attention for the default location...
if test x$softwaredir = x; then
	EPM_SOFTWARE="/etc/software"
else
	EPM_SOFTWARE="$softwaredir"
fi

AC_DEFINE_UNQUOTED(EPM_SOFTWARE, "$EPM_SOFTWARE")
AC_SUBST(EPM_SOFTWARE)

AC_DEFINE_UNQUOTED(EPM_GZIP, "$GZIP")
AC_SUBST(EPM_GZIP)

AC_DEFINE_UNQUOTED(EPM_STRIP, "$STRIP")
AC_SUBST(EPM_STRIP)

AC_DEFINE_UNQUOTED(EPM_BINDIR, "$exec_prefix/bin")
AC_SUBST(EPM_BINDIR)

AC_DEFINE_UNQUOTED(EPM_LIBDIR, "$libdir/epm")
AC_SUBST(EPM_LIBDIR)

AC_DEFINE_UNQUOTED(EPM_DATADIR, "$datadir/epm")
AC_SUBST(EPM_DATADIR)

dnl Figure out the right option to use for the RPM build architecture...
if test "x$RPM" != x; then
	# It would be nice if Red Hat could make up its mind...
	rpmversion="`$RPM --version | awk '{print $3}' | awk -F. '{print $1 * 10000 + $2 * 100 + $3}'`"

	if test $rpmversion -lt 30000; then
		AC_DEFINE_UNQUOTED(EPM_RPMARCH, "--buildarch ")
	else
		if test $rpmversion -ge 40003; then
			AC_DEFINE_UNQUOTED(EPM_RPMARCH, "--target ")
		else
			AC_DEFINE_UNQUOTED(EPM_RPMARCH, "--target=")
		fi
	fi
fi

if test "x$RPMBUILD" != x; then
	AC_DEFINE_UNQUOTED(EPM_RPMBUILD, "$RPMBUILD")
	AC_DEFINE(EPM_RPMTOPDIR)
else
	if test "x$RPM" != x; then
		AC_DEFINE_UNQUOTED(EPM_RPMBUILD, "$RPM")
	else
		AC_DEFINE_UNQUOTED(EPM_RPMBUILD, "rpm")
	fi
fi

dnl Output the makefile, list file, and config header...
AC_OUTPUT(Makefile epm.list doc/Makefile)
