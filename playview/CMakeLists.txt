CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(playview C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(VERSION 0.1.0)

SET(SRCS
#################################################
#play view src
#################################################
	src/vp-play-view.c
	src/common/vp-play-util.c
	src/common/vp-play-config.c
	src/common/vp-play-preference.c
	src/common/vp-play-ug.c
	#src/view/vp-play-multi-view.c
	src/view/vp-play-normal-view.c
	src/widget/vp-play-bookmark.c
	src/widget/vp-play-button.c
	src/widget/vp-play-more.c
	src/widget/vp-play-popup.c
	src/widget/vp-play-subtitle.c
	src/widget/vp-play-volume.c
	src/widget/vp-play-progressbar.c
	src/widget/vp-play-loading-ani.c
	src/widget/vp-play-volume-popup.c
	src/widget/vp-play-brightness-popup.c
	src/core/vp-mm-player.c
	src/core/vp-sound.c
	src/core/vp-media-contents.c
	src/core/vp-image-util.c
	src/core/vp-device.c
	src/core/vp-media-key.c
	src/core/vp-sensor.c
	src/core/vp-multi-path.c
	src/core/vp-hollic.c
	src/core/vp-device-language.c
	src/core/vp-avrcp.c
	src/feature/vp-share.c
	src/feature/vp-sound-alive.c
	src/feature/vp-sound-path.c
	src/feature/vp-repeat.c
	src/feature/vp-setting.c
	src/feature/vp-audio-track.c
	src/feature/vp-subtitle.c
	src/feature/vp-subtitle-sync.c
	src/feature/vp-subtitle-select.c
	src/feature/vp-subtitle-track.c
	src/feature/vp-subtitle-font.c
	src/feature/vp-subtitle-size.c
	src/feature/vp-subtitle-color.c
	src/feature/vp-subtitle-alignment.c
	src/feature/vp-subtitle-bg-color.c
	src/feature/vp-subtitle-edge.c
	src/feature/vp-detail.c
	src/feature/vp-capture.c
	src/feature/vp-play-speed.c
	src/feature/vp-zoom-guide.c
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/video-downloader/include
	${CMAKE_SOURCE_DIR}/common/include
	${CMAKE_SOURCE_DIR}/feature/include
	${CMAKE_SOURCE_DIR}/core/include

#################################################
#play view include
#################################################
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/src/include
	${CMAKE_CURRENT_SOURCE_DIR}/src/common/include
	${CMAKE_CURRENT_SOURCE_DIR}/src/view/include
	${CMAKE_CURRENT_SOURCE_DIR}/src/widget/include
	${CMAKE_CURRENT_SOURCE_DIR}/src/core/include
	${CMAKE_CURRENT_SOURCE_DIR}/src/feature/include
)

INCLUDE(FindPkgConfig)
pkg_check_modules(${PROJECT_NAME}_pkgs REQUIRED
	elementary
	#ecore-x
	evas
	edje
	vconf
	capi-appfw-application
	capi-system-system-settings
	glib-2.0
	libxml-2.0
	appcore-efl
	capi-appfw-application
	capi-appfw-preference
	capi-media-player
	capi-media-image-util
	capi-network-wifi-direct
	capi-system-media-key
	capi-content-media-content
	capi-system-sensor
	capi-system-runtime-info
	capi-web-url-download
	shortcut
	notification
	capi-content-mime-type
	capi-network-connection
	fontconfig
	minicontrol-provider
	capi-appfw-app-manager
	storage
	accounts-svc
	tapi
	libcore-context-manager
	capi-system-device
	capi-network-bluetooth
	capi-base-utils-i18n
	capi-network-wifi
	efl-extension
	capi-telephony
)

FOREACH(flag ${${PROJECT_NAME}_pkgs_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS} -Wall")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

IF(ENABLE_HW_BACK_KEY)
ADD_DEFINITIONS("-D_ENABLE_HW_BACK_KEY")
ENDIF(ENABLE_HW_BACK_KEY)

ADD_DEFINITIONS("-DUSE_I_FRAMESEKK")

# K feature
#ADD_DEFINITIONS("-DK_FEATURE_MICROSEEK")
ADD_DEFINITIONS("-DSUBTITLE_K_FEATURE")

# support multi-language of subtitle
ADD_DEFINITIONS("-D_SUBTITLE_MULTI_LANGUAGE")

# apply new feature for trim view
ADD_DEFINITIONS("-D_TRIME_VIEW_NEW_FEATURE")

# apply native buffer sync
#ADD_DEFINITIONS("-D_NATIVE_BUFFER_SYNC")

# apply native buffer sync
#ADD_DEFINITIONS("-DFLICK_JUMP")

ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRCS})

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${VERSION} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${${PROJECT_NAME}_pkgs_LDFLAGS} feature common core video-downloader -ldl)

SET(VIDEO_PLAYER_PACKAGE "org.tizen.video-player")
SET(VIDEO_PLAYER_EDJ_DIR "/usr/apps/${VIDEO_PLAYER_PACKAGE}/res/edje")
SET(VIDEO_SHARED_DATA_DIR "/opt/usr/apps/${VIDEO_PLAYER_PACKAGE}/shared/data")

INSTALL(DIRECTORY DESTINATION ${VIDEO_PLAYER_EDJ_DIR})
INSTALL(DIRECTORY DESTINATION ${VIDEO_SHARED_DATA_DIR})

# EDJE
FUNCTION (BUILD_EDJ _in _out _base _dir)
	ADD_CUSTOM_TARGET(${_out}
		COMMAND
			edje_cc -no-save -id ${_base}/${_dir}
			${CMAKE_CURRENT_SOURCE_DIR}/${_base}/${_in}
			${CMAKE_BINARY_DIR}/${_out}
		DEPENDS
			${CMAKE_CURRENT_SOURCE_DIR}/${_base}/${_in}
	)
	ADD_DEPENDENCIES(${PROJECT_NAME} ${_out})
	INSTALL(FILES ${CMAKE_BINARY_DIR}/${_out} DESTINATION ${VIDEO_PLAYER_EDJ_DIR})
ENDFUNCTION (BUILD_EDJ)

FUNCTION (BUILD_CUSTOM_EDJ _in _out _base _id _sd)
	ADD_CUSTOM_TARGET(${_out}
		COMMAND
			edje_cc -no-save -id ${_base}/${_id} -sd ${_base}/${_sd}
			${CMAKE_CURRENT_SOURCE_DIR}/${_base}/${_in}
			${CMAKE_BINARY_DIR}/${_out}
		DEPENDS
			${CMAKE_CURRENT_SOURCE_DIR}/${_base}/${_in}
	)
	ADD_DEPENDENCIES(${PROJECT_NAME} ${_out})
	INSTALL(FILES ${CMAKE_BINARY_DIR}/${_out} DESTINATION ${VIDEO_PLAYER_EDJ_DIR})
ENDFUNCTION (BUILD_CUSTOM_EDJ)

BUILD_EDJ(pv-normal-main.edc			pv-normal-main.edj			res/edc/playview	images)
BUILD_EDJ(pv-normal-control.edc			pv-normal-control.edj		res/edc/playview	images)
BUILD_EDJ(pv-normal-top-control.edc		pv-normal-top-control.edj	res/edc/playview	images)
BUILD_EDJ(pv-normal-function.edc		pv-normal-function.edj		res/edc/playview	images)
BUILD_EDJ(pv-multi-main.edc				pv-multi-main.edj			res/edc/playview	images)
BUILD_EDJ(pv-trim-main.edc				pv-trim-main.edj			res/edc/playview	images)
BUILD_EDJ(pv-trim-control.edc			pv-trim-control.edj			res/edc/playview	images)
BUILD_EDJ(pv-trim-top-control.edc		pv-trim-top-control.edj		res/edc/playview	images)
BUILD_EDJ(pv-trim-item.edc				pv-trim-item.edj			res/edc/playview	images)
BUILD_EDJ(pv-chapter-main-new.edc		pv-chapter-main-new.edj		res/edc/playview	images)
BUILD_EDJ(pv-progressbar.edc			pv-progressbar.edj			res/edc/playview	images)
BUILD_EDJ(pv-volume.edc					pv-volume.edj				res/edc/playview	images)
BUILD_EDJ(pv-volume-popup.edc			pv-volume-popup.edj			res/edc/playview	images)
BUILD_EDJ(pv-brightness-popup.edc		pv-brightness-popup.edj		res/edc/playview	images)
BUILD_EDJ(pv-bookmark.edc				pv-bookmark.edj				res/edc/playview	images)
BUILD_EDJ(pv-bookmark-item.edc			pv-bookmark-item.edj		res/edc/playview	images)
BUILD_EDJ(pv-resource.edc				pv-resource.edj				res/edc/playview	images)
BUILD_EDJ(pv-subtitle.edc				pv-subtitle.edj				res/edc/playview	images)
BUILD_EDJ(pv-popup.edc					pv-popup.edj				res/edc/playview	images)
BUILD_EDJ(pv-sensor-popup.edc			pv-sensor-popup.edj			res/edc/playview	images)
BUILD_EDJ(pv-speed-popup.edc			pv-speed-popup.edj			res/edc/playview	images)
BUILD_EDJ(pv-zoom-guide.edc				pv-zoom-guide.edj			res/edc/playview	images)
BUILD_EDJ(pv-gesture.edc				pv-gesture.edj				res/edc/playview	images)
BUILD_EDJ(pv-subtitle-color-popup.edc   	pv-subtitle-color-popup.edj            res/edc/playview        images)
BUILD_EDJ(pv-colorselector.edc           pv-colorselector.edj            res/edc/playview        images)

BUILD_CUSTOM_EDJ(pv-minicontroller.edc	pv-minicontroller.edj		res/edc/playview	images	custom)
BUILD_CUSTOM_EDJ(pv-custom-winset.edc	pv-custom-winset.edj		res/edc/playview	images	custom)


#install resouce files.
# Lack of a trailing slash means install this directory, trailing slash means
# install everything in this directory.

# Install image files.

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION lib)



