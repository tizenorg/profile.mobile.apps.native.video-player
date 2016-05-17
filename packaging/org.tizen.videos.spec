Name:       org.tizen.videos
Summary:    video library & store
Version:    1.0.40
Release:    1
Group:      Applications
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable" || "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(libmedia-service)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(efreet)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-appfw-preference)
BuildRequires: pkgconfig(capi-media-player)
BuildRequires: pkgconfig(capi-media-image-util)
BuildRequires: pkgconfig(capi-media-metadata-extractor)
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(capi-content-mime-type)
BuildRequires: pkgconfig(capi-system-media-key)
BuildRequires: pkgconfig(capi-web-url-download)
BuildRequires: pkgconfig(capi-system-sensor)
BuildRequires: pkgconfig(shortcut)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(accounts-svc)
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(capi-system-runtime-info)
BuildRequires: pkgconfig(capi-network-wifi)
BuildRequires: pkgconfig(capi-network-wifi-direct)
BuildRequires: boost-devel
BuildRequires: boost-thread
BuildRequires: boost-system
BuildRequires: boost-filesystem
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(minicontrol-provider)
BuildRequires: pkgconfig(capi-appfw-app-manager)
BuildRequires: pkgconfig(storage)
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(libcore-context-manager)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires: pkgconfig(capi-network-bluetooth)
BuildRequires: pkgconfig(capi-base-utils-i18n)
BuildRequires: pkgconfig(capi-ui-efl-util)

BuildRequires: cmake
BuildRequires: gettext-tools
BuildRequires: edje-tools
BuildRequires: prelink
BuildRequires: libicu-devel
#BuildRequires: hash-signer

BuildRequires: pkgconfig(tapi)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(capi-telephony)
BuildRequires: pkgconfig(libtzplatform-config)

%description
video library


Description: chapter video manange app for video-player.

%package -n org.tizen.video-player
Summary:    video file video-player.
Group:      Applications
License:    Apache-2.0

%description -n org.tizen.video-player

Description: video file video-player.

%prep
%setup -q

%ifarch %{arm}
%define ARCH arm
%else
%define ARCH i586
%endif

%build
%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
%endif

CFLAGS+=" -fvisibility=hidden"; export CFLAGS
CXXFLAGS+=" -fvisibility=hidden"; export CXXFLAGS
FFLAGS+=" -fvisibility=hidden"; export FFLAGS

cmake . -DCMAKE_INSTALL_PREFIX="%{TZ_SYS_RO_APP}/org.tizen.videos" \
	-DCMAKE_INSTALL_VP_PREFIX="%{TZ_SYS_RO_APP}/org.tizen.video-player" \
	-DCMAKE_DESKTOP_ICON_DIR="%{TZ_SYS_RO_ICONS}/default/small" \
	-DCMAKE_APP_SHARE_PACKAGES_DIR="%{TZ_SYS_RO_PACKAGES}" \
	-DARCH=%{ARCH} \
	-DTZ_SYS_RO_APP=%{TZ_SYS_RO_APP}

%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

%make_install
#execstack -c %{buildroot}%{TZ_SYS_RO_APP}/org.tizen.videos/bin/videos
mkdir -p %{buildroot}%{TZ_SYS_SHARE}/license
cp LICENSE %{buildroot}%{TZ_SYS_SHARE}/license/%{name}
%define tizen_sign 1
%define tizen_sign_base %{TZ_SYS_RO_APP}/org.tizen.videos;
%define tizen_sign_base %{TZ_SYS_RO_APP}/org.tizen.video-player;
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1

%post
#/usr/bin/signing-client/hash-signer-client.sh -a -d -p platform %{TZ_SYS_RO_APP}/org.tizen.videos

%postun

%post -n org.tizen.video-player
/sbin/ldconfig

#/usr/bin/signing-client/hash-signer-client.sh -a -d -p platform /usr/apps/org.tizen.video-player

%files -n org.tizen.videos
%manifest org.tizen.videos.manifest
%defattr(-,root,root,-)
%{TZ_SYS_RO_APP}/org.tizen.videos/bin/*
%{TZ_SYS_RO_APP}/org.tizen.videos/res/locale/*
%{TZ_SYS_RW_ICONS}/default/small/org.tizen.videos.png
%{TZ_SYS_RO_APP}/org.tizen.videos/res/edje/*
%{TZ_SYS_RO_PACKAGES}/org.tizen.videos.xml
%{TZ_SYS_RO_APP}/org.tizen.videos/shared/res/video.edc
%{TZ_SYS_RO_APP}/org.tizen.videos/res/images/core_theme_bg_01.png

#%{TZ_SYS_RO_APP}/org.tizen.videos/res/script/*
#%{TZ_SYS_RO_APP}/org.tizen.videos/libexec/*

#%{TZ_SYS_RO_APP}/org.tizen.videos/author-signature.xml
#%{TZ_SYS_RO_APP}/org.tizen.videos/signature1.xml

%files -n org.tizen.video-player
%manifest org.tizen.video-player.manifest
%defattr(-,root,root,-)
%{TZ_SYS_RO_APP}/org.tizen.video-player/bin/*
%{TZ_SYS_RO_APP}/org.tizen.video-player/res/locale/*
%{TZ_SYS_RO_APP}/org.tizen.video-player/res/edje/*
%{TZ_SYS_RW_ICONS}/default/small/org.tizen.video-player.png
%{TZ_SYS_RO_PACKAGES}/org.tizen.video-player.xml
#%{TZ_SYS_RO_APP}/org.tizen.video-player/author-signature.xml
#%{TZ_SYS_RO_APP}/usr/apps/org.tizen.video-player/signature1.xml

%{TZ_SYS_SHARE}/license/%{name}