CMAKE_MINIMUM_REQUIRED	( VERSION 2.8 )
PROJECT			( artik-api-sensor-devices C )

SET ( SRC_SENSOR_DEVICES
					${CMAKE_CURRENT_SOURCE_DIR}/devices/accelerometer_arduino.c
					${CMAKE_CURRENT_SOURCE_DIR}/devices/humidity_arduino.c
					${CMAKE_CURRENT_SOURCE_DIR}/devices/light_arduino.c
					${CMAKE_CURRENT_SOURCE_DIR}/devices/temperature_arduino.c
					${CMAKE_CURRENT_SOURCE_DIR}/devices/proximity_arduino.c
					${CMAKE_CURRENT_SOURCE_DIR}/devices/infrared_flame_arduino.c
					${CMAKE_CURRENT_SOURCE_DIR}/devices/K6DS3.c
					${CMAKE_CURRENT_SOURCE_DIR}/devices/HTS221.c
					${CMAKE_CURRENT_SOURCE_DIR}/devices/LPS25HBTR.c
					${CMAKE_CURRENT_SOURCE_DIR}/devices/CM3323E.c
					${CMAKE_CURRENT_SOURCE_DIR}/devices/S5712CCDL1_I4T1U.c
)
