###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for P3SignalProcessing in Qt

# Set the executable name
SET( EXECUTABLE_NAME P3SignalProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  P3SignalProcessing.cpp
  PipeDefinition.cpp 
)
SET( HDR_PROJECT

)

# Set required signal processing source files
SET( SRC_SIGNALPROCESSING
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterCell.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterGroup.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/P3TemporalFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LinearClassifier.cpp
)
SET( HDR_SIGNALPROCESSING
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterCell.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterGroup.h 
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/P3TemporalFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LinearClassifier.h
)

# Add additional EXTLIB libs
BCI2000_USE( "MATH" )

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_SIGNALPROCESSING}" "${HDR_SIGNALPROCESSING}" 
  "${INC_EXTLIB}"
)
