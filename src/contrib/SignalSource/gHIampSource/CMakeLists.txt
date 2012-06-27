###########################################################################
## $Id:  $
## Authors: griffin.milsap@gmail.com
## Description: Build information for the gHIampSource source module
###########################################################################

IF( NOT WIN32 OR CMAKE_CL_64 )

  MESSAGE( "-- gHIamp doesn't support this OS.  Will not build." )

ELSE()

  # Set the executable name
  SET( EXECUTABLE_NAME gHIampSource )

  # Set the project specific sources
  SET( SRC_PROJECT
    gHIampADC.cpp
    gHIampDevice.cpp
  )

  SET( HDR_PROJECT
    gHIampADC.h
    gHIampDevice.h
  )

  INCLUDE_DIRECTORIES(
    ${BCI2000_SRC_DIR}/contrib/SignalSource/gHIampSource/lib/inc
  )

  IF( BORLAND )
    LINK_DIRECTORIES(
      ${BCI2000_SRC_DIR}/contrib/SignalSource/gHIampSource/lib/omf
    )
  ELSE( BORLAND )
    LINK_DIRECTORIES(
      ${BCI2000_SRC_DIR}/contrib/SignalSource/gHIampSource/lib/coff
    )
  ENDIF( BORLAND )

  # Add the source module to the Makefile or compiler project file:
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )

  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} gHIamp )

ENDIF()
