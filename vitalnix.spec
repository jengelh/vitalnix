
Name:		vitalnix
Version:	3.3~beta1
Release:	0
Group:		System/Base
Summary:	Vitalnix User Management Suite and Essential Tools
License:	LGPL
URL:		http://vitalnix.sourceforge.net/

#Source:		http://downloads.sf.net/sourceforge/%name/%name-%version.tar.bz2
Source:		vitalnix-%version.tar.bz2
BuildRoot:	%_tmppath/%name-%version-build
BuildRequires:	cups-devel >= 1.3, gcc-c++
BuildRequires:	libHX-devel >= 3.0.1
BuildRequires:	libmysqlclient-devel >= 5.0
BuildRequires:	libxml2-devel >= 2.6
BuildRequires:	openldap2-devel >= 2.3, openssl-devel >= 0.9.7
BuildRequires:	pam-devel >= 0.99, perl >= 5.8.0, php >= 5.0
BuildRequires:	pkg-config >= 0.19, w3m >= 0.5.1, wxGTK-devel >= 2.7.0
%define pfx	/opt/%name-%version

%description
Vitalnix is a suite consisting of a library providing unified methods
to access different user databases (Shadow, LDAP, etc), tools for
user/group management, and a program for managing users in batch,
suitable for large systems.

Developers:
-----------
	Jan Engelhardt
	Cordula Petzold
	(Contributors see doc/index.html)

%package devel
Group:		Development/Libraries/C and C++
Summary:	Development files for Vitalnix
Requires:	vitalnix = %version-%release

%description devel
This package contains the development files for Vitalnix.

Developers:
-----------
	Jan Engelhardt
	Cordula Petzold

%prep
%setup -q

%build
if [ ! -e configure ]; then
	./autogen.sh;
fi;
%configure \
	--sysconfdir="%_sysconfdir/vitalnix" \
	--prefix="%pfx" \
	--bindir="%pfx/bin" \
	--sbindir="%pfx/sbin" \
	--includedir="%pfx/include" \
	--libdir="%pfx/%_lib" \
	--datadir="%pfx/share" \
	--with-pkgconfigdir="%_libdir/pkgconfig";
make %{?jobs:-j%jobs};

%install
b="%buildroot";
rm -Rf "$b";
mkdir "$b";
make install DESTDIR="$b";
install -dm0755 \
	"$b/%_sysconfdir/openldap/schema" \
	"$b/%_lib/security" \
	"$b/%_libdir/cups/backend" \
	"$b/%_bindir" \
	"$b/%_sbindir";
rm -f "%pfx/%_lib"/drv_*.la "%pfx/%_lib"/pam_ihlogon.la;

ln -s "%pfx/%_lib/pam_ihlogon.so" "$b/%_lib/security/";
ln -s "%pfx/sbin/lpacct_scv" "$b/%_libdir/cups/backend/scv";
ln -s "%pfx/share/vitalnix/vitalnix.schema" "$b/%_sysconfdir/openldap/schema/";

for i in vx{finger,id,randpw}; do
	ln -s "%pfx/bin/$i" "$b/%_bindir/";
done;
for i in vx{ckuuid,dbdump,fixuuid,newuser,pwlfmt,tryauth} \
    vxgroup{add,bld,mod,del} vxuser{add,mod,del,sync}; do
	ln -s "%pfx/sbin/$i" "$b/%_sbindir/";
done;

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%dir %_sysconfdir/%name
%config(noreplace) %_sysconfdir/%name/*
%_sysconfdir/openldap/schema
/%_lib/security/*
%_libdir/cups
%_bindir/*
%_sbindir/*
%dir %pfx
%pfx/bin
%pfx/sbin
%dir %pfx/%_lib
%pfx/%_lib/lib*.so.*
%pfx/%_lib/drv_*.so
%pfx/%_lib/pam_*.so
%dir %pfx/share
%dir %pfx/share/vitalnix
%config(noreplace) %pfx/share/vitalnix/*
%doc %_mandir/*/*.[178].*
%doc src/doc/*.[178].html src/doc/*.css src/doc/*.png

%files devel
%defattr(-,root,root)
%_libdir/pkgconfig/*
%pfx/include
%pfx/%_lib/lib*.so
%pfx/%_lib/*.la
%doc %_mandir/*/*.3.*
%doc src/doc/*.3.html

%changelog
