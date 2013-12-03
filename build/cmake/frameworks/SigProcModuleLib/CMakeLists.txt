###########################################################################
## $Id: SigProcModule.cmake -1   $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Builds a static library containing components specific to
##   signal processing modules.

ADD_SUBDIRECTORY( test )

SET( NAME BCI2000FrameworkSigProcModule )

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/SpatialFilter.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/SpatialFilterGroup.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/IIRFilterBase.cpp 
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/IIRBandpass.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/ThreadedFilter.cpp  
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/WindowingFilter.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/SpectrumThread.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/ARSpectrum.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/FFTSpectrum.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/SpectralEstimator.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/ARFilter.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/P3TemporalFilter.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/LinearClassifier.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/LPFilter.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/ExpressionFilter.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/Normalizer.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/FrequencyEstimator.cpp

  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/AverageDisplay.cpp 
#  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/ComplexDemodulator.cpp 
#  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/ConditionalIntegrator.cpp 
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/RandomFilter.cpp 
#  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/StateTransform.cpp 
)

# Add Extlib dependencies
BCI2000_INCLUDE( "MATH" )
BCI2000_INCLUDE( "FFT" )

UTILS_INCLUDE( frameworks/SigProcModule )
BCI2000_ADD_REGISTRY( ${REGISTRY_NAME} SRC_BCI2000_FRAMEWORK DEPTARGETS )
BCI2000_ADD_TARGET( INFO "Framework library" STATIC_LIBRARY ${NAME} )

IF( FAILED )
  MESSAGE( WARNING "Could not satisfy dependencies of the SignalProcessing library. SignalProcessing modules will not build." )
ELSE( FAILED )

  TARGET_LINK_LIBRARIES( ${NAME} ${DEPLIBS} BCI2000FrameworkCore )
  ADD_DEPENDENCIES( ${NAME} ${DEPTARGETS} )

ENDIF( FAILED )
