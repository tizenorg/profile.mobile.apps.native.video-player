CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(common C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(VERSION 0.1.0)

SET(SRCS
	src/vp-db-util.c
	src/vp-preview-db.c
	src/vp-thumb-db.c
	src/vp-chapter-db.c
	src/vp-media-content-util.c
	src/vp-util.c
	src/vp-file-util.c
	src/vp-pinch-zoom.c
)

INCLUDE_DIRECTORIES(
	include
	src
	src/include
	src/uuid
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
	sqlite3
	capi-content-media-content
	storage
)

FOREACH(flag ${${PROJECT_NAME}_pkgs_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS} -Wall")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRCS})

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${VERSION} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${${PROJECT_NAME}_pkgs_LDFLAGS})


#install resouce files.
# Lack of a trailing slash means install this directory, trailing slash means
# install everything in this directory.

# Install image files.

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION lib)



