
FIND_LIBRARY(ARTIK_CONNECTIVITY_LIBRARIES
    NAMES artik-sdk-connectivity
    HINTS /usr/lib/
          /usr/local/lib
)

FIND_PATH(ARTIK_CONNECTIVITY_INCLUDE_DIR
    NAMES artik_network.h
    HINTS /usr/include/artik/connectivity
          /usr/local/include/artik/connectivity
)

