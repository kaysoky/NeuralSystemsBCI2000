###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for MatlabSignalProcessing in Qt

# Set the executable name
SET( EXECUTABLE_NAME MatlabSignalProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  MatlabSignalProcessing.cpp
  PipeDefinition.cpp 
  MatlabFilter.cpp
  MatlabWrapper.cpp
)
SET( HDR_PROJECT
  MatlabFilter.h
  MatlabWrapper.h
)

# Set required signal processing source files 
SET( SRC_SIGNALPROCESSING
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterCell.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterGroup.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LinearClassifier.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/Normalizer.cpp 
)
SET( HDR_SIGNALPROCESSING
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterCell.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterGroup.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LinearClassifier.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/Normalizer.h
)

# Make sure the macro knows we want to use EXTLIB libs!
BCI2000_USE( "MATH" )
BCI2000_USE( "MATLAB" )

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_SIGNALPROCESSING}" "${HDR_SIGNALPROCESSING}" 
  "${INC_EXTLIB}"
)
