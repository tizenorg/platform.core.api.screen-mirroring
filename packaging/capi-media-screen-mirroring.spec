Name:       capi-media-screen-mirroring
Summary:    A screen mirroring library in Tizen C API
Version:    0.1.34
Release:    0
Group:      Multimedia/API
License:    Apache License, Version 2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-base-1.0)
BuildRequires:  pkgconfig(gstreamer-rtsp-server-1.0)
BuildRequires:  pkgconfig(capi-network-wifi-direct)
BuildRequires:  pkgconfig(wifi-direct)
BuildRequires:  pkgconfig(mm-wfd)
BuildRequires:  pkgconfig(iniparser)

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description


%package devel
Summary:  A Screen Mirroring library in Tizen C API (Development)
Group:    Development/Multimedia
Requires: %{name} = %{version}-%{release}

%description devel

%prep
%setup -q


%build
#export CFLAGS+=" -Wcast-align"
#export CFLAGS+=" -Wextra -Wno-array-bounds"
#export CFLAGS+=" -Wno-ignored-qualifiers -Wno-unused-parameter -Wshadow"
#export CFLAGS+=" -Wwrite-strings -Wswitch-default"
#export CFLAGS+=" -Wall -Wcast-qual -Wno-empty-body"
#export CFLAGS+=" -Werror"

MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`

%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/usr/share/dbus-1/system-services/
install -m 755 org.tizen.scmirroring.server.service %{buildroot}/usr/share/dbus-1/system-services/
mkdir -p %{buildroot}/%{_datadir}/license
cp -rf %{_builddir}/%{name}-%{version}/LICENSE.APLv2.0 %{buildroot}%{_datadir}/license/%{name}

#License
mkdir -p %{buildroot}/%{_datadir}/license
cp -rf %{_builddir}/%{name}-%{version}/LICENSE.APLv2.0 %{buildroot}/%{_datadir}/license/%{name}

mkdir -p %{buildroot}/usr/etc
cp -rf config/scmirroring_src.ini %{buildroot}/usr/etc/scmirroring_src.ini

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest capi-media-screen-mirroring.manifest
%{_libdir}/libcapi-media-screen-mirroring.so*
%{_datadir}/dbus-1/system-services/org.tizen.scmirroring.server.service
#License
%{_datadir}/license/%{name}
%{_bindir}/miracast_server
/usr/etc/scmirroring_src.ini

%files devel
%{_includedir}/media/scmirroring_src.h
%{_includedir}/media/scmirroring_sink.h
%{_includedir}/media/scmirroring_type.h
%{_includedir}/media/scmirroring_src_ini.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-screen-mirroring.so


