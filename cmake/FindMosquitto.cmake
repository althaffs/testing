
FIND_LIBRARY(LIBMOSQUITTO_LIBRARIES
    NAMES mosquitto
    HINTS /usr/lib
          /usr/local/lib
)

FIND_PATH(LIBMOSQUITTO_INCLUDE_DIR
    NAMES mosquitto.h
    HINTS /usr/include
          /usr/local/include
)



