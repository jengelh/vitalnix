
Name:		vitalnix
Version:	3.1.0
Release:	0
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
%setup

%build
%configure \
	--sysconfdir="%_sysconfdir/vitalnix" \
	--bindir="%pfx/bin" \
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
install -dm0755 "$b/%_lib/security" "$b/%_libdir/cups/backend";
ln -s "%pfx/%_lib/pam_ihlogon.so" "$b/%_lib/security/";
ln -s "%pfx/bin/lpacct_scv" "$b/%_libdir/cups/backend/scv";

%clean
rm -Rf "%buildroot";

%files
%defattr(-,root,root)
%config(noreplace) %_sysconfdir/%name/*
/%_lib/security/*
%_libdir/cups/backend/*
%_libdir/pkgconfig/*
%dir %pfx
%pfx/bin
%pfx/include
%pfx/%_lib
%dir %pfx/share
%config(noreplace) %pfx/share/*
#%doc obj/doc/*

%changelog -n vitalnix
