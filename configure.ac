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

AC_CFLAGS="-D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
	-D_REENTRANT -Wall -Waggregate-return -Wmissing-declarations \
	-Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes \
	-Winline -fPIC -pipe";
AC_CXXFLAGS="-D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
	-D_REENTRANT -Wall -Wno-pointer-arith -Wredundant-decls -fPIC -pipe";

CFLAGS_LIBCRYPTO=`pkg-config libcrypto --cflags`;
LDFLAGS_LIBCRYPTO=`pkg-config libcrypto --libs`;
CFLAGS_LIBMYSQL_INSANE=`mysql_config --cflags`;
LDFLAGS_LIBMYSQL_INSANE=`mysql_config --libs_r`;
CFLAGS_LIBXML=`pkg-config libxml-2.0 --cflags`;
LDFLAGS_LIBXML=`pkg-config libxml-2.0 --libs`;
CXXFLAGS_LIBWX=`wx-config --cflags`;
LDFLAGS_LIBWX=`wx-config --libs`;
PACKAGE_VERSION_SIMPLE="_xrelease";

m4_sinclude(m4lib/gcc4_visibility.m4)

AC_DEFINE_UNQUOTED(PACKAGE_VERSION_SIMPLE, ["_xrelease"], [Simplified package version])
AC_SUBST(PACKAGE_VERSION_SIMPLE)
AC_SUBST(AC_CFLAGS)
AC_SUBST(AC_CXXFLAGS)
AC_SUBST(CFLAGS_LIBCRYPTO)
AC_SUBST(LDFLAGS_LIBCRYPTO)
AC_SUBST(CFLAGS_LIBMYSQL_INSANE)
AC_SUBST(LDFLAGS_LIBMYSQL_INSANE)
AC_SUBST(CFLAGS_LIBXML)
AC_SUBST(LDFLAGS_LIBXML)
AC_SUBST(CXXFLAGS_LIBWX)
AC_SUBST(LDFLAGS_LIBWX)

mkdir -p include/vitalnix;
if [[ -L include/vitalnix/config.h ]]; then
	:;
elif [[ -e include/vitalnix/config.h ]]; then
	AC_MSG_ERROR([include/vitalnix/config.h already exists and is not a symbolic link!])
else
	ln -s ../../config.h include/vitalnix/config.h;
fi;

AC_OUTPUT(GNUmakefile)
