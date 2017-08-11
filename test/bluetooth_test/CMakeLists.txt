CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )
PROJECT ( bluetooth-test )

FIND_PACKAGE ( ArtikBase )
FIND_PACKAGE ( ArtikBluetooth )
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES( CUNIT REQUIRED cunit )

SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter" )

INCLUDE_DIRECTORIES(
	${ARTIK_BASE_INCLUDE_DIR}
	${ARTIK_BLUETOOTH_INCLUDE_DIR}
)

ADD_EXECUTABLE ( bluetooth-test artik_bluetooth_test.c )
TARGET_LINK_LIBRARIES ( bluetooth-test
	${ARTIK_BASE_LIBRARIES}
	${ARTIK_BLUETOOTH_LIBRARIES}
)

INSTALL ( TARGETS bluetooth-test RUNTIME DESTINATION lib/artik-sdk/tests )

SET( TESTS
	avrcp
	panu
	nap
	spp
	ftp
	advertiser
	observer
	gatt_client
	gatt_client_rw
	agent
	gatt_server
	hrp_collector
	hrp_sensor
	unit
	spp_client
	unit_ftp
	unit_spp
	unit_avrcp
	unit_nap
	unit_panu
)

FOREACH ( test ${TESTS})
	SET ( TARGET bluetooth-test-${test} )
	ADD_EXECUTABLE ( ${TARGET}
		artik_bluetooth_test_${test}.c
		artik_bluetooth_test_commandline.c
	)

	TARGET_LINK_LIBRARIES ( ${TARGET}
		${ARTIK_BASE_LIBRARIES}
		${ARTIK_BLUETOOTH_LIBRARIES}
		${CUNIT_LIBRARIES}
	)

	INSTALL ( TARGETS ${TARGET} RUNTIME DESTINATION lib/artik-sdk/tests )
ENDFOREACH ( test )
