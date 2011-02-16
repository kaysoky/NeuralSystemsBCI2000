###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for the Emotiv module

# Set the executable name
SET( EXECUTABLE_NAME Emotiv )

# Set the project specific sources
SET( SRC_PROJECT
  Emotiv.cpp
  EmotivADC.cpp 
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/SourceFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRFilterBase.cpp
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.cpp
)
SET( HDR_PROJECT
  EmotivADC.h
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/SourceFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRFilterBase.h 
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.h
)

INCLUDE_DIRECTORIES( 
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing
  ${BCI2000_SRC_DIR}/extlib/math
)

# Define the library
IF( BORLAND )
  LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/Emotiv/lib/omf )
ELSE( BORLAND )
  LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/Emotiv/lib/coff )
ENDIF( BORLAND )

INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/Emotiv/lib/include )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${INC_EXTLIB}"
)

# Link the Emotiv Library
TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} edk )

ADD_CUSTOM_COMMAND(
  TARGET "${EXECUTABLE_NAME}"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/dll/edk.dll" "${BCI2000_ROOT_DIR}/prog"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/dll/edk_utils.dll" "${BCI2000_ROOT_DIR}/prog"
)
