# -*- Makefile -*-

.POSIX:

DEBUG    := 1
V        := 0

CC       := gcc
CXX      := g++
CFLAGS   := -D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
            -D_REENTRANT -DPIC -Wall -Waggregate-return \
            -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls \
            -Wshadow -Wstrict-prototypes -Winline -fPIC -fvisibility=hidden \
            -pipe \
            $(if ${S},-I${S},) $(if ${O},-I${O},)
CXXFLAGS := -D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
            -D_REENTRANT -DPIC -Wall -Wno-pointer-arith -Wredundant-decls \
            -fPIC -fvisibility=hidden -pipe \
            $(if ${S},-I${S},) $(if ${O},-I${O},)
DEPFLAGS  = -Wp,-MMD,$(@D)/.$(@F).d,-MT,$@ 
AS       := gcc # as
ASFLAGS  :=
LD       := gcc # ld
LDXX     := g++ # ld
LDFLAGS  := -L. -Wl,-rpath-link,.
SOFLAGS  := -shared
AR       := ar
ARFLAGS  :=

MYSQL_CFLAGS   := $(filter -I%,$(shell mysql_config --cflags))
MYSQL_LDFLAGS  := $(filter -L% -l% -pthread,$(shell mysql_config --libs_r))
LIBXML_CFLAGS  := $(shell xml2-config --cflags)
LIBXML_LDFLAGS := $(shell xml2-config --libs)
WX_CXXFLAGS    := $(shell wx-config --cxxflags)
WX_LDFLAGS     := $(shell wx-config --libs)

ifeq (${DEBUG},1)
  CFLAGS   += -ggdb3
  CXXFLAGS += -ggdb3
  ASFLAGS  += -ggdb3
  LDFLAGS  += -Wl,-rpath,${O}
  STRIP    := true
else
  CFLAGS   += -O2 -finline-functions -fomit-frame-pointer
  CXXFLAGS += -O2 -finline-functions -fomit-frame-pointer
  LDFLAGS  += -Wl,-O1
  STRIP    := strip -s
endif
ifeq (${PROF},1)
  CFLAGS   += -pg -fprofile-arcs -ftest-coverage
  CXXFLAGS += -pg -fprofile-arcs -ftest-coverage
  LDFLAGS  += -pg -fprofile-arcs -ftest-coverage
  STRIP    := true
endif

CFLAGS   += ${EXT_CFLAGS}
CXXFLAGS += ${EXT_CXXFLAGS}
ASFLAGS  += ${EXT_ASFLAGS}
#LDFLAGS  += ${EXT_LDFLAGS}
SOFLAGS  += ${EXT_SOFLAGS}
ARFLAGS  += ${EXT_ARFLAGS}

ifeq (${V},0)
  Q := @
  VECHO_CC  = ${Q}echo "  CC      " $@
  VECHO_CXX = ${Q}echo "  CXX     " $@
  VECHO_AS  = ${Q}echo "  AS      " $@
  VECHO_LD  = ${Q}echo "  LD      " $@
  VECHO_AR  = ${Q}echo "  AR      " $@
  VECHO_GEN = ${Q}echo "  GEN     " $@
else
  Q :=
endif

PREREQ = @mkdir -p $(@D)
srcobj = $(patsubst ${S}/%.c,%.o,$(wildcard ${S}/$(1)/*.c)) \
         $(patsubst ${S}/%.cpp,%.o,$(wildcard ${S}/$(1)/*.cpp))

#==============================================================================