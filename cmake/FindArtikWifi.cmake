
FIND_LIBRARY(ARTIK_WIFI_LIBRARIES
    NAMES artik-sdk-wifi
    HINTS /usr/lib/
          /usr/local/lib
)

FIND_PATH(ARTIK_WIFI_INCLUDE_DIR
    NAMES artik_wifi.h
    HINTS /usr/include/artik/wifi
          /usr/local/include/artik/wifi
)

