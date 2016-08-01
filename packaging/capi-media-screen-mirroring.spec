Name:       capi-media-screen-mirroring
Summary:    A screen mirroring library in Tizen C API
Version:    0.1.49
Release:    0
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-base-1.0)
BuildRequires:  pkgconfig(gstreamer-rtsp-server-1.0)
BuildRequires:  pkgconfig(capi-network-wifi-direct)
BuildRequires:  pkgconfig(mm-wfd)
BuildRequires:  pkgconfig(iniparser)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(libtzplatform-config)


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
export CFLAGS+=" -Wextra -Wno-array-bounds"
export CFLAGS+=" -Wno-ignored-qualifiers -Wno-unused-parameter -Wshadow"
export CFLAGS+=" -Wwrite-strings"
export CFLAGS+=" -DSYSCONFDIR=\\\"%{_sysconfdir}\\\""
#export CFLAGS+="-Wswitch-default"

MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`

%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
%cmake . \
         -DFULLVER=%{version} \
         -DMAJORVER=${MAJORVER} \
         -DTZ_SYS_BIN=%TZ_SYS_BIN \
         -DTZ_SYS_RO_SHARE=%TZ_SYS_RO_SHARE \
         -DTZ_SYS_RO_ETC=%TZ_SYS_RO_ETC

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}%{TZ_SYS_RO_SHARE}/dbus-1/system-services/
install -m 755 org.tizen.scmirroring.server.service %{buildroot}%{TZ_SYS_RO_SHARE}/dbus-1/system-services/
mkdir -p %{buildroot}/%{_datadir}/license
cp -rf %{_builddir}/%{name}-%{version}/LICENSE.APLv2.0 %{buildroot}%{_datadir}/license/%{name}
mkdir -p %{buildroot}/etc/dbus-1/system.d/
cp -rf %{_builddir}/%{name}-%{version}/packaging/org.tizen.scmirroring.server.conf %{buildroot}/etc/dbus-1/system.d/

#License
mkdir -p %{buildroot}/%{_datadir}/license
cp -rf %{_builddir}/%{name}-%{version}/LICENSE.APLv2.0 %{buildroot}/%{_datadir}/license/%{name}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest capi-media-screen-mirroring.manifest
%{_libdir}/libcapi-media-screen-mirroring.so*
%{_libdir}/libmiracast_server_lib.so
%{_datadir}/dbus-1/system-services/org.tizen.scmirroring.server.service
#License
%{_datadir}/license/%{name}
%{_bindir}/miracast_server
/etc/dbus-1/system.d/org.tizen.scmirroring.server.conf

%files devel
%{_includedir}/media/scmirroring_src.h
%{_includedir}/media/scmirroring_sink.h
%{_includedir}/media/scmirroring_type.h
%{_includedir}/media/scmirroring_src_ini.h
%{_includedir}/media/scmirroring_internal.h
%{_includedir}/media/miracast_server.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-screen-mirroring.so
%{_libdir}/libmiracast_server_lib.so
