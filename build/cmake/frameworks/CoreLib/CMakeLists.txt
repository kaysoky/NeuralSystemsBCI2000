###########################################################################
## $Id: BasicFramework.cmake 3482 2011-08-23 17:15:02Z mellinger $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Defines a static library containing a basic BCI2000 framework

SET( NAME BCI2000FrameworkCore )

MESSAGE( "---Adding Framework Library: ${NAME}" )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreLib/CoreLibFiles.cmake )

IF( WIN32 )
  SET( DEPLIBS ws2_32 winmm )
ENDIF() 
IF( NOT APPLE AND NOT WIN32 )
  SET( DEPLIBS rt pthread dl )
ENDIF()

# Include "usage header" to define include directories and extlib dependencies
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK DEPLIBS FAILED )
IF( FAILED )
  MESSAGE( ERROR "Could not satisfy dependencies of the CoreLib library." )
ELSE( FAILED )
  ADD_LIBRARY( ${NAME} STATIC ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} )
  TARGET_LINK_LIBRARIES( ${NAME} ${DEPLIBS} )
  
  # Set the project build folder
  SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )
ENDIF( FAILED )

