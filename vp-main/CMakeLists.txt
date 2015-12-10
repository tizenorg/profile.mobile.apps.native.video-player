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
	libtzplatform-config
)

#SET(VIDEO_PLAYER_PACKAGE "com.${VIDEO_PLAYER_VENDOR}.video-player")
SET(VIDEO_PLAYER_PACKAGE "org.tizen.video-player")
SET(VIDEO_PLAYER_DIR "${TZ_SYS_RO_APP}/${VIDEO_PLAYER_PACKAGE}")
SET(VIDEO_PLAYER_RES_DIR "${TZ_SYS_RO_APP}/${VIDEO_PLAYER_PACKAGE}/res")
SET(VIDEO_PLAYER_BINDIR "${TZ_SYS_RO_APP}/${VIDEO_PLAYER_PACKAGE}/bin")
SET(VP_RESDIR "${TZ_SYS_RO_APP}/${VIDEO_PLAYER_PACKAGE}/res")
SET(VP_LOCALEDIR "${VP_RESDIR}/locale")
SET(VP_CUR_THUMBDIR "/opt/usr/media/.cur_video_thumb")

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
INSTALL(FILES ${CMAKE_SOURCE_DIR}/res/images/org.tizen.video-player.png DESTINATION  ${CMAKE_DESKTOP_ICON_DIR})
INSTALL(FILES ${VIDEO_PLAYER_PACKAGE}.xml DESTINATION ${CMAKE_APP_SHARE_PACKAGES_DIR})

# install application HOME directory
INSTALL(DIRECTORY DESTINATION ${VP_CUR_THUMBDIR})
ADD_SUBDIRECTORY(res/po)

