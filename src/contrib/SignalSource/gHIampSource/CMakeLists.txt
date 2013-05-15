###########################################################################
## $Id:  $
## Authors: griffin.milsap@gmail.com
## Description: Build information for the gHIampSource source module
###########################################################################

IF( NOT USE_DYNAMIC_IMPORTS AND ( NOT WIN32 OR NOT MSVC ) )

  MESSAGE( "-- gHIamp doesn't support this Compiler/OS.  Will not build." )

ELSE()

  # Set the executable name
  SET( EXECUTABLE_NAME gHIampSource )

  # Set the project specific sources
  SET( SRC_PROJECT
    gHIampADC.cpp
    gHIampDevice.cpp
    lib/gHIamp.imports.cpp
  )

  SET( HDR_PROJECT
    gHIampADC.h
    gHIampDevice.h
    lib/gHIamp.imports.h
  )

  INCLUDE_DIRECTORIES(
    ${BCI2000_SRC_DIR}/contrib/SignalSource/gHIampSource/lib
  )

  IF( USE_DYNAMIC_IMPORTS )
  ELSEIF( BORLAND )
    LINK_DIRECTORIES(
      ${BCI2000_SRC_DIR}/contrib/SignalSource/gHIampSource/lib/omf
    )
  ELSEIF( CMAKE_CL_64 )
    LINK_DIRECTORIES(
      ${BCI2000_SRC_DIR}/contrib/SignalSource/gHIampSource/lib/coff64
    )
  ELSE()
    LINK_DIRECTORIES(
      ${BCI2000_SRC_DIR}/contrib/SignalSource/gHIampSource/lib/coff
    )
  ENDIF()

  # Add the source module to the Makefile or compiler project file:
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )

  IF( NOT USE_DYNAMIC_IMPORTS )
    TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} gHIamp )
  ENDIF()

ENDIF()
