
FIND_LIBRARY(ARTIK_MQTT_LIBRARIES
    NAMES artik-sdk-mqtt
    HINTS /usr/lib/
          /usr/local/lib
)

FIND_PATH(ARTIK_MQTT_INCLUDE_DIR
    NAMES artik_mqtt.h
    HINTS /usr/include/artik/mqtt
          /usr/local/include/artik/mqtt
)

