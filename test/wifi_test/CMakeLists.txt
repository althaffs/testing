CMAKE_MINIMUM_REQUIRED	( VERSION 2.8 )
PROJECT		  	( wifi-test )

FIND_PACKAGE ( ArtikBase )
FIND_PACKAGE ( ArtikWifi )

SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter" )

SET ( EXE_WIFI_TEST wifi-test )
SET ( EXE_WIFI_AP_TEST wifi-ap-test )

SET ( SRC_TEST_WIFI	artik_wifi_test.c
)
SET ( SRC_TEST_WIFI_AP	artik_wifi_ap_test.c
)

ADD_EXECUTABLE		( ${EXE_WIFI_TEST} ${SRC_TEST_WIFI} )
ADD_EXECUTABLE		( ${EXE_WIFI_AP_TEST} ${SRC_TEST_WIFI_AP} )

TARGET_INCLUDE_DIRECTORIES ( ${EXE_WIFI_TEST}
                             PUBLIC ${ARTIK_BASE_INCLUDE_DIR}
                             PUBLIC ${ARTIK_WIFI_INCLUDE_DIR}
)
TARGET_INCLUDE_DIRECTORIES ( ${EXE_WIFI_AP_TEST}
                             PUBLIC ${ARTIK_BASE_INCLUDE_DIR}
                             PUBLIC ${ARTIK_WIFI_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES ( ${EXE_WIFI_TEST}
                        ${ARTIK_BASE_LIBRARIES}
                        ${ARTIK_WIFI_LIBRARIES}
)
TARGET_LINK_LIBRARIES ( ${EXE_WIFI_AP_TEST}
                        ${ARTIK_BASE_LIBRARIES}
                        ${ARTIK_WIFI_LIBRARIES}
)

INSTALL ( TARGETS ${EXE_WIFI_TEST} RUNTIME DESTINATION lib/artik-sdk/tests )
INSTALL ( TARGETS ${EXE_WIFI_AP_TEST} RUNTIME DESTINATION lib/artik-sdk/tests )
