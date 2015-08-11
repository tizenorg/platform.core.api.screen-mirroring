SET(fw_name "capi-media-screen-mirroring")
SET(fw_test "${fw_name}-rtsp-server")

SET(INC_DIR ../include)
INCLUDE_DIRECTORIES(${INC_DIR})

INCLUDE(FindPkgConfig)
pkg_check_modules(${fw_test} REQUIRED glib-2.0 gio-2.0 dlog capi-base-common gstreamer-1.0 gstreamer-plugins-base-1.0 gstreamer-rtsp-server-1.0)
FOREACH(flag ${${fw_test}_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Wall -Werror -pie")

aux_source_directory(. sources)

ADD_LIBRARY(miracast_server_lib SHARED miracast_server_impl.c)
TARGET_LINK_LIBRARIES(miracast_server_lib ${fw_name} ${${fw_test}_LDFLAGS})
INSTALL(TARGETS miracast_server_lib DESTINATION ${LIB_INSTALL_DIR})

ADD_EXECUTABLE(miracast_server miracast_server.c)
TARGET_LINK_LIBRARIES(miracast_server ${fw_name} ${${fw_test}_LDFLAGS} miracast_server_lib)
INSTALL(TARGETS miracast_server DESTINATION /usr/bin)
