
FIND_LIBRARY(ARTIK_BASE_LIBRARIES
    NAMES artik-sdk-base
    HINTS /usr/lib/
          /usr/local/lib
)

FIND_PATH(ARTIK_BASE_INCLUDE_DIR
    NAMES artik_module.h
    HINTS /usr/include/artik/base
          /usr/local/include/artik/base
)

