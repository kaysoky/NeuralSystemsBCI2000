###########################################################################
## $Id: SigProcModule.cmake -1   $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Sets up include directories and dependencies for 
##   SigProc Modules using the SigProcModule library

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreModule.cmake )

# Define include directories
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/shared/modules/signalprocessing )

BCI2000_USE( "FFT" )
BCI2000_USE( "MATH" )

SET( REGISTRY_NAME SigProcRegistry )
FORCE_INCLUDE_OBJECT( ${REGISTRY_NAME} )

SET( LIBS ${LIBS} BCI2000FrameworkSigProcModule )
SET( DEPENDS ${DEPENDS} BCI2000FrameworkSigProcModule )

