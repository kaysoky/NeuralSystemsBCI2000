###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Build information for PythonSignalProcessing

# Set the executable name
SET( EXECUTABLE_NAME PythonSignalProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  PythonSignalProcessing.cpp
  ../shared/PythonFilter.cpp
  ../shared/PythonLink24.cpp
  ../shared/PythonLink25.cpp
  ../shared/PythonLink26.cpp
  ../shared/PythonWrapper.cpp
)
SET( HDR_PROJECT
  ../shared/PythonFilter.h
  ../shared/PythonWrapper.h
)

# Set required signal processing source files
SET( SRC_SIGNALPROCESSING
  ../../../../shared/modules/signalprocessing/SpatialFilter.cpp
  ../../../../shared/modules/signalprocessing/SpatialFilterCell.cpp
  ../../../../shared/modules/signalprocessing/SpatialFilterGroup.cpp
)
SET( HDR_SIGNALPROCESSING
  ../../../../shared/modules/signalprocessing/SpatialFilter.h
  ../../../../shared/modules/signalprocessing/SpatialFilterCell.h
  ../../../../shared/modules/signalprocessing/SpatialFilterGroup.h
)

# Define extra include directories
SET( INCLUDES
)

ADD_DEFINITIONS(
  -DDYNAMIC_PYTHON=1
)

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_SIGNALPROCESSING}" "${HDR_SIGNALPROCESSING}" 
  "${INCLUDES}"
)

ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  COMMAND ${CMAKE_COMMAND} -E copy "${BCI2000_ROOT_DIR}/prog/PythonSignalProcessing.exe" "${CMAKE_CURRENT_SRC_DIR}/../../demo/prog"
)
