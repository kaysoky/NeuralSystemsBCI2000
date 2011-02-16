###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for FFTSignalProcessing in Qt
 
# Set the executable name
SET( EXECUTABLE_NAME FFTSignalProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  FFTSignalProcessing.cpp
  PipeDefinition.cpp 
)
SET( HDR_PROJECT

)

# Set required signal processing source files
SET( SRC_SIGNALPROCESSING
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterCell.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterGroup.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/FFTFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LinearClassifier.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LPFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/Normalizer.cpp
)
SET( HDR_SIGNALPROCESSING 
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterCell.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterGroup.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/P3TemporalFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LinearClassifier.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LPFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/Normalizer.h
)

# Let the macro know we mean to include other libraries from EXTLIB
BCI2000_USE( "MATH" )
BCI2000_USE( "FFT" )

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_SIGNALPROCESSING}" "${HDR_SIGNALPROCESSING}" 
  "${INC_EXTLIB}"
)
