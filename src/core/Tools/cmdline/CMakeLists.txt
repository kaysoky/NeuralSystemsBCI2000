###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Build information for the command line tools in cmake

# Tools from the cmdline directory.
SET( TOOLS
  bci_dat2stream
  bci_stream2asc
  bci_decimate
  bci_prm2stream
  bci_stream2mat
  bci_stream2prm
  bci_stream2table
)

FOREACH( EXECUTABLE_NAME ${TOOLS} )

  # Set the project specific sources
  SET( SRC_PROJECT
    ${EXECUTABLE_NAME}.cpp
    bci_tool.cpp
  )

  SET( HDR_PROJECT
    bci_tool.h
  )

  # Create the Command line module
  BCI2000_ADD_TOOLS_CMDLINE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )
  
ENDFOREACH( EXECUTABLE_NAME )


# Filters residing in other directories.
SET( SIGPROC_DIR ${BCI2000_ROOT_DIR}/src/shared/modules/signalprocessing )

SET( SRC_FILTERS
  ${SIGPROC_DIR}/ARFilter
  ${SIGPROC_DIR}/ComplexDemodulator
  ${SIGPROC_DIR}/ConditionalIntegrator
  ${SIGPROC_DIR}/ExpressionFilter
  ${SIGPROC_DIR}/FFTFilter
  ${SIGPROC_DIR}/LinearClassifier
  ${SIGPROC_DIR}/LPFilter
  ${BCI2000_ROOT_DIR}/src/core/SignalProcessing/Matlab/MatlabFilter
  ${SIGPROC_DIR}/Normalizer
  ${SIGPROC_DIR}/P3TemporalFilter
  ${SIGPROC_DIR}/SpatialFilter
  ${SIGPROC_DIR}/StateTransform
  ${BCI2000_ROOT_DIR}/src/shared/modules/signalsource/TransmissionFilter
)

FOREACH( FILTER ${SRC_FILTERS} )

  GET_FILENAME_COMPONENT( EXECUTABLE_NAME ${FILTER} NAME_WE )

  SET( SRC_PROJECT
    ${FILTER}.cpp
    bci_tool.cpp
    bci_filtertool.cpp
  )

  SET( HDR_PROJECT
    ${FILTER}.h
    bci_tool.h
  )
  
  IF( ${EXECUTABLE_NAME} STREQUAL ARFilter )
    BCI2000_USE( "MATH" )
    SET( SRC_PROJECT
      ${SRC_PROJECT}
      ${SIGPROC_DIR}/ARChannel.cpp
      ${SIGPROC_DIR}/ARGroup.cpp
    )
    SET( HDR_PROJECT
      ${HDR_PROJECT}
      ${SIGPROC_DIR}/ARChannel.h
      ${SIGPROC_DIR}/ARGroup.h
    )
  ENDIF()    
  
  IF( ${EXECUTABLE_NAME} STREQUAL FFTFilter )
    BCI2000_USE( "FFT" )
  ENDIF()
  
  IF( ${EXECUTABLE_NAME} STREQUAL MatlabFilter )
    BCI2000_USE( "MATLAB" )
    SET( SRC_PROJECT
      ${SRC_PROJECT}
      ${BCI2000_ROOT_DIR}/src/core/SignalProcessing/Matlab/MatlabWrapper.cpp
    )
    SET( HDR_PROJECT
      ${HDR_PROJECT}
      ${BCI2000_ROOT_DIR}/src/core/SignalProcessing/Matlab/MatlabWrapper.h
    )
  ENDIF()

  IF( ${EXECUTABLE_NAME} STREQUAL SpatialFilter )
    SET( SRC_PROJECT
      ${SRC_PROJECT}
      ${SIGPROC_DIR}/SpatialFilterCell.cpp
      ${SIGPROC_DIR}/SpatialFilterGroup.cpp
    )
    SET( HDR_PROJECT
      ${HDR_PROJECT}
      ${SIGPROC_DIR}/SpatialFilterCell.h
      ${SIGPROC_DIR}/SpatialFilterGroup.h
    )
  ENDIF()    

  # Create the Command line module
  BCI2000_ADD_TOOLS_CMDLINE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
  )

ENDFOREACH( FILTER )
