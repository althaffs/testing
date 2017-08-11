Summary:            A SDK library for easing up development on Samsung's ARTIK IoT platform  
Name:               libartik-sdk
Version:            1.4
Release:            0%{?dist}
License:            Proprietary
Group:              Development/Libraries
URL:                http://www.artik.io

%define _binaries_in_noarch_packages_terminate_build   0

Requires: %{name}-base = %{version}-%{release}
Requires: %{name}-systemio = %{version}-%{release}
Requires: %{name}-connectivity = %{version}-%{release}
Requires: %{name}-media = %{version}-%{release}
Requires: %{name}-sensor = %{version}-%{release}
Requires: %{name}-bluetooth = %{version}-%{release}
Requires: %{name}-wifi = %{version}-%{release}
Requires: %{name}-zigbee = %{version}-%{release}
Requires: %{name}-lwm2m = %{version}-%{release}
Requires: %{name}-mqtt = %{version}-%{release}

%description
This library contains a SDK for addressing common features and quickly building
IoT platforms based on Samsung's ARTIK modules.

%package devel
Requires: %{name}-base-devel = %{version}-%{release}
Requires: %{name}-systemio-devel = %{version}-%{release}
Requires: %{name}-connectivity-devel = %{version}-%{release}
Requires: %{name}-media-devel = %{version}-%{release}
Requires: %{name}-sensor-devel = %{version}-%{release}
Requires: %{name}-bluetooth-devel = %{version}-%{release}
Requires: %{name}-wifi-devel = %{version}-%{release}
Requires: %{name}-zigbee-devel = %{version}-%{release}
Requires: %{name}-lwm2m-devel = %{version}-%{release}
Requires: %{name}-mqtt-devel = %{version}-%{release}
Summary: Files needed for building applications against the ARTIK SDK libraries

%description devel
This package contains development files for building programs against the ATIK SDK libraries

%package base
Group: Development/Libraries
Requires: glib2
Provides: %{name}-base.so.%{version}
Provides: %{name}-base.so.1
Summary: Base package needed by all the other packages of the SDK

%description base
This library contains the base library of the SDK for addressing common features

%package base-devel
Requires: %{name}-base = %{version}-%{release}
Summary: Files needed for building applications against the ARTIK SDK Base library

%description base-devel
This package contains development files for building programs against the ARTIK SDK Base library

%package systemio
Group: Development/Libraries
Requires: %{name}-base = %{version}-%{release}
Provides: %{name}-systemio.so.%{version}
Provides: %{name}-systemio.so.1
Summary: System IO package containing APIs for communicating over hardware busses (GPIO, SPI, I2C, etc...)

%description systemio
System IO package containing APIs for communicating over hardware busses (GPIO, SPI, I2C, etc...)

%package systemio-devel
Requires: %{name}-systemio = %{version}-%{release}
Summary: Files needed for building applications against the ARTIK SDK System IO library

%description systemio-devel
This package contains development files for building programs against the ARTIK SDK System IO library

%package connectivity
Group: Development/Libraries
Requires: %{name}-base = %{version}-%{release}
Requires: libwebsockets >= 1.7
Requires: libcurl
Requires: openssl-libs
Requires: artiksee = 0.5
Provides: %{name}-connectivity.so.%{version}
Provides: %{name}-connectivity.so.1
Summary: Connectivity package containing APIs for communicating over HTTP, websockets, etc...

%description connectivity
Connectivity package containing APIs for communicating over HTTP, websockets, etc...

%package connectivity-devel
Requires: %{name}-connectivity = %{version}-%{release}
Summary: Files needed for building applications against the ARTIK SDK Connectivity library

%description connectivity-devel
This package contains development files for building programs against the ARTIK SDK Connectivity library

%package media
Group: Development/Libraries
Requires: %{name}-base = %{version}-%{release}
Requires: gstreamer1
Requires: gstreamer1-plugins-base
Requires: gstreamer1-plugins-good
Provides: %{name}-media.so.%{version}
Provides: %{name}-media.so.1
Summary: Media package containing APIs for handling audio/video streams

%description media
Media package containing APIs for handling audio/video streams

%package media-devel
Requires: %{name}-media = %{version}-%{release}
Summary: Files needed for building applications against the ARTIK SDK Media library

%description media-devel
This package contains development files for building programs against the ARTIK SDK Media library

%package sensor
Group: Development/Libraries
Requires: %{name}-base = %{version}-%{release}
Provides: %{name}-sensor.so.%{version}
Provides: %{name}-sensor.so.1
Summary: Sensor package containing APIs for handling data from various sensors

%description sensor
Sensor package containing APIs for handling data from various sensors

%package sensor-devel
Requires: %{name}-sensor = %{version}-%{release}
Summary: Files needed for building applications against the ARTIK SDK Sensor library

%description sensor-devel
This package contains development files for building programs against the ARTIK SDK Sensor library

%package bluetooth
Group: Development/Libraries
Requires: %{name}-base = %{version}-%{release}
Requires: bluez >= 5.39
Provides: %{name}-bluetooth.so.%{version}
Provides: %{name}-bluetooth.so.1
Summary: Package containing APIs for communicating with other devices over BT/BLE

%description bluetooth
Package containing APIs for communicating with other devices over BT/BLE

%package bluetooth-devel
Requires: %{name}-bluetooth = %{version}-%{release}
Summary: Files needed for building applications against the ARTIK SDK Bluetooth library

%description bluetooth-devel
This package contains development files for building programs against the ARTIK SDK Bluetooth library

%package wifi
Group: Development/Libraries
Requires: %{name}-base = %{version}-%{release}
Requires: wpa_supplicant >= 2.5
Provides: %{name}-wifi.so.%{version}
Provides: %{name}-wifi.so.1
Summary: Package containing APIs for scanning and connecting to Wifi networks

%description wifi
Package containing APIs for scanning and connecting to Wifi networks

%package wifi-devel
Requires: %{name}-wifi = %{version}-%{release}
Summary: Files needed for building applications against the ARTIK SDK Wifi library

%description wifi-devel
This package contains development files for building programs against the ARTIK SDK Wifi library

%package zigbee
Group: Development/Libraries
Requires: %{name}-base = %{version}-%{release}
Requires: libzigbee >= 0.7
Provides: %{name}-zigbee.so.%{version}
Provides: %{name}-zigbee.so.1
Summary: Package containing APIs for communicating with other devices over Zigbee

%description zigbee
Package containing APIs for communicating with other devices over Zigbee

%package zigbee-devel
Requires: %{name}-zigbee = %{version}-%{release}
Summary: Files needed for building applications against the ARTIK SDK Zigbee library

%description zigbee-devel
This package contains development files for building programs against the ARTIK SDK Zigbee library

%package lwm2m
Group: Development/Libraries
Requires: %{name}-base = %{version}-%{release}
Requires: wakaama-client = 1.2
Provides: %{name}-lwm2m.so.%{version}
Provides: %{name}-lwm2m.so.1
Summary: Package containing APIs for communicating with other devices over LWM2M

%description lwm2m
Package containing APIs for communicating with other devices over LWM2M

%package lwm2m-devel
Requires: %{name}-lwm2m = %{version}-%{release}
Summary: Files needed for building applications against the ARTIK SDK LWM2M library

%description lwm2m-devel
This package contains development files for building programs against the ARTIK SDK LWM2M library

%package mqtt
Group: Development/Libraries
Requires: %{name}-base = %{version}-%{release}
Requires: mosquitto >= 1.4.10
Provides: %{name}-mqtt.so.%{version}
Provides: %{name}-mqtt.so.1
Summary: Package containing APIs for communicating with other devices over MQTT

%description mqtt
Package containing APIs for communicating with other devices over MQTT

%package mqtt-devel
Requires: %{name}-mqtt = %{version}-%{release}
Summary: Files needed for building applications against the ARTIK SDK MQTT library

%description mqtt-devel
This package contains development files for building programs against the ARTIK SDK MQTT library

%package tests
Group: Development/Libraries
Requires: %{name}-base = %{version}-%{release}
Requires: %{name}-systemio = %{version}-%{release}
Requires: %{name}-connectivity = %{version}-%{release}
Requires: %{name}-media = %{version}-%{release}
Requires: %{name}-bluetooth = %{version}-%{release}
Requires: %{name}-sensor = %{version}-%{release}
Requires: %{name}-wifi = %{version}-%{release}
Requires: %{name}-zigbee = %{version}-%{release}
Requires: %{name}-lwm2m = %{version}-%{release}
Requires: %{name}-mqtt = %{version}-%{release}
Requires: CUnit
Summary: Test programs to validate the ARTIK SDK.

%description tests
This package contains unit tests for the functions exposed by the ARTIK SDK.

%prep
rm -rf %{_builddir}/*
rm -rf %{buildroot}/*

%build
cd %{_builddir}

echo %{_host_cpu}
%if %(echo %arm | egrep -c %{_host_cpu})
cmake %{_srcdir} -DCMAKE_BUILD_TEST=1
%else
cmake %{_srcdir} -DCMAKE_TOOLCHAIN_FILE=%{_srcdir}/target/toolchain-cross-arm.cmake -DCMAKE_SYSROOT=%{_sysrootdir} -DCMAKE_BUILD_TEST=1
%endif

make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}

%post
/sbin/ldconfig

%files
%defattr(-,root,root)

%files devel
%defattr(-,root,root)

%files base
%defattr(-,root,root)
%{_libdir}/%{name}-base.so.*

%files base-devel
%defattr(-,root,root)
%{_libdir}/%{name}-base.so
%{_includedir}/artik/base/*
%{_libdir}/pkgconfig/%{name}-base.pc

%files systemio
%defattr(-,root,root)
%{_libdir}/%{name}-systemio.so.*

%files systemio-devel
%defattr(-,root,root)
%{_libdir}/%{name}-systemio.so
%{_includedir}/artik/systemio/*
%{_libdir}/pkgconfig/%{name}-systemio.pc

%files connectivity
%defattr(-,root,root)
%{_libdir}/%{name}-connectivity.so.*

%files connectivity-devel
%defattr(-,root,root)
%{_libdir}/%{name}-connectivity.so
%{_includedir}/artik/connectivity/*
%{_libdir}/pkgconfig/%{name}-connectivity.pc

%files media
%defattr(-,root,root)
%{_libdir}/%{name}-media.so.*

%files media-devel
%defattr(-,root,root)
%{_libdir}/%{name}-media.so
%{_includedir}/artik/media/*
%{_libdir}/pkgconfig/%{name}-media.pc

%files sensor
%defattr(-,root,root)
%{_libdir}/%{name}-sensor.so.*

%files sensor-devel
%defattr(-,root,root)
%{_libdir}/%{name}-sensor.so
%{_includedir}/artik/sensor/*
%{_libdir}/pkgconfig/%{name}-sensor.pc

%files bluetooth
%defattr(-,root,root)
%{_libdir}/%{name}-bluetooth.so.*

%files bluetooth-devel
%defattr(-,root,root)
%{_libdir}/%{name}-bluetooth.so
%{_includedir}/artik/bluetooth/*
%{_libdir}/pkgconfig/%{name}-bluetooth.pc

%files wifi
%defattr(-,root,root)
%{_libdir}/%{name}-wifi.so.*

%files wifi-devel
%defattr(-,root,root)
%{_libdir}/%{name}-wifi.so
%{_includedir}/artik/wifi/*
%{_libdir}/pkgconfig/%{name}-wifi.pc

%files zigbee
%defattr(-,root,root)
%{_libdir}/%{name}-zigbee.so.*

%files zigbee-devel
%defattr(-,root,root)
%{_libdir}/%{name}-zigbee.so
%{_includedir}/artik/zigbee/*
%{_libdir}/pkgconfig/%{name}-zigbee.pc

%files lwm2m
%defattr(-,root,root)
%{_libdir}/%{name}-lwm2m.so.*

%files lwm2m-devel
%defattr(-,root,root)
%{_libdir}/%{name}-lwm2m.so
%{_includedir}/artik/lwm2m/*
%{_libdir}/pkgconfig/%{name}-lwm2m.pc

%files mqtt
%defattr(-,root,root)
%{_libdir}/%{name}-mqtt.so.*

%files mqtt-devel
%defattr(-,root,root)
%{_libdir}/%{name}-mqtt.so
%{_includedir}/artik/mqtt/*
%{_libdir}/pkgconfig/%{name}-mqtt.pc

%files tests
%defattr(-,root,root)
%{_libdir}/artik-sdk/tests/*

%changelog
* Tue Oct 25 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 1.0-2
- various bug fixing
* Tue Oct 11 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 1.0-1
- loop: lower the priority of the idle function callbacks that could
prevent the glib from running normaly under certain circumstances
- media: fix playsound API to use the glib main loop
- websocket: fix wrong handling of callbacks in the node.js addon
* Fri Sep 23 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 1.1-0
- add support for MQTT client
- change LWM2M client API to match the new wakaama-client library
* Wed Aug 31 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 1.0-0
- bug fixing
* Fri Aug 12 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 0.13-1
- bug fixing
* Fri Aug 5 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 0.12-1
- bug fixing
- add LWM2M client support
* Fri Jul 8 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 0.11-1
- bug fixing
- add support for Artik 710 platform
* Wed Jun 22 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 0.10-1
- bug fixing after code rearchitecture
* Mon Jun 13 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 0.9-1
- add zigbee module
- change synchronous API calls to asynchronous using glib's loop
- reorganize the sources to generate several SDK libraries instead of one
* Fri Apr 22 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 0.4-1
- add Time module
- add platform auto-detection
- add media module and API for playing sound files
* Fri Apr 15 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 0.3-1
- add support for Artik 10 development platform
- add Wifi manager
- add PUT/DELETE requests in the HTTP module
* Fri Apr 8 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 0.2-1
- add ADC module
* Thu Mar 31 2016 Gregory Lemercier <g.lemercier@ssi.samsung.com> 0.1-1
- first implementation of the API, containing access for GPIO, I2C, and UART

