CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )

SET ( LIB_VERSION_MAJOR 1 CACHE STRING "Library version major")
SET ( LIB_VERSION_MINOR 4 CACHE STRING "Library version minor")
SET ( LIB_VERSION_PATCH 0 CACHE STRING "Library version patch")

SET ( MODULE "" CACHE STRING "Compile specific source module " )
SET ( TEST_X "" CACHE STRING "Compile specific unit test " )
SET ( NAME_TARGET "" CACHE STRING "Compile to a specific architecture target" )
SET ( PATH_TARGET "" CACHE STRING "Path of target ROOT " )

PROJECT	( artik-sdk C CXX )

SET ( EXT_DLL .so )
IF ( "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" )
    SET ( EXT_DLL .dylib )
ENDIF( )

SET ( NAME_TARGET ${CMAKE_SYSTEM_PROCESSOR} )

SET ( LIB_TARGET Release/${NAME_TARGET}/lib )
SET ( BIN_TARGET Release/${NAME_TARGET}/bin )

SET ( LIB_DIR ${CMAKE_BINARY_DIR}/${LIB_TARGET} )
SET ( BIN_DIR ${CMAKE_BINARY_DIR}/${BIN_TARGET} )

IF ( CMAKE_SYSROOT )
    SET ( CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT} )
    SET ( C_CPP_FLAGS "-Werror -Wall -Wpedantic -Wno-unused-parameter -Wno-unused-result --sysroot=${CMAKE_SYSROOT} -Wl,--sysroot=${CMAKE_SYSROOT}" )
    SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_CPP_FLAGS}" )
    SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_CPP_FLAGS}" )
    SET ( NODE_CFLAGS "-I${CMAKE_SYSROOT}/usr/include" )
    LINK_DIRECTORIES ( ${LIB_DIR} ${CMAKE_SYSROOT}/usr/lib ${CMAKE_SYSROOT}/usr )
	IF (DEFINED CMAKE_PKG_CONFIG_PATH)
		SET ( ENV{PKG_CONFIG_PATH} "${CMAKE_PKG_CONFIG_PATH}" )
		SET ( ENV{PKG_CONFIG_LIBDIR} "${CMAKE_PKG_CONFIG_PATH}")
	ELSE ( )
		SET ( ENV{PKG_CONFIG_PATH} "${CMAKE_SYSROOT}/lib/pkgconfig" )
		set ( ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/lib/pkgconfig" )
	ENDIF ( )
	MESSAGE("PKG CONFIG PATH is $ENV{PKG_CONFIG_PATH}")
	SET ( ENV{PKG_CONFIG_DIR} "")
	SET ( ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})
ELSE ( )
    LINK_DIRECTORIES ( ${LIB_DIR} )
    SET ( NODE_CFLAGS "" )
ENDIF ( )

SET ( LIB_INC ${CMAKE_CURRENT_SOURCE_DIR}/inc )
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

SET ( CMAKE_INSTALL_PREFIX /usr/ )
SET ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR} )
SET ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR} )
SET ( CMAKE_SKIP_BUILD_RPATH TRUE )

FILE ( GLOB pkgconfig "${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/*.pc" )
INSTALL ( FILES ${pkgconfig} DESTINATION lib/pkgconfig )

SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" )

IF ( NOT CMAKE_BUILD_TYPE )
	MESSAGE ( "-- Defaulting to Release build" )
	SET ( CMAKE_BUILD_TYPE Release CACHE STRING "Build configuration (Release|Debug)" FORCE)
ENDIF ( )

SET ( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DCONFIG_RELEASE" )
SET ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DCONFIG_RELEASE" )

# Figure out host processor
EXECUTE_PROCESS ( COMMAND uname -p
		  COMMAND xargs echo -n
		  OUTPUT_VARIABLE HOST_PROCESSOR )

MESSAGE ( "-- Host Processor: " ${HOST_PROCESSOR} )
MESSAGE ( "-- Target Processor: " ${CMAKE_SYSTEM_PROCESSOR} )
MESSAGE ( "-- Cross compiling: " ${CMAKE_CROSSCOMPILING} )

FUNCTION ( exec_cmake_target name_target path_target )
	 EXECUTE_PROCESS ( COMMAND rm -rf CMakeCache.txt    )
ENDFUNCTION ( exec_cmake_target )

FUNCTION ( exec_check_syntax target path_target )
	 ADD_CUSTOM_TARGET ( ${target}
			      COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/scripts/code-syntax.sh ${path_target}
			      VERBATIM )
ENDFUNCTION ( exec_check_syntax )

FUNCTION ( conf_build_option )
	 MAKE_DIRECTORY ( ${BIN_DIR} )
	 MAKE_DIRECTORY ( ${LIB_DIR} )
ENDFUNCTION ( conf_build_option )

FUNCTION ( build_src_c_cpp module )
	 SET ( ARCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/ )
	 SET ( MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/ )
	 ADD_SUBDIRECTORY ( src )
ENDFUNCTION ( build_src_c_cpp )

FUNCTION ( build_src module )
        ADD_DEFINITIONS ( -DLIB_VERSION_MAJOR=${LIB_VERSION_MAJOR}
                          -DLIB_VERSION_MINOR=${LIB_VERSION_MINOR}
                          -DLIB_VERSION_PATCH=${LIB_VERSION_PATCH} )
	  build_src_c_cpp( ${module} )
ENDFUNCTION ( build_src )

FUNCTION ( build_test unit )
	 SET ( TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/module_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/http_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/cloud_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/gpio_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/loop_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/i2c_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/serial_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/pwm_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/adc_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/wifi_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/media_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/time_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/security_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/spi_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/sensor_test )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/websocket_test  )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/bluetooth_test  )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/zigbee_test  )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/layercpp_test  )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/network_test  )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/lwm2m_test  )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/mqtt_test  )
	 ADD_SUBDIRECTORY ( ${TEST_DIR}/sdr_test )
ENDFUNCTION ( build_test )

FUNCTION ( check_syntax )
	  execute_process( COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -name "*.[ch]" -path "*/src/modules/*"  -printf "%h/%f " OUTPUT_VARIABLE SRC_DATA )
	  execute_process( COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -name "*.h" -path "*/inc/*"  -printf "%h/%f " OUTPUT_VARIABLE INC_DATA )
	  execute_process( COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -name "*.[ch]" -path "*/test/*" -printf "%h/%f " OUTPUT_VARIABLE TEST_DATA )
	  exec_check_syntax( "check_syntax" "${SRC_DATA} ${INC_DATA} ${TEST_DATA}")
ENDFUNCTION ( check_syntax )

FUNCTION ( build_main )
    conf_build_option()
    check_syntax()
    build_src("all")
    IF ( CMAKE_BUILD_TEST )
    	build_test("all")
    ENDIF ( CMAKE_BUILD_TEST )
ENDFUNCTION ( build_main )

build_main ( )
