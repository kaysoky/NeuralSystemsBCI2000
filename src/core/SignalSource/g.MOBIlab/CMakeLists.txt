###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for gMOBIlab

# Set the executable name
SET( EXECUTABLE_NAME gMOBIlab )

# Set the project specific sources
SET( SRC_PROJECT
  gMOBIlab.cpp
  gMOBIlabADC.cpp 
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/SourceFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRFilterBase.cpp
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.cpp
)
SET( HDR_PROJECT
  gMOBIlabADC.h
  gMOBIlabDriver.h
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/SourceFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRFilterBase.h
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.h
  ${BCI2000_SRC_DIR}/extlib/math/IIRFilter.h
)

SET( INC_EXTLIB
  ${INC_EXTLIB}
  ${BCI2000_SRC_DIR}/extlib/math
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing
)

# Tell the macro to link against the gMOBIlab libraries in extlib
BCI2000_USE( "GMOBILAB" )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${INC_EXTLIB}"
)


