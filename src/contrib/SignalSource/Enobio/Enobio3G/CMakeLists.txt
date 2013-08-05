###########################################################################
## $Id$
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
  IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
    SET( libdir lib )
  ELSE()
    # N/A
  ENDIF()
ENDIF()

# Put everything together
IF( NOT libdir )
  MESSAGE( "-- ENOBIO3G does not provide libraries for this compiler/OS,  Will not build" )
ELSE()

  LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/${libdir} )

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

ENDIF()


