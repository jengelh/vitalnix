#
#	configure.ac - gnu cold boilerplate
#	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2007 - 2008
#
#	This file is part of Vitalnix. Vitalnix is free software; you
#	can redistribute it and/or modify it under the terms of the GNU
#	Lesser General Public License as published by the Free Software
#	Foundation; either version 2.1 or 3 of the License.
#

AC_INIT([vitalnix], [3.2.0])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_INSTALL
AM_INIT_AUTOMAKE([-Wall])
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AM_PROG_AS
AC_DISABLE_STATIC
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_LANG_PUSH([C])
AC_COMPILE_IFELSE([
	#ifndef __i386__
	#	error Not i386.
	#endif
], [x86_32=1], [x86_32=0])
AC_LANG_POP([C])
# Select hand-tuned assembler code on x86_32.
AM_CONDITIONAL([X86_32], [test $x86_32 -eq 1])

#
#	cups, ldap
#
AC_CHECK_HEADERS([cups/backend.h ldap.h])
AM_CONDITIONAL([HAVE_CUPS], [test "$ac_cv_header_cups_backend_h" == "yes"])
AM_CONDITIONAL([HAVE_LDAP], [test "$ac_cv_header_ldap_h" == "yes"])

#
#	doc
#
AC_PATH_PROGS([PERL], [perl perl5])
AC_PATH_PROGS([PHP], [php php5])

AM_CONDITIONAL([GOT_TOOLS_FOR_DOC],
	[$am_make -v | grep -q GNU && test -n "$PERL" -a -n "$PHP"])

regular_CFLAGS="-D_FILE_OFFSET_BITS=64 \
	-D_REENTRANT -Wall -Waggregate-return -Wmissing-declarations \
	-Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes \
	-pipe";
regular_CXXFLAGS="-D_FILE_OFFSET_BITS=64 \
	-D_REENTRANT -Wall -Wno-pointer-arith -Wredundant-decls -pipe";

#
#	libc
#
AC_CHECK_HEADERS([crypt.h lastlog.h paths.h utmpx.h])
#dirstamp
AC_CHECK_MEMBERS(
	[struct stat.st_mtimensec, struct stat.st_mtim,
	struct stat.st_mtimespec, struct stat.st_mtime],
	[], [], [#include <sys/stat.h>])

#
#	openssl-devel, libxml-devel
#
PKG_CHECK_MODULES([libHX], [libHX >= 2.7])
PKG_CHECK_MODULES([libcrypto], [libcrypto >= 0.9.8])
PKG_CHECK_MODULES([libxml], [libxml-2.0 >= 2.6])

#
#	mysql
#
AC_MSG_CHECKING([for mysql])
mysql_config --cflags >/dev/null 2>/dev/null;
ret=$?;
libmysql_CFLAGS="";
libmysql_LIBS="";
AM_CONDITIONAL([HAVE_MYSQL], [test "$ret" -eq 0])
if test "$?" -eq 0; then
	z=$(mysql_config --cflags);
	for i in $z; do
		if test "${i:0:2}" = "-I"; then
			libmysql_CFLAGS="$libmysql_CFLAGS $i";
		fi;
	done;
	z=$(mysql_config --libs_r);
	for i in $z; do
		if test "${i:0:2}" = "-L" -o "${i:0:2}" = "-l"; then
			libmysql_LIBS="$libmysql_LIBS $i";
		fi;
	done;
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
fi;

#
#	pam
#
AC_CHECK_LIB([pam], [pam_authenticate], [have_pam=yes])
AM_CONDITIONAL([HAVE_PAM], [test "$have_pam" = "yes"])

#
#	wxWidgets
#
AC_MSG_CHECKING([for wxWidgets])
libwx_CFLAGS=`wx-config --cflags 2>/dev/null`;
ret=$?;
AM_CONDITIONAL([HAVE_WXWIDGETS], [test "$ret" -eq 0])
if [[ "$ret" -eq 0 ]]; then
	AC_MSG_RESULT([yes])
	libwx_LIBS=`wx-config --libs`;
else
	AC_MSG_RESULT([no])
fi;

AC_ARG_WITH([pkgconfigdir], AS_HELP_STRING([--with-pkgconfigdir=PATH],
	[Path to the pkgconfig directory [[LIBDIR/pkgconfig]]]),
	[pkgconfigdir="$withval"], [pkgconfigdir='${libdir}/pkgconfig'])
AC_SUBST([pkgconfigdir])

CHECK_GCC_FVISIBILITY

AC_SUBST([regular_CFLAGS])
AC_SUBST([regular_CXXFLAGS])
AC_SUBST([libmysql_CFLAGS])
AC_SUBST([libmysql_LIBS])
AC_SUBST([libwx_CFLAGS])
AC_SUBST([libwx_LIBS])

mkdir -p src/include/vitalnix;
if test -L src/include/vitalnix/config.h; then
	:;
elif test -e src/include/vitalnix/config.h; then
	AC_MSG_ERROR([src/include/vitalnix/config.h already exists and is not a symbolic link!])
else
	ln -s ../../../config.h src/include/vitalnix/config.h;
fi;

AC_CONFIG_FILES([Makefile etc/Makefile share/Makefile src/Makefile
	src/doc/GNUmakefile src/include/Makefile vitalnix.pc])
AC_OUTPUT
