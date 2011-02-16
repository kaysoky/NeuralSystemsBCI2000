###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for ARSignalProcessing in Qt

# Set the executable name
SET( EXECUTABLE_NAME ARSignalProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  ARSignalProcessing.cpp
  PipeDefinition.cpp 
)
SET( HDR_PROJECT

)

# Set required signal processing source files
SET( SRC_SIGNALPROCESSING 
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterCell.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterGroup.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ARFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ARGroup.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ARChannel.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LinearClassifier.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LPFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ExpressionFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/Normalizer.cpp 
)
SET( HDR_SIGNALPROCESSING 
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterCell.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterGroup.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ARFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ARGroup.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ARChannel.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LinearClassifier.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LPFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ExpressionFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/Normalizer.h
)

# Let the macro know we want to compile with the Extlib math lib
BCI2000_USE( "MATH" )

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_SIGNALPROCESSING}" "${HDR_SIGNALPROCESSING}" 
  "${INC_EXTLIB}"
)


