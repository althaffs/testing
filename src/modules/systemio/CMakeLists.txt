CMAKE_MINIMUM_REQUIRED	( VERSION 2.8 )
PROJECT			( artik-sdk-systemio C CXX )

SET ( LIB_SYSTEMIO artik-sdk-systemio CACHE INTERNAL "" FORCE )
SET ( ARTIK_SYSTEMIO_INCLUDE_DIR ${LIB_INC}/systemio CACHE INTERNAL "" FORCE )
SET ( ARTIK_SYSTEMIO_LIBRARIES ${LIB_SYSTEMIO} CACHE INTERNAL "" FORCE )

SET ( SRC_SYSTEMIO		
					adc/linux_adc.c
					adc/artik_adc.c
					gpio/linux_gpio.c
					gpio/artik_gpio.c
					i2c/linux_i2c.c
					i2c/artik_i2c.c
					pwm/linux_pwm.c
					pwm/artik_pwm.c
					serial/linux_serial.c
					serial/artik_serial.c
					spi/linux_spi.c
					spi/artik_spi.c
					adc/cpp/artik_adc.cpp
					gpio/cpp/artik_gpio.cpp
					i2c/cpp/artik_i2c.cpp
					pwm/cpp/artik_pwm.cpp
					serial/cpp/artik_serial.cpp
					spi/cpp/artik_spi.cpp
)

ADD_LIBRARY ( ${LIB_SYSTEMIO} SHARED ${SRC_SYSTEMIO} )

TARGET_INCLUDE_DIRECTORIES ( ${LIB_SYSTEMIO} PUBLIC
							 ${ARTIK_BASE_INCLUDE_DIR}
							 ${ARTIK_SYSTEMIO_INCLUDE_DIR}
							 ${ARTIK_BASE_INCLUDE_DIR}/cpp
							 ${ARTIK_SYSTEMIO_INCLUDE_DIR}/cpp
)

TARGET_LINK_LIBRARIES ( ${LIB_SYSTEMIO} 
						${LIB_BASE}
)

SET_TARGET_PROPERTIES ( ${LIB_SYSTEMIO} PROPERTIES VERSION ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH} SOVERSION ${LIB_VERSION_MAJOR} OUTPUT_NAME ${LIB_SYSTEMIO} )

INSTALL ( TARGETS ${LIB_SYSTEMIO} LIBRARY DESTINATION lib )
FILE ( GLOB SYSTEMIO_HEADERS "${LIB_INC}/systemio/*.h" )
FILE ( GLOB SYSTEMIO_HEADERS_CPP "${LIB_INC}/systemio/cpp/*.hh" )
INSTALL ( FILES ${SYSTEMIO_HEADERS} DESTINATION include/artik/systemio )
INSTALL ( FILES ${SYSTEMIO_HEADERS_CPP} DESTINATION include/artik/systemio/cpp )
