###########################################################################
## $Id: BasicFramework.cmake 3482 2011-08-23 17:15:02Z mellinger $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Defines a static library containing a basic BCI2000 framework

# Setup build config information
FUNCTION( BUILD_CONFIG )
  SET( config )

  IF( 0 )
  GET_CMAKE_PROPERTY( vars VARIABLES )
  FOREACH( var ${vars} )
    IF( var MATCHES "^BUILD_" AND ${var} )
      SET( config "${config} ${var}" )
    ENDIF()
  ENDFOREACH()
  ENDIF( 0 )
  
  IF( QT_QMAKE_EXECUTABLE )
    EXECUTE_PROCESS( COMMAND ${QT_QMAKE_EXECUTABLE} -v RESULT_VARIABLE result OUTPUT_VARIABLE output )
    IF( result EQUAL 0 )
      STRING( REGEX MATCHALL "[^\n]+" lines ${output} )
      FOREACH( line ${lines} )
        IF( NOT line MATCHES "^QMake" )
          SET( config "${config} ${line}" )
        ENDIF()
      ENDFOREACH()
    ENDIF()
  ENDIF()

  ADD_DEFINITIONS( -DBUILD_CONFIG=\"${config}\" )

ENDFUNCTION()


ADD_SUBDIRECTORY( test )

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
#BCI2000_ADD_REGISTRY( ${REGISTRY_NAME} SRC_BCI2000_FRAMEWORK DEPTARGETS )
BUILD_CONFIG()
BCI2000_ADD_TARGET( STATIC_LIBRARY ${NAME} )
IF( FAILED )
  MESSAGE( ERROR "Could not satisfy dependencies of the CoreLib library." )
ELSE( FAILED )
  TARGET_LINK_LIBRARIES( ${NAME} ${DEPLIBS} )
ENDIF( FAILED )
