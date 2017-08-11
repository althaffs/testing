
FIND_LIBRARY(ARTIK_MEDIA_LIBRARIES
    NAMES artik-sdk-media
    HINTS /usr/lib/
          /usr/local/lib
)

FIND_PATH(ARTIK_MEDIA_INCLUDE_DIR
    NAMES artik_media.h
    HINTS /usr/include/artik/media
          /usr/local/include/artik/media
)

