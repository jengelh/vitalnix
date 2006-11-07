
Name:           vitalnix
Version:        2.99_beta8
Release:        0
Group:          System/Base
Summary:        Vitalnix User Management Suite and Essential Tools
License:        LGPL2
URL:            http://vitalnix.sourceforge.net/

Source:         http://heanet.dl.sourceforge.net/sourceforge/%name/%name-%version.tar.bz2
BuildRoot:      %_tmppath/%name-%version-build
BuildRequires:  openssl-devel, perl >= 5.6.0, rsync, wxWidgets-devel >= 2.7.0
%define pfx     /opt/%name-%version

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

%prep
%setup

%build
mkdir obj;
pushd obj;
../configure \
    --sysconfdir="%_sysconfdir/vitalnix" \
    --includedir="%pfx/include" \
    --libdir="%pfx/%_lib" \
    CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS";
make %{?jobs:-j%jobs};
popd;

%install
b="%buildroot";
rm -Rf "$b";

# /etc
mkdir -p "$b/%_sysconfdir";
cp -a etc "$b/%_sysconfdir/%name";

# /usr/bin
install -dm0755 "$b/%_bindir";
install -pm0755 obj/vitalnix-config "$b/%_bindir/";

# /usr/lib/pkgconfig
install -dm0755 "$b/%_libdir/pkgconfig";
install -pm0644 obj/vitalnix.pc "$b/%_libdir/pkgconfig/";

# /opt/vitalnix3
install -dm0755 "$b/%pfx";

# ./include
rsync -PHSav src/include/ "$b/%pfx/include/";
rsync -PHSav obj/include/ "$b/%pfx/include/";

# ./bin
install -dm0755 "$b/%pfx/bin";
find obj -maxdepth 1 -type f ! -iname "*.so" -perm +111 -exec install -pm0755 "{}" "$b/%pfx/bin/" ";";

# ./lib
install -dm0755 "$b/%pfx/%_lib";
find obj -maxdepth 1 -type f -iname "*.so" -perm +111 -exec install -pm0755 "{}" "$b/%pfx/%_lib/" ";";

# ./share
install -dm0755 "$b/%pfx/share";
install -pm0644 share/* "$b/%pfx/share/";

%clean
rm -Rf "%buildroot";

%files
%defattr(-,root,root)
%config(noreplace) %_sysconfdir/%name/*
%_bindir/*
%_libdir/pkgconfig/*
%dir %pfx
%pfx/bin
%pfx/include
%pfx/%_lib
%dir %pfx/share
%config(noreplace) %pfx/share/*
%doc obj/doc/*

%changelog -n vitalnix
