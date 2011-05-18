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
  
  # Setup the 3DAPI
  IF( "${USE}" STREQUAL "3DAPI" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/3DAPI.cmake )
    IF( 3DAPI_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_3DAPI}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_3DAPI}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_3DAPI} )
      SET( ${LIBRARIES}
        ${${LIBRARIES}}
        ${LIBS_EXTLIB_3DAPI}
      )
    ENDIF( 3DAPI_OK )
    UNSET( 3DAPI_OK )

  # Setup SAPI
  ELSEIF( "${USE}" STREQUAL "SAPI" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/SAPI.cmake )
    IF( SAPI_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_SAPI}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_SAPI}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_SAPI} )
      LINK_DIRECTORIES( ${LIBDIR_EXTLIB_SAPI} )
      SET( ${LIBRARIES}
        ${${LIBRARIES}}
        ${LIBS_EXTLIB_SAPI}
      )
    ENDIF( SAPI_OK )
    UNSET( SAPI_OK )

  # Setup DirectSound
  ELSEIF( "${USE}" STREQUAL "DSOUND" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/DirectSound.cmake )
    IF( DSOUND_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_DSOUND}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_DSOUND}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_DSOUND} )
      LINK_DIRECTORIES( ${LIBDIR_EXTLIB_DSOUND} )
      SET( ${LIBRARIES}
        ${${LIBRARIES}}
        ${LIBS_EXTLIB_DSOUND}
      )
    ENDIF( DSOUND_OK )
    UNSET( DSOUND_OK )

  # Setup Math
  ELSEIF( "${USE}" STREQUAL "MATH" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/Math.cmake )
    IF( MATH_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_MATH}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_MATH}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_MATH} )
    ENDIF( MATH_OK )
    UNSET( MATH_OK )

  # Setup FFT
  ELSEIF( "${USE}" STREQUAL "FFT" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/FFT.cmake )
    IF( FFT_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_FFT}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_FFT}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_FFT} )
      LINK_DIRECTORIES( ${LIBDIR_EXTLIB_FFT} )
      SET( ${LIBRARIES}
        ${${LIBRARIES}}
        ${LIBS_EXTLIB_FFT}
      )
    ENDIF( FFT_OK )
    UNSET( FFT_OK )

  # Setup Matlab
  ELSEIF( "${USE}" STREQUAL "MATLAB" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/Matlab.cmake )
    IF( MATLAB_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_MATLAB}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_MATLAB}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_MATLAB} )
    ENDIF( MATLAB_OK )
    UNSET( MATLAB_OK )

  # Setup gUSBAmp
  ELSEIF( "${USE}" STREQUAL "GUSBAMP" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/gUSBamp.cmake )
    IF( GUSBAMP_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_GUSBAMP}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_GUSBAMP}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_GUSBAMP} )
      LINK_DIRECTORIES( ${LIBDIR_EXTLIB_GUSBAMP} )
      SET( ${LIBRARIES}
        ${${LIBRARIES}}
        ${LIBS_EXTLIB_GUSBAMP}
      )
    ENDIF( GUSBAMP_OK )
    UNSET( GUSBAMP_OK )

  # Setup gMOBIlab
  ELSEIF( "${USE}" STREQUAL "GMOBILAB" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/gMOBIlab.cmake )
    IF( GMOBILAB_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_GMOBILAB}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_GMOBILAB}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_GMOBILAB} )
      LINK_DIRECTORIES( ${LIBDIR_EXTLIB_GMOBILAB} )
      SET( ${LIBRARIES}
        ${${LIBRARIES}}
        ${LIBS_EXTLIB_GMOBILAB}
      )
    ENDIF( GMOBILAB_OK )
    UNSET( GMOBILAB_OK )

  # Setup gMOBIlabPlus
  ELSEIF( "${USE}" STREQUAL "GMOBILABPLUS" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/gMOBIlabPlus.cmake )
    IF( GMOBILABPLUS_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_GMOBILABPLUS}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_GMOBILABPLUS}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_GMOBILABPLUS} )
      LINK_DIRECTORIES( ${LIBDIR_EXTLIB_GMOBILABPLUS} )
      SET( ${LIBRARIES}
        ${${LIBRARIES}}
        ${LIBS_EXTLIB_GMOBILABPLUS}
      )
    ENDIF( GMOBILABPLUS_OK )
    UNSET( GMOBILABPLUS_OK )
  
  # Setup vAmp
  ELSEIF( "${USE}" STREQUAL "VAMP" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/vAmp.cmake )
    IF( VAMP_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_VAMP}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_VAMP}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_VAMP} )
      LINK_DIRECTORIES( ${LIBDIR_EXTLIB_VAMP} )
      SET( ${LIBRARIES}
        ${${LIBRARIES}}
        ${LIBS_EXTLIB_VAMP}
      )
    ENDIF( VAMP_OK )
    UNSET( VAMP_OK )

  # Setup alglib
  ELSEIF( "${USE}" STREQUAL "ALGLIB" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/alglib.cmake )
    IF( ALGLIB_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_ALGLIB}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_ALGLIB}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_ALGLIB} )
    ENDIF( ALGLIB_OK )
    UNSET( ALGLIB_OK )
  
  # Setup FieldTrip
  ELSEIF( "${USE}" STREQUAL "FIELDTRIP" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/FieldTrip.cmake )
    IF( FIELDTRIP_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_FIELDTRIP}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_FIELDTRIP}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_FIELDTRIP} )
    ENDIF( FIELDTRIP_OK )
    UNSET( FIELDTRIP_OK )

  # Setup pthreads
  ELSEIF( "${USE}" STREQUAL "PTHREADS" )
    INCLUDE( ${BCI2000_CMAKE_DIR}/extlib/pthreads.cmake )
    IF( PTHREADS_OK )
      SET( ${SOURCES}
        ${${SOURCES}}
        ${SRC_EXTLIB_PTHREADS}
      )
      SET( ${HEADERS}
        ${${HEADERS}}
        ${HDR_EXTLIB_PTHREADS}
      )
      INCLUDE_DIRECTORIES( ${INC_EXTLIB_PTHREADS} )
      LINK_DIRECTORIES( ${LIBDIR_EXTLIB_PTHREADS} )
      SET( ${LIBRARIES}
        ${${LIBRARIES}}
        ${LIBS_EXTLIB_PTHREADS}
      )
    ENDIF( PTHREADS_OK )
    UNSET( PTHREADS_OK )
    
  ELSE()
    MESSAGE( FATAL_ERROR "Invalid argument to BCI2000_USE macro: " ${USE} )
  ENDIF()

ENDFOREACH( USE )

# Clear out all dependencies for next project
UNSET( BCI2000_USING )

ENDMACRO( BCI2000_SETUP_EXTLIB_DEPENDENCIES SRC_FRAMEWORK HDR_FRAMEWORK LIBS )

# Add an EXTLIB dependency to ${BCI2000_USING}
MACRO( BCI2000_USE LIB )
SET( BCI2000_USING 
  ${BCI2000_USING}
  ${LIB}
)
ENDMACRO( BCI2000_USE LIB ) 
