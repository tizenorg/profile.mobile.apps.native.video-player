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
BuildRequires: pkgconfig(vconf)
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

BuildRequires: cmake
BuildRequires: gettext-tools
BuildRequires: edje-tools
BuildRequires: prelink
BuildRequires: libicu-devel
#BuildRequires: hash-signer

BuildRequires: pkgconfig(tapi)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(capi-telephony)

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

%if  0%{?sec_product_feature_cloud_enable_content_sync_dropbox}
cmake . -DCMAKE_INSTALL_PREFIX=/usr/apps/org.tizen.videos  -DENABLE_CLOUD_FEATURE=YES -DARCH=%{ARCH}
%else
cmake . -DCMAKE_INSTALL_PREFIX=/usr/apps/org.tizen.videos  -DARCH=%{ARCH}
%endif

%if 0%{?sec_product_feature_msg_disable_mms}
cmake . -DCMAKE_INSTALL_PREFIX=/usr/apps/org.tizen.videos  -DCMAKE_DISABLE_FEATURE_MMS=YES -DARCH=%{ARCH}
%else
cmake . -DCMAKE_INSTALL_PREFIX=/usr/apps/org.tizen.videos  -DARCH=%{ARCH}
%endif

%if 0%{?sec_product_feature_multiwindow}
cmake . -DCMAKE_INSTALL_PREFIX=/usr/apps/org.tizen.videos  -DCMAKE_ENABLE_MULTI_WINDOW=YES -DARCH=%{ARCH}
%else
cmake . -DCMAKE_INSTALL_PREFIX=/usr/apps/org.tizen.videos  -DARCH=%{ARCH}
%endif

%if 0%{?sec_product_feature_hw_enable_back_menu_key}
cmake . -DCMAKE_INSTALL_PREFIX=/usr/apps/org.tizen.videos  -DENABLE_HW_BACK_KEY=YES -DARCH=%{ARCH}
%else
cmake . -DCMAKE_INSTALL_PREFIX=/usr/apps/org.tizen.videos  -DARCH=%{ARCH}
%endif

%if 0%{?sec_product_feature_nfc_disable}
cmake . -DCMAKE_INSTALL_PREFIX=/usr/apps/org.tizen.videos  -DENABLE_NFC_FEATURE=YES -DARCH=%{ARCH}
%else
cmake . -DCMAKE_INSTALL_PREFIX=/usr/apps/org.tizen.videos  -DARCH=%{ARCH}
%endif

%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

if [ ! -d %{buildroot}/opt/usr/apps/org.tizen.videos/data/ ]
then
        mkdir -p %{buildroot}/opt/usr/apps/org.tizen.videos/data/
fi

if [ ! -f %{buildroot}/opt/usr/apps/org.tizen.videos/data/.video-thumb.db ];
	rm -rf %{buildroot}/opt/usr/apps/org.tizen.videos/data/.video-thumb.db*
then
	echo -e "install video thumb DB in install"
	sqlite3 %{buildroot}/opt/usr/apps/org.tizen.videos/data/.video-thumb.db 'PRAGMA journal_mode=PERSIST;
	CREATE TABLE video_preview(preview_uuid INTEGER PRIMARY KEY AUTOINCREMENT, file_path TEXT, file_size INTEGER,  last_hit_date INTEGER, media_id TEXT);
	CREATE TABLE video_thumb(thumb_uuid INTEGER PRIMARY KEY AUTOINCREMENT, thumb_idx INTEGER, file_path TEXT, file_size INTEGER,  media_id TEXT);
	CREATE TABLE video_chapter(chapter_uuid INTEGER PRIMARY KEY AUTOINCREMENT, chapter_idx INTEGER, file_path TEXT, file_size INTEGER,  media_id TEXT);'
fi

%make_install
execstack -c %{buildroot}/usr/apps/org.tizen.videos/bin/videos
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}
%define tizen_sign 1
%define tizen_sign_base /usr/apps/org.tizen.videos;/usr/apps/org.tizen.video-player;
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1

%post
#/usr/bin/signing-client/hash-signer-client.sh -a -d -p platform /usr/apps/org.tizen.videos

%postun

%post -n org.tizen.video-player
/sbin/ldconfig

if [ ! -d %{buildroot}/opt/usr/apps/org.tizen.videos/ ]
then
        mkdir -p %{buildroot}/opt/usr/apps/org.tizen.videos/
fi

if [ ! -d %{buildroot}/opt/usr/apps/org.tizen.videos/data/ ]
then
        mkdir -p %{buildroot}/opt/usr/apps/org.tizen.videos/data/
fi

if [ ! -d %{buildroot}/opt/usr/media/.video_thumb ]
then
        mkdir -p %{buildroot}/opt/usr/media/.video_thumb
fi

if [ ! -d %{buildroot}/opt/usr/media/.cur_video_thumb ]
then
        mkdir -p %{buildroot}/opt/usr/media/.cur_video_thumb
fi

if [ ! -d %{buildroot}/opt/usr/media/.video_thumb/tmp ]
then
        mkdir -p %{buildroot}/opt/usr/media/.video_thumb/tmp
fi

if [ ! -d %{buildroot}/opt/usr/media/.video_thumb/sdp ]
then
        mkdir -p %{buildroot}/opt/usr/media/.video_thumb/sdp
fi

if [ ! -f %{buildroot}/opt/usr/apps/org.tizen.videos/data/.video-thumb.db ];
	rm -rf %{buildroot}/opt/usr/apps/org.tizen.videos/data/.video-thumb.db*
then
	echo -e "install video thumb DB in postfile"
	sqlite3 %{buildroot}/opt/usr/apps/org.tizen.videos/data/.video-thumb.db 'PRAGMA journal_mode=PERSIST;
	CREATE TABLE video_preview(preview_uuid INTEGER PRIMARY KEY AUTOINCREMENT, file_path TEXT, file_size INTEGER,  last_hit_date INTEGER, media_id TEXT);
	CREATE TABLE video_thumb(thumb_uuid INTEGER PRIMARY KEY AUTOINCREMENT, thumb_idx INTEGER, file_path TEXT, file_size INTEGER,  media_id TEXT);
	CREATE TABLE video_chapter(chapter_uuid INTEGER PRIMARY KEY AUTOINCREMENT, chapter_idx INTEGER, file_path TEXT, file_size INTEGER,  media_id TEXT);'
fi

chmod 660 %{buildroot}/opt/usr/apps/org.tizen.videos/data/.video-thumb.db
chmod 660 %{buildroot}/opt/usr/apps/org.tizen.videos/data/.video-thumb.db-journal

# Change file owner
chown -R app:app /opt/usr/apps/org.tizen.videos/data
chown -R app:app /opt/usr/apps/org.tizen.video-player/data
chown -R 5000:5000 /opt/usr/media/.video_thumb
chown -R 5000:5000 /opt/usr/media/.cur_video_thumb
chown -R 5000:5000 /opt/usr/media/.video_thumb/tmp
chown -R 5000:5000 /opt/usr/media/.video_thumb/sdp

chown -R 5000:5000 /opt/usr/apps/org.tizen.videos/data/.video-thumb.db
chown -R 5000:5000 /opt/usr/apps/org.tizen.videos/data/.video-thumb.db-journal

chmod 777 /opt/usr/apps/org.tizen.videos/data
chmod 777 /opt/usr/apps/org.tizen.video-player/shared/data

#/usr/bin/signing-client/hash-signer-client.sh -a -d -p platform /usr/apps/org.tizen.video-player

%files -n org.tizen.videos
%manifest org.tizen.videos.manifest

/usr/apps/org.tizen.videos/bin/*
/usr/apps/org.tizen.videos/res/locale/*
/usr/share/icons/default/small/org.tizen.videos.png
/usr/apps/org.tizen.videos/res/edje/*
#/usr/apps/org.tizen.videos/res/images/*
/usr/share/packages/org.tizen.videos.xml
/usr/apps/org.tizen.videos/shared/res/video.edc
/usr/apps/org.tizen.videos/res/images/core_theme_bg_01.png

#/usr/apps/org.tizen.videos/res/script/*
#/usr/apps/org.tizen.videos/libexec/*

/etc/smack/accesses.d/org.tizen.videos.efl

#/usr/apps/org.tizen.videos/author-signature.xml
#/usr/apps/org.tizen.videos/signature1.xml

%files -n org.tizen.video-player
/etc/smack/accesses.d/org.tizen.video-player.efl
%manifest org.tizen.video-player.manifest
%defattr(-,root,root,-)
/opt/usr/media/.video_thumb
/opt/usr/media/.cur_video_thumb
/opt/usr/media/.video_thumb/tmp
/opt/usr/media/.video_thumb/sdp
/opt/usr/apps/org.tizen.videos/data
/usr/apps/org.tizen.video-player/bin/*
/usr/apps/org.tizen.video-player/res/locale/*
/usr/apps/org.tizen.video-player/res/edje/*
/usr/share/icons/default/small/org.tizen.video-player.png
/usr/apps/org.tizen.video-player/shared/res/video_preview.png
/usr/apps/org.tizen.video-player/shared/res/video_preview_l.png
/opt/usr/apps/org.tizen.video-player/data
/opt/usr/apps/org.tizen.video-player/shared/data
/usr/share/packages/org.tizen.video-player.xml
#/usr/apps/org.tizen.video-player/author-signature.xml
#/usr/apps/org.tizen.video-player/signature1.xml

/usr/share/license/%{name}
