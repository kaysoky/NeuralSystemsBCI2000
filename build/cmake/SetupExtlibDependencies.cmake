###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Modifies the framework and sets up libraries to be linked

# Sets up the extlib dependencies by looping through the BCI2000_INCLUDING var
MACRO( BCI2000_SETUP_EXTLIB_DEPENDENCIES SRC_FRAMEWORK HDR_FRAMEWORK LIBS FAILED )

# Make sure the input is treated as variables
SET( SOURCES "${SRC_FRAMEWORK}" )
SET( HEADERS "${HDR_FRAMEWORK}" )
SET( LIBRARIES "${LIBS}" )
SET( ${FAILED} "" )

# We'll loop through the using statements
FOREACH( INC ${BCI2000_INCLUDING} )
  
  IF( NOT EXISTS ${BCI2000_CMAKE_DIR}/extlib/${INC}.cmake )
    MESSAGE( FATAL_ERROR
      "Unknown extlib dependency: ${INC}."
      "Make sure all BCI2000_INCLUDE statements have a corresponding .cmake file in src/extlib."
    )
  ELSE()
    SET( EXTLIB_OK FALSE )
    UNSET( INC_EXTLIB )
    UNSET( HDR_EXTLIB )
    UNSET( SRC_EXTLIB )
    UNSET( LIBDIR_EXTLIB )
    UNSET( LIBS_EXTLIB )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/${INC}.cmake )
    IF( EXTLIB_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB} )
      LINK_DIRECTORIES( ${LIBDIR_EXTLIB} )
      SET( ${LIBRARIES}
        ${${LIBRARIES}}
        ${LIBS_EXTLIB}
      )
    ELSE( EXTLIB_OK )
      MESSAGE( " - WARNING: Could not satisfy dependency on ${INC} extlib." )
      SET( ${FAILED}
        ${${FAILED}}
        ${INC}
      )
    ENDIF( EXTLIB_OK )
  ENDIF()

ENDFOREACH( INC )

# Clear out all dependencies for next project
UNSET( BCI2000_INCLUDING )

ENDMACRO( BCI2000_SETUP_EXTLIB_DEPENDENCIES SRC_FRAMEWORK HDR_FRAMEWORK LIBS )

# Add an EXTLIB dependency to ${BCI2000_INCLUDING}
MACRO( BCI2000_INCLUDE LIB )
  STRING(
    TOUPPER( ${LIB} LIB ) 
  )
  SET( BCI2000_INCLUDING 
    ${BCI2000_INCLUDING}
    ${LIB}
  )
ENDMACRO( BCI2000_INCLUDE LIB ) 

# Use an EXTLIB that is contained in a library
MACRO( BCI2000_USE LIB )
  STRING( TOUPPER( ${LIB} LIB ) )
  IF( NOT EXISTS ${BCI2000_CMAKE_DIR}/extlib/${LIB}.cmake )
    MESSAGE( FATAL_ERROR
      "Unknown extlib dependency: ${LIB}."
      "Make sure all BCI2000_USE statements have a corresponding .cmake file in src/extlib."
    )
  ELSE()
    SET( EXTLIB_OK FALSE )
    UNSET( INC_EXTLIB )
    UNSET( LIBDIR_EXTLIB )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/${LIB}.cmake )
    IF( EXTLIB_OK )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB} )
      LINK_DIRECTORIES( ${LIBDIR_EXTLIB} )
    ENDIF()
  ENDIF()
ENDMACRO( BCI2000_USE LIB )
