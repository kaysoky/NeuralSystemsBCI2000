###########################################################################
## $Id: BasicFramework.cmake 3482 2011-08-23 17:15:02Z mellinger $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Defines a static library containing a basic BCI2000 framework

ADD_SUBDIRECTORY( test )

SET( NAME BCI2000FrameworkCore )

UTILS_INCLUDE( frameworks/CoreLib/CoreLibFiles )

IF( WIN32 )
  SET( DEPLIBS ws2_32 winmm )
ENDIF() 
IF( NOT APPLE AND NOT WIN32 )
  SET( DEPLIBS rt pthread dl )
ENDIF()

# Include "usage header" to define include directories and extlib dependencies
UTILS_INCLUDE( frameworks/Core )
#BCI2000_ADD_REGISTRY( ${REGISTRY_NAME} SRC_BCI2000_FRAMEWORK DEPTARGETS )
BCI2000_ADD_TARGET( INFO "Framework library" STATIC_LIBRARY ${NAME} )
IF( FAILED )
  MESSAGE( ERROR "Could not satisfy dependencies of the CoreLib library." )
ELSE( FAILED )
  TARGET_LINK_LIBRARIES( ${NAME} ${DEPLIBS} )
ENDIF( FAILED )
