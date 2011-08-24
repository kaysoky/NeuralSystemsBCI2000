###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Modifies the framework and sets up libraries to be linked

# Sets up the extlib dependencies by looping through the BCI2000_USING var
MACRO( BCI2000_SETUP_EXTLIB_DEPENDENCIES SRC_FRAMEWORK HDR_FRAMEWORK LIBS )

# Make sure the input is treated as variables
SET( SOURCES "${SRC_FRAMEWORK}" )
SET( HEADERS "${HDR_FRAMEWORK}" )
SET( LIBRARIES "${LIBS}" )

# We'll loop through the using statements
FOREACH( USE ${BCI2000_USING} )
  
  IF( NOT EXISTS ${BCI2000_CMAKE_DIR}/extlib/${USE}.cmake )
    MESSAGE( FATAL_ERROR "Unknown extlib dependency: ${USE}. Make sure all BCI2000_USE statements have a corresponding .cmake file in src/extlib." )
  ELSE()
    SET( EXTLIB_OK FALSE )
    UNSET( INC_EXTLIB )
    UNSET( HDR_EXTLIB )
    UNSET( SRC_EXTLIB )
    UNSET( LIBDIR_EXTLIB )
    UNSET( LIBS_EXTLIB )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/${USE}.cmake )
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
    ENDIF( EXTLIB_OK )
  ENDIF()

ENDFOREACH( USE )

# Clear out all dependencies for next project
UNSET( BCI2000_USING )

ENDMACRO( BCI2000_SETUP_EXTLIB_DEPENDENCIES SRC_FRAMEWORK HDR_FRAMEWORK LIBS )

# Add an EXTLIB dependency to ${BCI2000_USING}
MACRO( BCI2000_USE LIB )
STRING(
  TOUPPER( ${LIB} LIB ) 
)
SET( BCI2000_USING 
  ${BCI2000_USING}
  ${LIB}
)
ENDMACRO( BCI2000_USE LIB ) 
