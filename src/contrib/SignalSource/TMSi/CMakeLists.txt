###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for TMSi source module

# Set the executable name
SET( EXECUTABLE_NAME TMSi )

# Set the project specific sources
SET( SRC_PROJECT
  TMSi.cpp
  TMSiADC.cpp
  TMSiSDK/Feature.cpp
  TMSiSDK/RTDevice.cpp
)
SET( HDR_PROJECT
  TMSiADC.h
  TMSiSDK/Feature.h
  TMSiSDK/RTDevice.h
  TMSiSDK/Sadio.h
)

INCLUDE_DIRECTORIES( ${BCI2000_ROOT_DIR}/src/contrib/SignalSource/TMSi/TMSiSDK )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${INC_EXTLIB}"
)
