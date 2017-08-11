CMAKE_MINIMUM_REQUIRED	( VERSION 2.8 )
PROJECT			( artik-sdk C )

FIND_PACKAGE(PkgConfig)
ADD_SUBDIRECTORY ( modules )

#FILE ( GLOB headers "${LIB_INC}/*" )
#FILE ( GLOB platform_headers "${LIB_INC}/platform/artik_*.h" )

#INSTALL ( FILES ${headers} DESTINATION include/artik/ )
#INSTALL ( FILES ${platform_headers} DESTINATION include/artik/sdk/platform )

