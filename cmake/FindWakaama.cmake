
FIND_LIBRARY(LIBWAKAAMA_LIBRARIES
    NAMES wakaama-client
    HINTS /usr/lib
          /usr/local/lib
)

FIND_PATH(LIBWAKAAMA_INCLUDE_DIR
    NAMES lwm2mclient.h
    HINTS /usr/include
          /usr/local/include
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(WAKAAMA REQUIRED_VARS LIBWAKAAMA_LIBRARIES LIBWAKAAMA_INCLUDE_DIR)



