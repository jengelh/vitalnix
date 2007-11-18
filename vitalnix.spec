
Name:		vitalnix
Version:	3.1.0
Release:	1
Group:		System/Base
Summary:	Vitalnix User Management Suite and Essential Tools
License:	LGPL
URL:		http://vitalnix.sourceforge.net/

Source:		http://heanet.dl.sourceforge.net/sourceforge/%name/%name-%version.tar.bz2
BuildRoot:	%_tmppath/%name-%version-build
BuildRequires:	cups-devel, gcc-c++
BuildRequires:	libHX-devel >= 1.10, libxml2-devel, mysql-devel >= 5.0
BuildRequires:	openldap2-devel
BuildRequires:	openssl-devel, pam-devel, perl >= 5.6.0, php >= 5, pkg-config
BuildRequires:	w3m, wxGTK-devel >= 2.7.0
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

%debug_package
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
ln -s "%pfx/%_lib/pam_ihlogon.so" "$b/%_lib/security/";
ln -s "%pfx/sbin/lpacct_scv" "$b/%_libdir/cups/backend/scv";
ln -s "%pfx/share/vitalnix/vitalnix.schema" "$b/%_sysconfdir/openldap/schema/";

for i in vxrandpw; do
	ln -s "%pfx/bin/$i" "$b/%_bindir/";
done;
for i in md{ckuuid,fixuuid,pwlfmt,single,sync} vx{finger,tryauth} \
    vx{user,group}{add,mod,del}; do
	ln -s "%pfx/sbin/$i" "$b/%_sbindir/";
done;

rm -f "$b/%pfx/%_lib"/*.la;

%clean
rm -Rf "%buildroot";

%files
%defattr(-,root,root)
%config(noreplace) %_sysconfdir/%name/*
%_sysconfdir/openldap/schema/*
/%_lib/security/*
%_libdir/cups/backend/*
%_libdir/pkgconfig/*
%_bindir/*
%_sbindir/*
%dir %pfx
%pfx/bin
%pfx/sbin
%pfx/include
%pfx/%_lib
%dir %pfx/share
%dir %pfx/share/vitalnix
%config(noreplace) %pfx/share/vitalnix/*
%doc src/doc/*.html src/doc/*.css
