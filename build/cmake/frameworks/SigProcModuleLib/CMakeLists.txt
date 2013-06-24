###########################################################################
## $Id: SigProcModule.cmake -1   $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Builds a static library containing components specific to
##   signal processing modules.

ADD_SUBDIRECTORY( test )

SET( NAME BCI2000FrameworkSigProcModule )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreModuleLib.cmake )

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
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

# Add Extlib dependencies
BCI2000_INCLUDE( "MATH" )
BCI2000_INCLUDE( "FFT" )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/SigProcModule.cmake )
BCI2000_ADD_REGISTRY( ${REGISTRY_NAME} SRC_BCI2000_FRAMEWORK DEPTARGETS )
BCI2000_ADD_TARGET( INFO "Framework library" STATIC_LIBRARY ${NAME} )

IF( FAILED )
  MESSAGE( WARNING "Could not satisfy dependencies of the SignalProcessing library. SignalProcessing modules will not build." )
ELSE( FAILED )

  TARGET_LINK_LIBRARIES( ${NAME} ${DEPLIBS} BCI2000FrameworkCore )
  ADD_DEPENDENCIES( ${NAME} ${DEPTARGETS} )

ENDIF( FAILED )
