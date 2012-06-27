###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for SignalGenerator in Qt

# Set the executable name
SET( EXECUTABLE_NAME SignalGenerator )

# Set the project specific sources
SET( SRC_PROJECT
  SignalGeneratorADC.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/logging/TestLogger.cpp
)
SET( HDR_PROJECT
  SignalGeneratorADC.h
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/logging/TestLogger.h
)

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
