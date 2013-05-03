###########################################################################
## $Id: CMakeLists.txt 3117 2011-02-17 13:39:56Z mellinger $
## Authors: masayo.haneda@starlab.es / javier.acedo@starlab.es
## Description: Build information for ENOBIO3G

# Set the executable name
SET( EXECUTABLE_NAME ENOBIO3G )
INCLUDE_DIRECTORIES( 
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing
  ${BCI2000_SRC_DIR}/extlib/math
)
# Set the project specific sources
SET( SRC_PROJECT
  ENOBIO.cpp
  ENOBIOADC.cpp
  EnobioStatusConsumer.cpp
  EnobioDataConsumer.cpp 
${BCI2000_SRC_DIR}/shared/modules/signalsource/SourceFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRFilterBase.cpp
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.cpp

)
SET( HDR_PROJECT
  ENOBIOADC.h
  EnobioStatusConsumer.h
  EnobioDataConsumer.h
${BCI2000_SRC_DIR}/shared/modules/signalsource/SourceFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRFilterBase.h 
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.h
)






# Define the library
IF( MSVC )
  LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/ENOBIO3G/lib/ )
ENDIF( MSVC )

# Put everything together
IF( NOT MSVC )
  MESSAGE( "-- ENOBIO3G does not provide libraries for this compiler/OS,  Will not build" )
ELSE( NOT MSVC )

  # Create the signal source module
  BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
    "${INC_EXTLIB}"
  )

  # Link the Enobio Library
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} Enobio3GAPI )

  ADD_CUSTOM_COMMAND(
  	TARGET "${EXECUTABLE_NAME}"
  	POST_BUILD
  	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/Enobio3GAPI.dll" "${BCI2000_ROOT_DIR}/prog"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/WinBluetoothAPI.dll" "${BCI2000_ROOT_DIR}/prog"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/QtCore4.dll" "${BCI2000_ROOT_DIR}/prog"
    
  
  )

ENDIF( NOT MSVC)


