###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for NIDAQ-MX source module

# Set the executable name
SET( EXECUTABLE_NAME NIDAQ_mx_Source )

# Set the project specific sources
SET( SRC_PROJECT
  NIDAQ_mx_Source.cpp
  NIADC.cpp
  NIDAQ-Logging/NIDAQFilter.cpp
  NIDAQ-Logging/NIDAQLogger.cpp
)
SET( HDR_PROJECT
  NIDAQmxADC.h
  nidaq/NIDAQmx.h
  NIDAQ-Logging/NIDAQFilter.h
  NIDAQ-Logging/NIDAQLogger.h
)

INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NIDAQ-MX/nidaq )
LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NIDAQ-MX/nidaq )

IF( BORLAND OR MSVC )
  # Create the signal source module
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )
ELSE()
  MESSAGE( "NIDAQ-MX libraries not available for this compiler/OS, NIDAQ-MX source will not build." )
ENDIF()

IF( BORLAND )
  TARGET_LINK_LIBRARIES( "${EXECUTABLE_NAME}" NIDAQmx.lib )
ELSEIF( MSVC )
  TARGET_LINK_LIBRARIES( "${EXECUTABLE_NAME}" NIDAQmx_orig.lib )
ENDIF()
