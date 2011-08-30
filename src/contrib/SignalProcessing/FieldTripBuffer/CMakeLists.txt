###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for FieldTripBuffer module

# Set the executable name
SET( EXECUTABLE_NAME FieldTripBuffer )

# Set the project specific sources
SET( SRC_PROJECT
  FieldTripBuffer.cpp
  FieldTripBufferFilter.cpp
  PipeDefinition.cpp
)
SET( HDR_PROJECT
  FieldTripBufferFilter.h
)

BCI2000_INCLUDE( "FIELDTRIP" )
IF( WIN32 )
  BCI2000_INCLUDE( "PTHREADS" )
ENDIF( WIN32 )

# Create the signal processing module
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

IF( MSVC AND WIN32 )
  ADD_CUSTOM_COMMAND(
    TARGET ${EXECUTABLE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${BCI2000_ROOT_DIR}/src/extlib/fieldtrip/pthreads-win32/lib/pthreadVC2.dll ${BCI2000_ROOT_DIR}/prog
  )
ENDIF( MSVC AND WIN32 )

IF( MINGW AND WIN32 )
  ADD_CUSTOM_COMMAND(
    TARGET ${EXECUTABLE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${BCI2000_ROOT_DIR}/src/extlib/fieldtrip/pthreads-win32/lib/pthreadGC2.dll ${BCI2000_ROOT_DIR}/prog
  )
ENDIF( MINGW AND WIN32 )
