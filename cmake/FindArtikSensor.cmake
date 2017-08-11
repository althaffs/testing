
FIND_LIBRARY(ARTIK_SENSOR_LIBRARIES
    NAMES artik-sdk-sensor
    HINTS /usr/lib/
          /usr/local/lib
)

FIND_PATH(ARTIK_SENSOR_INCLUDE_DIR
    NAMES artik_sensor.h
    HINTS /usr/include/artik/sensor
          /usr/local/include/artik/sensor
)

