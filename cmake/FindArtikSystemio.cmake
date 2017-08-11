
FIND_LIBRARY(ARTIK_SYSTEMIO_LIBRARIES
    NAMES artik-sdk-systemio
    HINTS /usr/lib/
          /usr/local/lib
)

FIND_PATH(ARTIK_SYSTEMIO_INCLUDE_DIR
    NAMES artik_gpio.h
    HINTS /usr/include/artik/systemio
          /usr/local/include/artik/systemio
)

