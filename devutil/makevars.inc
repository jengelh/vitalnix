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

DEBUG    := 1

CC       := gcc
CXX      := g++
CFLAGS   := -D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
            -D_REENTRANT -DPIC -Wall -Waggregate-return \
            -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls \
            -Wshadow -Wstrict-prototypes -Winline -fPIC -pipe
CXXFLAGS := -D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
            -D_REENTRANT -DPIC -Wall -Wno-pointer-arith -Wredundant-decls \
            -fPIC -pipe
AS       := gcc # as
ASFLAGS  :=
LD       := gcc # ld
LDXX     := g++ # ld
LDFLAGS  := -L. -Wl,-rpath,.
SOFLAGS  := -shared
AR       := ar
ARFLAGS  :=
PHP      := php

ifeq (${DEBUG},1)
  CFLAGS   += -ggdb3
  CXXFLAGS += -ggdb3
  ASFLAGS  += -ggdb3
  STRIP    := @true
else
  CFLAGS   += -O2 -finline-functions -fomit-frame-pointer
  CXXFLAGS += -O2 -finline-functions -fomit-frame-pointer
  STRIP    := strip -s
endif
ifeq (${PROF},1)
  CFLAGS   += -pg -fprofile-arcs -ftest-coverage
  CXXFLAGS += -pg -fprofile-arcs -ftest-coverage
  LDFLAGS  += -pg
  STRIP    := @true
endif

CFLAGS   += -Iinclude -I. ${EXT_CFLAGS}
CXXFLAGS += -Iinclude -I. ${EXT_CXXFLAGS}
ASFLAGS  += ${EXT_ASFLAGS}
LDFLAGS  += ${EXT_LDFLAGS}
SOFLAGS  += ${EXT_SOFLAGS}
ARFLAGS  += ${EXT_ARFLAGS}

ifeq (${V},)
  CC    := @${CC}
  CXX   := @${CXX}
  AS    := @${AS}
  LD    := @${LD}
  LDXX  := @${LDXX}
  AR    := @${AR}
  STRIP := @${STRIP}
  PHP   := @${PHP}

  VECHO_DEP = @echo "  DEP   " $@
  VECHO_CC  = @echo "  CC    " $@
  VECHO_CXX = @echo "  CXX   " $@
  VECHO_AS  = @echo "  AS    " $@
  VECHO_LD  = @echo "  LD    " $@
  VECHO_AR  = @echo "  AR    " $@
  VECHO_PHP = @echo "  PHP   " $@
endif

MAKEFLAGS += -Rr

#==============================================================================
