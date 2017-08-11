CMAKE_MINIMUM_REQUIRED	( VERSION 2.8 )
PROJECT			( artik-sdk-wifi C CXX )

SET ( LIB_WIFI artik-sdk-wifi CACHE INTERNAL "" FORCE )
SET ( ARTIK_WIFI_INCLUDE_DIR ${LIB_INC}/wifi CACHE INTERNAL "" FORCE )
SET ( ARTIK_WIFI_LIBRARIES ${LIB_WIFI} CACHE INTERNAL "" FORCE )

SET ( SRC_WIFI
					artik_wifi.c
					linux/linux_wifi.c
					linux/wifi.c
					linux/wpas/eloop.c
					linux/wpas/common.c
					linux/wpas/os_unix.c
					linux/wpas/wpa_debug.c
					linux/wpas/wpa_ctrl.c
					linux/wpas/wpa_cli.c
					cpp/artik_wifi.cpp
)

INCLUDE_DIRECTORIES (  linux )

ADD_LIBRARY ( ${LIB_WIFI} SHARED ${SRC_WIFI} )


TARGET_INCLUDE_DIRECTORIES ( ${LIB_WIFI} PUBLIC
							 ${ARTIK_BASE_INCLUDE_DIR}
							 ${ARTIK_WIFI_INCLUDE_DIR}
							 ${ARTIK_BASE_INCLUDE_DIR}/cpp
							 ${ARTIK_WIFI_INCLUDE_DIR}/cpp
)

TARGET_LINK_LIBRARIES ( ${LIB_WIFI}
						${LIB_BASE}
)

SET_TARGET_PROPERTIES ( ${LIB_WIFI} PROPERTIES VERSION ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH} SOVERSION ${LIB_VERSION_MAJOR} OUTPUT_NAME ${LIB_WIFI} )

INSTALL ( TARGETS ${LIB_WIFI} LIBRARY DESTINATION lib )
FILE ( GLOB WIFI_HEADERS "${LIB_INC}/wifi/*.h" )
FILE ( GLOB WIFI_HEADERS_CPP "${LIB_INC}/wifi/cpp/*.hh" )
INSTALL ( FILES ${WIFI_HEADERS} DESTINATION include/artik/wifi )
INSTALL ( FILES ${WIFI_HEADERS_CPP} DESTINATION include/artik/wifi/cpp )
