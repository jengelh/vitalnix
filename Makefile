#~~syntax:makefile
#==============================================================================
# Vitalnix User Management Suite
#   Copyright © Jan Engelhardt <jengelh [at] linux01 gwdg de>, 2003 - 2005
#   -- License restrictions apply (LGPL v2.1)
#
#   This file is part of Vitalnix.
#   Vitalnix is free software; you can redistribute it and/or modify it
#   under the terms of the GNU Lesser General Public License as published
#   by the Free Software Foundation; however ONLY version 2 of the License.
#
#   Vitalnix is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this program kit; if not, write to:
#   Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
#   Boston, MA  02110-1301  USA
#
#   -- For details, see the file named "LICENSE.LGPL2"
#==============================================================================

include devutil/makevars.inc

ifneq ($(shell uname -p | grep -Ei 'i.86|athlon'),)
  MCPU_IS_X86 := 1
else
  MCPU_IS_X86 := 0
endif

### Base targets --------------------------------------------------------------
#
.PHONY: all clean distclean

all: libvxadb.so \
     libvxam_shadow.so \
     cgi_chpasswd cgi_vwquota \
     cspark \
     listusers \
     accdbinfo \
     vxmgr vuseradd vusermod vuserdel vgroupadd vgroupmod vgroupdel \
     $(patsubst %.php,%.html,$(wildcard doc/[a-y]*.php))

clean:
	rm -f *.so *.so.[0-9]* */*.o cgi_chpasswd cgi_vwquota cspark \
	 listusers accdbinfo vxmgr v{user,group}{add,mod,del};

distclean: clean
	rm -f */.*.d doc/*.html;

%.o: %.c
	${VECHO_CC}
	${CC} ${CFLAGS} -Wp,-MMD,$(@D)/.$(@F).d,-MT,$@ -c -o $@ $<;

%.o: %.cpp
	${VECHO_CXX}
	${CXX} ${CXXFLAGS} -Wp,-MMD,$(@D)/.$(@F).d,-MT,$@ -c -o $@ $<;

%.o: %.S
	${VECHO_AS}
	${AS} ${ASFLAGS} -c -o $@ $<;

-include */.*.d

### Target: libaccdb ----------------------------------------------------------
#
LIBACCDB_OBJS := accdb/accdb.o accdb/bf.o accdb/phonemic.o
ifeq (${MCPU_IS_X86},1)
  LIBACCDB_OBJS += accdb/bf_x86.o
accdb/bf.o: accdb/bf.c
	${VECHO_CC}
	${CC} -DBF_ASM=1 ${CFLAGS} -Wp,-MMD,$(@D)/.$(@F).d,-MT,$@ -c -o $@ $<;
endif

libvxadb.so: ${LIBACCDB_OBJS}
	${VECHO_LD}
	${LD} ${LDFLAGS} ${SOFLAGS} -o $@ $^ -lHX -lcrypt;
	${STRIP} -s $@;

### Target: ACCDB back-end modules --------------------------------------------
#
libvxam_shadow.so: backends/shadow.o libvxadb.so
	${VECHO_LD}
	${LD} ${LDFLAGS} ${SOFLAGS} -o $@ $^ -lHX;
	${STRIP} -s $@;

### Target: ACCDB programs ----------------------------------------------------
#
vxmgr: sysprog/vxmgr.o sysprog/shared.o sysprog/useradd.o sysprog/usermod.o \
    sysprog/userdel.o sysprog/groupadd.o sysprog/groupmod.o \
    sysprog/groupdel.o libvxadb.so
	${VECHO_LD}
	${LD} ${LDFLAGS} -o $@ $^ -lHX;
	${STRIP} -s $@;

vuseradd: vxmgr
	ln -fs $< $@;

vusermod: vxmgr
	ln -fs $< $@;

vuserdel: vxmgr
	ln -fs $< $@;

vgroupadd: vxmgr
	ln -fs $< $@;

vgroupmod: vxmgr
	ln -fs $< $@;

vgroupdel: vxmgr
	ln -fs $< $@;

### C-spark -------------------------------------------------------------------
#
cspark: cui/main.o cui/autorun.o cui/data.o cui/pwlist.o cui/sync.o \
    cui/xml_in.o libvxadb.so
	${VECHO_LD}
	${LDXX} ${LDFLAGS} -o $@ $^ -lHX -lxml2;
	${STRIP} -s $@;

### Target: Supply programs ---------------------------------------------------
#
accdbinfo: supply/accdbinfo.o sysprog/shared.o libvxadb.so
	${VECHO_LD}
	${LD} ${LDFLAGS} -o $@ $^ -lHX;
	${STRIP} -s $@;

### Target: Development helper programs ---------------------------------------
#
listusers: devutil/listusers.o libvxadb.so
	${VECHO_LD}
	${LD} ${LDFLAGS} -o $@ $^;
	${STRIP} -s $@;

### CGI -----------------------------------------------------------------------
#
cgi_chpasswd: cgi/chpasswd.o cgi/base.o cgi/sh_auth.o
	${VECHO_LD}
	${LD} ${LDFLAGS} -o $@ $^ -lHX -lcrypt -lpam;

cgi_vwquota: cgi/cquota.o cgi/base.o cgi/sh_auth.o
	${VECHO_LD}
	${LD} ${LDFLAGS} -o $@ $^ -lHX -lcrypt -lpam;

### Documentation -------------------------------------------------------------
#
doc/%.html: doc/%.php
	${VECHO_PHP}
	${PHP} -q $< | perl -pe 's{href="([^/]*?)\.php"}{href="$$1.html"}' >$@;

#==============================================================================
