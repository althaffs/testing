
FIND_LIBRARY(ARTIK_BLUETOOTH_LIBRARIES
    NAMES artik-sdk-bluetooth
    HINTS /usr/lib/
          /usr/local/lib
)

FIND_PATH(ARTIK_BLUETOOTH_INCLUDE_DIR
    NAMES artik_bluetooth.h
    HINTS /usr/include/artik/bluetooth
          /usr/local/include/artik/bluetooth
)

