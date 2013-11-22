###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Defines a static library containing LibTiny utils.

ADD_SUBDIRECTORY( test )

SET( NAME LibTiny )

UTILS_INCLUDE( frameworks/LibTiny/Files )

IF( WIN32 )
  SET( DEPLIBS ws2_32 winmm )
ENDIF() 
IF( NOT APPLE AND NOT WIN32 )
  SET( DEPLIBS rt pthread dl )
ENDIF()

# Include "usage header" to define include directories and extlib dependencies
UTILS_INCLUDE( frameworks/LibTiny )
BCI2000_ADD_TARGET( INFO "Tiny utility library" STATIC_LIBRARY ${NAME} ${SRC_LIBTINY})
IF( FAILED )
  MESSAGE( ERROR "Could not satisfy dependencies of the LibTiny library." )
ELSE( FAILED )
  TARGET_LINK_LIBRARIES( ${NAME} ${DEPLIBS} )
ENDIF( FAILED )
