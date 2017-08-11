
FIND_LIBRARY(LIBZIGBEE_LIBRARIES
    NAMES zigbee
    HINTS /usr/lib/
          /usr/local/lib
)

FIND_PATH(LIBZIGBEE_INCLUDE_DIR
    NAMES zigbee.h
    HINTS /usr/include/zigbee
          /usr/local/include/zigbee
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ZIGBEE REQUIRED_VARS LIBZIGBEE_LIBRARIES LIBZIGBEE_INCLUDE_DIR)
