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
Name:     vitalnix
Version:  2.0.2
Release:  1
Group:    System/Base
Summary:  Vitalnix User Management Suite
Source:   http://prdownloads.sourceforge.net/%name/%name-%version.tbz2
License:  LGPL2
Vendor:   Vitalnix Project (http://vitalnix.sf.net/)
URL:      http://%name.sf.net/

BuildRoot:   %_tmppath/%name-%version-build
AutoReqProv: no
Provides:    libaccdb.so.0.1
Requires:    libHX.so.0.5 libc.so.6 libdl.so.2 libpopt.so.1 libpthread.so.0 libxml2.so.2

%description
Vitalnix is a suite consisting of a library providing unified methods to access
different user databases (Shadow, LDAP, etc), tools for user/group management,
and a program for managing users in batch, suitable for large systems.

Author(s):
----------
  Jan Engelhardt <jengelh [at] linux01 gwdg de>
  (Contributors see doc/index.html)

%prep
%setup

%build
cd "$RPM_BUILD_DIR"/%name-%version/;
./Mk EXT_CFLAGS="$RPM_OPT_FLAGS" DEBUG=0;

%install
b="$RPM_BUILD_ROOT";
[ "$b" != / -a -d "$b" ] && rm -Rfv "$b";
cd "$RPM_BUILD_DIR"/%name-%version/;

bindir="$b/bin";
etcdir="$b/etc";
usrbin="$b/usr/bin";
usrinc="$b/usr/include";
usrlib="$b/usr/lib";
usrsbin="$b/usr/sbin";
varlib="$b/var/lib";
usrdoc="$b""%_defaultdocdir";

# [accdb] libaccdb
install -dp "$etcdir"/libaccdb;
install -dp "$usrinc";
install -dp "$usrlib";
install -pm0644 vetc/autouid "$etcdir"/;
install -pm0644 vetc/libaccdb/accdb "$etcdir"/libaccdb/;
install -pm0644 include/accdb.h include/accdb_int.h "$usrinc"/;
install -pm0755 libaccdb.so.0.1 "$usrlib"/;
ln -fs libaccdb.so.0.1 "$usrlib"/libaccdb.so;

# [backends] ACCDB backend modules
install -dp "$etcdir"/libaccdb;
install -dp "$usrlib";
install -pm0644 vetc/libaccdb/shadow "$etcdir"/libaccdb/;
install -pm0755 accdb_shadow.so "$usrlib"/;

# [cui / cspark] C-Spark
install -dp "$usrlib"/%name;
install -pm0755 cspark "$usrlib"/%name;
install -pm0644 vetc/spark.cfg "$usrlib"/%name;

# [sysprog] Vitalnix System Programs
install -dp "$etcdir";
install -dp "$usrsbin";
install -pm0644 vetc/{user,group}{add,mod,del}.conf "$etcdir"/;
install -pm0755 v{user,group}{add,mod,del} "$usrsbin"/;

# [supply] Vitalnix Supply Programs
install -dp "$etcdir"/init.d;
install -dp "$usrlib"/%name;
install -pm0755 accdbinfo "$usrlib"/vitalnix;
install -pm0755 supply/{mkgroupdir,sb_DE.tex,sdf2xml,sg.rtf,sg_DE.rtf,sort_pwf} "$usrlib"/vitalnix;

# [doc] Vitalnix Documentation
install -d "$usrdoc";
cp -av doc "$usrdoc"/%name;

#------------------------------------------------------------------------------
# "win32$" tags for "makezip" program
%files
%defattr(-,root,root)

# [accdb] libaccdb
%config /etc/autouid
%config /etc/libaccdb/accdb
/usr/include/accdb.h
/usr/include/accdb_int.h
/usr/lib/libaccdb.so
/usr/lib/libaccdb.so.0.1
#win32$ vetc/autouid
#win32$ vetc/libaccdb/accdb
#win32$ include/accdb.h
#win32$ include/accdb_int.h
#win32$ libaccdb.0.1.dll

# [backends] ACCDB backend modules
%config /etc/libaccdb/shadow
/usr/lib/accdb_shadow.so
#win32$ vetc/libaccdb/shadow
#win32$ accdb_shadow.dll

# [cui / cspark] C-Spark
%config /usr/lib/vitalnix/spark.cfg
/usr/lib/vitalnix/cspark
#######/var/lib/vitalnix
#win32$ vetc/spark.cfg
#win32$ cspark.exe

# [sysprog] Vitalnix System Programs
%config /etc/useradd.conf
%config /etc/usermod.conf
%config /etc/userdel.conf
%config /etc/groupadd.conf
%config /etc/groupmod.conf
%config /etc/groupdel.conf
/usr/sbin/vuseradd
/usr/sbin/vusermod
/usr/sbin/vuserdel
/usr/sbin/vgroupadd
/usr/sbin/vgroupmod
/usr/sbin/vgroupdel
#win32$ vetc/useradd.conf
#win32$ vetc/usermod.conf
#win32$ vetc/userdel.conf
#win32$ vetc/groupadd.conf
#win32$ vetc/groupmod.conf
#win32$ vetc/groupdel.conf
#win32$ vuseradd.exe
#win32$ vusermod.exe
#win32$ vuserdel.exe
#win32$ vgroupadd.exe
#win32$ vgroupmod.exe
#win32$ vgroupdel.exe

# [supply] Vitalnix Supply Programs
%dir /usr/lib/%name
/usr/lib/%name/accdbinfo
# Flagged as config since you might have changed these
%config /usr/lib/%name/mkgroupdir
%config /usr/lib/%name/sb_DE.tex
%config /usr/lib/%name/sdf2xml
%config /usr/lib/%name/sg.rtf
%config /usr/lib/%name/sg_DE.rtf
%config /usr/lib/%name/sort_pwf
#win32$ accdbinfo.exe
#win32$ supply/mkgroupdir
#win32$ supply/sb_DE.tex
#win32$ supply/sdf2xml
#win32$ supply/sg.rtf
#win32$ supply/sg_DE.rtf
#win32$ supply/sort_pwf

# [doc] Vitalnix Documentation
%docdir %_defaultdocdir/%name
%_defaultdocdir/%name
#win32$ doc

# [win32] Additional libraries needed
#win32$ libHX.0.5.dll
#win32$ libiconv-2.dll
#win32$ libintl-2.dll
#win32$ libxml2.dll
#win32$ popt1.dll

#==============================================================================
