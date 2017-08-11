
FIND_LIBRARY(ARTIK_LWM2M_LIBRARIES
    NAMES artik-sdk-lwm2m
    HINTS /usr/lib/
          /usr/local/lib
)

FIND_PATH(ARTIK_LWM2M_INCLUDE_DIR
    NAMES artik_lwm2m.h
    HINTS /usr/include/artik/lwm2m
          /usr/local/include/artik/lwm2m
)

