CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(videos C CXX)

EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE CMAKE_SYSTEM_VERSION)

SET(SRCS
src/video-player.c

src/common/mp-util-media-service.c
src/common/mp-video-info-ctrl.c
src/common/mp-util.c
src/common/mp-drm-ctrl.c
src/common/mp-util-config.c
src/common/mp-util-preference.c
src/common/mp-video-util-db-controller.c
src/common/mp-rotate-ctrl.c
src/common/mp-video-sound-manager.c

src/feature/mp-launching-video-displayer.c
src/feature/mp-util-move.c

src/widget/mp-external-ug.c
src/widget/mp-video-list-option-ctrl.c
src/widget/mp-video-view-popup-ctrl.c
src/widget/mp-video-list-sort-ctrl.c
src/widget/mp-util-widget-ctrl.c
src/widget/mp-video-rename-ctrl.c
src/widget/mp-video-list-view-as-ctrl.c
src/widget/mp-footer-toolbar.c
src/widget/mp-video-nocontent-layout.c

src/view/mp-library-view-mgr.c
src/view/mp-video-list-view-main.c
src/view/mp-video-list-view-common.c
src/view/mp-video-list-view-normal.c
src/view/mp-video-list-view-select.c
src/view/mp-video-list-view-folder.c
src/view/mp-video-list-view-item-of-folder.c
src/view/mp-video-list-view-thumbnail.c
src/view/mp-video-list-share-via-view.c
src/view/mp-video-list-folder-share-via-view.c
src/view/mp-video-list-remove-ctrl.c
src/view/mp-video-search-view.c
src/view/mp-video-detail-view.c
src/view/mp-video-list-remove-view.c
src/view/mp-video-list-personal-view.c
src/view/mp-video-list-personal-ctrl.c
src/viewMgr/videos-view-mgr.c
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/feature
	${CMAKE_SOURCE_DIR}/include/viewMgr
	${CMAKE_SOURCE_DIR}/include/widget
	${CMAKE_SOURCE_DIR}/include/view
	${CMAKE_SOURCE_DIR}/core/include
	${CMAKE_SOURCE_DIR}/videocam/include
	${CMAKE_SOURCE_DIR}/videocam/include

	${CMAKE_SOURCE_DIR}/video-downloader/include

## [ MEDIA SERVICE] ###############################
	${CMAKE_SOURCE_DIR}/common/include
	${CMAKE_SOURCE_DIR}/feature/include
	${CMAKE_SOURCE_DIR}/core/include
## [ Play View] ###############################
	${CMAKE_SOURCE_DIR}/playview/include
)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	elementary
	vconf
	capi-appfw-application
	capi-system-system-settings
	libxml-2.0
	evas
	edje
	efreet-mime
	capi-appfw-preference
	capi-media-image-util
	capi-media-metadata-extractor
	capi-network-wifi-direct
	capi-content-media-content
	capi-web-url-download
	capi-content-mime-type
	capi-system-media-key
	shortcut
	capi-system-sensor
	notification
	capi-system-runtime-info
	capi-network-connection
	appcore-efl
	tapi
	libcore-context-manager
	capi-base-utils-i18n
	efl-extension
	capi-telephony
	capi-appfw-preference
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -DMACHINE_$ENV{MACHINE}")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Werror -fPIC  -fvisibility=hidden")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fdata-sections -ffunction-sections -Wl,--gc-sections")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -Wall -g -fpie")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -g -fpie")
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Xassembler -W -fPIC")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")


#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
#SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC -fvisibility=hidden")
#SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fpie")
#SET(CMAKE_C_FLAGS_RELEASE "-O2 -fpie")


ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFACTORYFS=\"$ENV{FACTORYFS}\"")
ADD_DEFINITIONS("-D_LARGEFILE64_SOURCE")
#ADD_DEFINITIONS("-D_FILE_OFFSET_BITS=\"\"64")

IF(ENABLE_HW_BACK_KEY)
ADD_DEFINITIONS("-D_ENABLE_HW_BACK_KEY")
ENDIF(ENABLE_HW_BACK_KEY)


#MMS for DOCOMO.
IF(CMAKE_DISABLE_FEATURE_MMS)
message("##### Disable MMS for DOCOMO #####")
ADD_DEFINITIONS("-DMP_DISABLE_MMS")
ENDIF(CMAKE_DISABLE_FEATURE_MMS)

IF(CMAKE_ENABLE_MULTI_WINDOW)
#ADD_DEFINITIONS("-D_MULTI_WINDOW")
ENDIF(CMAKE_ENABLE_MULTI_WINDOW)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(DATADIR "/opt/usr/apps/org.tizen.videos/data")
SET(LOCALEDIR "${RESDIR}/locale")
SET(ICONDIR "${RESDIR}/icons/default/small")
SET(EDJDIR "${RESDIR}/edje")
SET(SHAREDRESDIR "${PREFIX}/shared/res")
SET(SLP_LD_PATH_FLAGS -L${CMAKE_CURRENT_SOURCE_DIR}/src/ext)

ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-DRESDIR=\"${RESDIR}\"")
ADD_DEFINITIONS("-DDATADIR=\"${DATADIR}\"")
ADD_DEFINITIONS("-DTHUMBDIR=\"${THUMBDIR}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DICONDIR=\"${ICONDIR}\"")
ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")
ADD_DEFINITIONS("-D_FILE_OFFSET_BITS=64")
ADD_DEFINITIONS("-DUSE_SWITCH")
#ADD_DEFINITIONS("-DUSE_DOWNLOAD_CODEC")
ADD_DEFINITIONS("-DENABLE_LANDSCAPE")
ADD_DEFINITIONS("-DENABLE_PERSONAL")
ADD_DEFINITIONS("-DENABLE_SUBTITLE")
ADD_DEFINITIONS("-DENABLE_NOTICE_POPUP")
ADD_DEFINITIONS("-D_USE_OPENGL_BACKEND")
ADD_DEFINITIONS("-DENABLE_EMBEDED_PROGRESS")
ADD_DEFINITIONS("-DENABLE_S_BEAM")

###	Define Features ###

##	Videos	##

#	Video list view	#
#ADD_DEFINITIONS("-DVS_FEATURE_LIST_VIEW")
ADD_DEFINITIONS("-DVS_FEATURE_THUMBNAIL_VIEW")
ADD_DEFINITIONS("-DVS_FEATURE_FOLDER_VIEW")

#	DRM FEATURE		#
#ADD_DEFINITIONS("-DENABLE_DRM_FEATURE");

IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")
SET(CMAKE_LDFLAGS "-Wl,-zdefs" )

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} ${gles_pkg_LDFLAGS} feature common playview core video-downloader -ldl)

ADD_CUSTOM_TARGET(vp_font.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp_font.edc
${CMAKE_BINARY_DIR}/res/edc/vp_font.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp_font.edc)

ADD_CUSTOM_TARGET(image_res.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/image_res.edc
${CMAKE_BINARY_DIR}/res/edc/image_res.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/image_res.edc)

ADD_CUSTOM_TARGET(panel.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/panel.edc
${CMAKE_BINARY_DIR}/res/edc/panel.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/panel.edc)

ADD_CUSTOM_TARGET(vp-custom-winset-style.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images -sd ${CMAKE_SOURCE_DIR}/res/edc
${CMAKE_SOURCE_DIR}/res/edc/vp-custom-winset-style.edc
${CMAKE_BINARY_DIR}/res/edc/vp-custom-winset-style.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-custom-winset-style.edc)

ADD_CUSTOM_TARGET(popup_progressbar.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/popup_progressbar.edc
${CMAKE_BINARY_DIR}/res/edc/popup_progressbar.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/popup_progressbar.edc)

ADD_CUSTOM_TARGET(landscape_thumbnail.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/landscape_thumbnail.edc
${CMAKE_BINARY_DIR}/res/edc/landscape_thumbnail.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/landscape_thumbnail.edc)

ADD_CUSTOM_TARGET(vp-speedbox-popup.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp-speedbox-popup.edc
${CMAKE_BINARY_DIR}/res/edc/vp-speedbox-popup.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-speedbox-popup.edc)

ADD_CUSTOM_TARGET(vp-remove-list-internal.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp-remove-list-internal.edc
${CMAKE_BINARY_DIR}/res/edc/vp-remove-list-internal.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-remove-list-internal.edc)

ADD_CUSTOM_TARGET(vp-main-listview-internal.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp-main-listview-internal.edc
${CMAKE_BINARY_DIR}/res/edc/vp-main-listview-internal.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-main-listview-internal.edc)

ADD_CUSTOM_TARGET(vp-genlist-no-item.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp-genlist-no-item.edc
${CMAKE_BINARY_DIR}/res/edc/vp-genlist-no-item.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-genlist-no-item.edc)

ADD_CUSTOM_TARGET(vp-device-list-internal.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp-device-list-internal.edc
${CMAKE_BINARY_DIR}/res/edc/vp-device-list-internal.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-device-list-internal.edc)

ADD_CUSTOM_TARGET(vp-main-listview-preview.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images -sd ${CMAKE_SOURCE_DIR}/res/edc
${CMAKE_SOURCE_DIR}/res/edc/vp-main-listview-preview.edc
${CMAKE_BINARY_DIR}/res/edc/vp-main-listview-preview.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-main-listview-preview.edc)

ADD_CUSTOM_TARGET(vp-search-view-internal.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp-search-view-internal.edc
${CMAKE_BINARY_DIR}/res/edc/vp-search-view-internal.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-search-view-internal.edc)

ADD_CUSTOM_TARGET(vp-images.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp-images.edc
${CMAKE_BINARY_DIR}/res/edc/vp-images.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-images.edc)

ADD_CUSTOM_TARGET(vp-motion-popup.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp-motion-popup.edc
${CMAKE_BINARY_DIR}/res/edc/vp-motion-popup.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-motion-popup.edc)

ADD_CUSTOM_TARGET(vp-ctxpopup-option.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp-ctxpopup-option.edc
${CMAKE_BINARY_DIR}/res/edc/vp-ctxpopup-option.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-ctxpopup-option.edc)

ADD_CUSTOM_TARGET(vp-thumbnail-view.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images -sd ${CMAKE_SOURCE_DIR}/res/edc
${CMAKE_SOURCE_DIR}/res/edc/vp-thumbnail-view.edc
${CMAKE_BINARY_DIR}/res/edc/vp-thumbnail-view.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-thumbnail-view.edc)

ADD_CUSTOM_TARGET(vp-main-layout.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp-main-layout.edc
${CMAKE_BINARY_DIR}/res/edc/vp-main-layout.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-main-layout.edc)

ADD_CUSTOM_TARGET(vp-naviframe.edj COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images
${CMAKE_SOURCE_DIR}/res/edc/vp-naviframe.edc
${CMAKE_BINARY_DIR}/res/edc/vp-naviframe.edj DEPENDS
${CMAKE_SOURCE_DIR}/res/edc/vp-naviframe.edc)

ADD_DEPENDENCIES(${PROJECT_NAME} vp_font.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} image_res.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} panel.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-custom-winset-style.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} popup_progressbar.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} landscape_thumbnail.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-speedbox-popup.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-remove-list-internal.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-main-listview-internal.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-genlist-no-item.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-device-list-internal.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-main-listview-preview.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-search-view-internal.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-images.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-motion-popup.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-ctxpopup-option.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-thumbnail-view.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-main-layout.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} vp-naviframe.edj)

######################################
# install desktop file & icon, edje  #
######################################

#INSTALL(DIRECTORY DESTINATION ${DATADIR})
#INSTALL(DIRECTORY DESTINATION ${THUMBDIR})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp_font.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/image_res.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/panel.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-custom-winset-style.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/popup_progressbar.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/landscape_thumbnail.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-speedbox-popup.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-remove-list-internal.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-main-listview-internal.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-genlist-no-item.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-device-list-internal.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-main-listview-preview.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-search-view-internal.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-images.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-motion-popup.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-ctxpopup-option.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-thumbnail-view.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-main-layout.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edc/vp-naviframe.edj DESTINATION ${EDJDIR})

INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.videos.xml DESTINATION /usr/share/packages)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/res/shared/res/video.edc DESTINATION ${SHAREDRESDIR})

INSTALL(FILES ${CMAKE_SOURCE_DIR}/res/images/org.tizen.videos.png DESTINATION /usr/share/icons/default/small/)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/res/images/core_theme_bg_01.png DESTINATION ${RESDIR}/images)

#INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/res/images/ DESTINATION ${RESDIR}/images)
#INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/res/images/icon/ DESTINATION ${RESDIR}/images/icon)
#INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/res/images/toolbar/ DESTINATION ${RESDIR}/images/toolbar)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.videos.efl DESTINATION /etc/smack/accesses.d)

#INSTALL(FILES ${CMAKE_SOURCE_DIR}/res/images/video_fake_image.jpg DESTINATION /usr/apps/org.tizen.videos/res/images/)

#video-downloader
ADD_SUBDIRECTORY(video-downloader)

ADD_SUBDIRECTORY(res/po)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(feature)
ADD_SUBDIRECTORY(playview)

#chapter-svc
#ADD_SUBDIRECTORY(chapter-svc)

#video-player
ADD_SUBDIRECTORY(vp-main)

