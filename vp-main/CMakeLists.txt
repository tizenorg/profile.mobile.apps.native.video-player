CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(video-player C CXX)


SET(SRCS
	src/vp-main.c
	src/serviceParser/vp-service-parser.c
)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include/serviceParser
)

INCLUDE(FindPkgConfig)
pkg_check_modules(${PROJECT_NAME}_pkgs REQUIRED
	elementary
	ecore
	ecore-evas
	edje
	eina
	evas
	dlog
	ecore-input
	glib-2.0
	capi-appfw-application
	capi-content-media-content
	capi-web-url-download
	libcore-context-manager
)

#SET(VIDEO_PLAYER_PACKAGE "com.${VIDEO_PLAYER_VENDOR}.video-player")
SET(APPDIR ${CMAKE_INSTALL_APPDIR})
SET(SHAREDIR ${CMAKE_INSTALL_SHAREDIR})

SET(VIDEO_PLAYER_PACKAGE "org.tizen.video-player")
SET(VIDEO_PLAYER_DIR "${APPDIR}/${VIDEO_PLAYER_PACKAGE}")
SET(VIDEO_PLAYER_RES_DIR "${APPDIR}/${VIDEO_PLAYER_PACKAGE}/res")
SET(VIDEO_PLAYER_BINDIR "${APPDIR}/${VIDEO_PLAYER_PACKAGE}/bin")
SET(VP_APPDIR "${APPDIR}")
SET(DESKTOPDIR "${SHAREDIR}")
SET(ICONDIR ${CMAKE_INSTALL_ICON_DIR})
SET(VIDEO_PLAYER_DATADIR "${APPDIR}/${VIDEO_PLAYER_PACKAGE}/data")
SET(VP_RESDIR "${APPDIR}/${VIDEO_PLAYER_PACKAGE}/res")
SET(VP_LOCALEDIR "${VP_RESDIR}/locale")
SET(VP_DATADIR "${APPDIR}/org.tizen.videos/data")
SET(VP_THUMBDIR "/opt/usr/media/.video_thumb")
SET(VP_CUR_THUMBDIR "/opt/usr/media/.cur_video_thumb")
SET(VP_TEMPDIR "/opt/usr/media/.video_thumb/tmp")
SET(VP_SDPDIR "/opt/usr/media/.video_thumb/sdp")

ADD_DEFINITIONS("-DVP_LOCALEDIR=\"${VP_LOCALEDIR}\"")

FOREACH(flag ${${PROJECT_NAME}_pkgs_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC -fvisibility=hidden")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -fPIC")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie -Wl,--rpath=/usr/lib")
SET(CMAKE_LDFLAGS "-Wl,-zdefs" )

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${${PROJECT_NAME}_pkgs_LDFLAGS} feature common playview video-downloader -ldl)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${VIDEO_PLAYER_BINDIR})

# install desktop file & icon
INSTALL(FILES ${CMAKE_SOURCE_DIR}/res/images/org.tizen.video-player.png DESTINATION ${ICONDIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/res/images/video_preview.png DESTINATION ${VP_APPDIR}/org.tizen.video-player/shared/res/)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/res/images/video_preview_l.png DESTINATION ${VP_APPDIR}/org.tizen.video-player/shared/res/)
INSTALL(FILES ${VIDEO_PLAYER_PACKAGE}.xml DESTINATION ${DESKTOPDIR})

# install application HOME directory
INSTALL(DIRECTORY DESTINATION ${VIDEO_PLAYER_DATADIR})
INSTALL(DIRECTORY DESTINATION ${VP_DATADIR})
INSTALL(DIRECTORY DESTINATION ${VP_THUMBDIR})
INSTALL(DIRECTORY DESTINATION ${VP_CUR_THUMBDIR})
INSTALL(DIRECTORY DESTINATION ${VP_TEMPDIR})
INSTALL(DIRECTORY DESTINATION ${VP_SDPDIR})
ADD_SUBDIRECTORY(res/po)

