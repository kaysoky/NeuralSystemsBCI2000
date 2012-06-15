###########################################################################
## $Id: SigProcModule.cmake -1   $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Builds a static library containing components specific to
##   signal processing modules.

SET( NAME BCI2000FrameworkSigProcModule )

MESSAGE( "---Adding Framework Library: ${NAME}" )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreModuleLib.cmake )

SET( SIGPROCSOURCES 
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterGroup.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRFilterBase.cpp 
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRBandpass.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ThreadedFilter.cpp  
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/WindowingFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpectrumThread.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ARSpectrum.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/FFTSpectrum.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpectralEstimator.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ARFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/P3TemporalFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LinearClassifier.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LPFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ExpressionFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/Normalizer.cpp

  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/AverageDisplay.cpp 
#  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ComplexDemodulator.cpp 
#  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ConditionalIntegrator.cpp 
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/RandomFilter.cpp 
#  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/StateTransform.cpp 
)
SET( SIGPROCHEADERS 
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterGroup.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRFilterBase.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRBandpass.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ThreadedFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/WindowingFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpectrumThread.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ARSpectrum.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/FFTSpectrum.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/SpectralEstimator.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ARFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/P3TemporalFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LinearClassifier.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/LPFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ExpressionFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/Normalizer.h

  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/AverageDisplay.h 
#  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ComplexDemodulator.h 
#  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/ConditionalIntegrator.h 
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/RandomFilter.h
#  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/StateTransform.h
)

SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\signalprocessing FILES ${SIGPROCSOURCES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\signalprocessing FILES ${SIGPROCHEADERS} )

# Add the required signal processing files into the framework for this project
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${SIGPROCSOURCES}
)
SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${SIGPROCHEADERS}
)

# Add Extlib dependencies
BCI2000_INCLUDE( "MATH" )
BCI2000_INCLUDE( "FFT" )

# Create the library project
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/SigProcModule.cmake )
BCI2000_ADD_REGISTRY( ${REGISTRY_NAME} SRC_BCI2000_FRAMEWORK DEPTARGETS )
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK DEPLIBS FAILED )
IF( FAILED )
  MESSAGE( WARNING "Could not satisfy dependencies of the SignalProcessing library. SignalProcessing modules will not build." )
ELSE( FAILED )
  ADD_LIBRARY( ${NAME} STATIC ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} )
  TARGET_LINK_LIBRARIES( ${NAME} ${DEPLIBS} BCI2000FrameworkCore )
  ADD_DEPENDENCIES( ${NAME} ${DEPTARGETS} )
  # Add Pre-processor defines
  IF( NOT BORLAND )
    SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=2 -DUSE_QT" )
  ELSE( NOT BORLAND )
    SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=2" )
  ENDIF( NOT BORLAND )
  # Link against the Qt/VCL Libraries
  IF( BORLAND )
    TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} cp32mt.lib )
  ELSE( BORLAND )
    TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} )
  ENDIF( BORLAND )

  # Set the project build folder
  SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )
ENDIF( FAILED )

