
FIND_LIBRARY(ARTIK_ZIGBEE_LIBRARIES
    NAMES artik-sdk-zigbee
    HINTS /usr/lib/
          /usr/local/lib
)

FIND_PATH(ARTIK_ZIGBEE_INCLUDE_DIR
    NAMES artik_zigbee.h
    HINTS /usr/include/artik/zigbee
          /usr/local/include/artik/zigbee
)

