
FIND_LIBRARY(DL_LIBRARIES
    NAMES dl
    HINTS /usr/lib
          /usr/local/lib
)

FIND_PATH(DL_INCLUDE_DIR
    NAMES dlfcn.h
    HINTS /usr/include
          /usr/local/include
)

