CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )
PROJECT ( artik-sdk-bluetooth C CXX )

PKG_CHECK_MODULES ( GLIB REQUIRED glib-2.0 )
PKG_CHECK_MODULES ( GIO REQUIRED gio-2.0 )

IF ( NOT EXISTS ${LIB_INC} )
	SET ( LIB_INC ${CMAKE_CURRENT_SOURCE_DIR}/../../../inc )
ENDIF ( NOT EXISTS ${LIB_INC} )
SET ( LIB_VERSION_MAJOR 0 CACHE STRING "Library version major")
SET ( LIB_VERSION_MINOR 10 CACHE STRING "Library version minor")
SET ( LIB_BLUETOOTH artik-sdk-bluetooth CACHE INTERNAL "" FORCE )
SET ( ARTIK_BASE_INCLUDE_DIR ${LIB_INC}/base CACHE INTERNAL "" FORCE )
SET ( ARTIK_BLUETOOTH_INCLUDE_DIR ${LIB_INC}/bluetooth CACHE INTERNAL "" FORCE )
SET ( ARTIK_BLUETOOTH_LIBRARIES ${LIB_BLUETOOTH} CACHE INTERNAL "" FORCE )

SET ( SRC_BLUETOOTH	artik_bluetooth.c
	linux/a2dp.c
	linux/avrcp.c
	linux/adapter.c
	linux/agent.c
	linux/advertisement.c
	linux/bt.c
	linux/core.c
	linux/device.c
	linux/gatt.c
	linux/gatt_client.c
	linux/gatt_server.c
	linux/helper.c
	linux/pan.c
	linux/spp.c
	linux/ftp.c
	linux/assigned_numbers.c
	linux/gatt.c
	linux/linux_bluetooth.c
	cpp/artik_bluetooth.cpp
)

ADD_LIBRARY ( ${LIB_BLUETOOTH} SHARED ${SRC_BLUETOOTH} )

TARGET_INCLUDE_DIRECTORIES ( ${LIB_BLUETOOTH} PUBLIC
	${ARTIK_BASE_INCLUDE_DIR}
	${ARTIK_BLUETOOTH_INCLUDE_DIR}
	${GLIB_INCLUDE_DIRS}
	${LIB_INC}/base/cpp
	${LIB_INC}/bluetooth/cpp
)

INCLUDE_DIRECTORIES ( linux )

TARGET_LINK_LIBRARIES ( ${LIB_BLUETOOTH}
	${LIB_BASE}
	${GLIB_LIBRARIES}
	${GIO_LIBRARIES}
)

SET_TARGET_PROPERTIES ( ${LIB_BLUETOOTH} PROPERTIES
	VERSION ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH}
	SOVERSION ${LIB_VERSION_MAJOR}
	OUTPUT_NAME ${LIB_BLUETOOTH}
)

INSTALL ( TARGETS ${LIB_BLUETOOTH} LIBRARY DESTINATION lib )
FILE ( GLOB BLUETOOTH_HEADERS "${LIB_INC}/bluetooth/*.h" )
FILE ( GLOB BLUETOOTH_HEADERS_CPP "${LIB_INC}/bluetooth/cpp/*.hh" )
INSTALL ( FILES ${BLUETOOTH_HEADERS} DESTINATION include/artik/bluetooth )
INSTALL ( FILES ${BLUETOOTH_HEADERS_CPP} DESTINATION include/artik/bluetooth/cpp )
