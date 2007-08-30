#
#	configure.ac - gnu cold boilerplate
#	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2007
#
#	This file is part of Vitalnix. Vitalnix is free software; you
#	can redistribute it and/or modify it under the terms of the GNU
#	Lesser General Public License as published by the Free Software
#	Foundation; either version 2.1 or 3 of the License.
#

define(_xrelease, 3.1.0)
define(_xtmp, esyscmd(
	# strip off esyscmd newline
	s=`which svnversion`;
	if [[ -n "$s" -a -x "$s" ]]; then
		echo -en `$s`;
	fi;
))
AC_INIT(Vitalnix, _xrelease[]ifelse(_xtmp, exported, [], [_SVN]_xtmp))
AC_CONFIG_HEADERS(config.h)
AC_PROG_INSTALL
AM_INIT_AUTOMAKE
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AM_PROG_AS
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

case `uname -m` in
	(i?86) x86_32=1;;
	(*)    x86_32=0;;
esac;
AM_CONDITIONAL(X86_32, test $x86_32 -eq 1)

regular_CFLAGS="-D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
	-D_REENTRANT -Wall -Waggregate-return -Wmissing-declarations \
	-Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes \
	-Winline -fPIC -pipe";
regular_CXXFLAGS="-D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
	-D_REENTRANT -Wall -Wno-pointer-arith -Wredundant-decls -fPIC -pipe";

PKG_CHECK_MODULES([libcrypto], [libcrypto])
PKG_CHECK_MODULES([libxml], [libxml-2.0])
libmysql_CFLAGS_INSANE=`mysql_config --cflags`;
libmysql_LIBS_INSANE=`mysql_config --libs_r`;
libwx_CFLAGS=`wx-config --cflags`;
ret=$?
AM_CONDITIONAL(BUILD_STEELMILL, test "$ret" -eq 0)
if [[ "$ret" -eq 0 ]]; then
	libwx_LIBS=`wx-config --libs`;
fi;
PACKAGE_VERSION_SIMPLE="_xrelease";

m4_sinclude(m4lib/gcc4_visibility.m4)

AC_DEFINE_UNQUOTED(PACKAGE_VERSION_SIMPLE, ["_xrelease"], [Simplified package version])
AC_SUBST(PACKAGE_VERSION_SIMPLE)
AC_SUBST(regular_CFLAGS)
AC_SUBST(regular_CXXFLAGS)
AC_SUBST(libmysql_CFLAGS_INSANE)
AC_SUBST(libmysql_LIBS_INSANE)
AC_SUBST(libwx_CFLAGS)
AC_SUBST(libwx_LIBS)

mkdir -p include/vitalnix;
if [[ -L include/vitalnix/config.h ]]; then
	:;
elif [[ -e include/vitalnix/config.h ]]; then
	AC_MSG_ERROR([include/vitalnix/config.h already exists and is not a symbolic link!])
else
	ln -s ../../config.h include/vitalnix/config.h;
fi;

AC_OUTPUT(GNUmakefile)
