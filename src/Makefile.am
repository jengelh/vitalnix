# -*- Makefile -*-

BUILT_SOURCES =
CLEANFILES =

SUBDIRS = . include
if GOT_TOOLS_FOR_DOC
SUBDIRS += doc
endif

regular_DCONFIG = \
	-DCONFIG_SYSCONFDIR='"${sysconfdir}"' \
	-DCONFIG_LIBDIR='"${libdir}"'
regular_INCLUDES = \
	-Iinclude -I${srcdir}/include

AM_CFLAGS   = ${regular_CFLAGS} ${GCC_FVISIBILITY_HIDDEN} \
              ${regular_DCONFIG} ${regular_INCLUDES} ${libHX_CFLAGS}
AM_CXXFLAGS = ${regular_CXXFLAGS} ${GCC_FVISIBILITY_HIDDEN} \
              ${regular_DCONFIG} ${regular_INCLUDES} ${libHX_CFLAGS}

noinst_PROGRAMS    =
bin_PROGRAMS       =
sbin_PROGRAMS      =
noinst_LTLIBRARIES =
lib_LTLIBRARIES    =

.PHONY: .FORCE

.FORCE:

# *_SOURCES and *_LIBADD only impose a compile-time ordering,
# but because some of the generated libraries link against another,
# there is an install-time ordering that needs to be respected.
# (Since during install they get linked again, by libtool.)

# Level-1 libraries.
include libvxcgi/Automakefile
include libvxcli/Automakefile
include libvxcore/Automakefile
include libvxutil/Automakefile

# Level-2 libraries. Depend on level-1 libraries.
include libvxdb/Automakefile
include libvxeds/Automakefile
include libvxmdfmt/Automakefile

# Level-3 libraries. Depend on level-2 libraries.
include libvxmdsync/Automakefile
include drivers/Automakefile

# Programs
include cgiutils/Automakefile
include clutils/Automakefile
include devutil/Automakefile
include lpacct/Automakefile
include steelmill/Automakefile
