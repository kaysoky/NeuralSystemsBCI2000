###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Sets up include directories and dependencies for 
##   SignalSource Modules using the SigSrcModule library

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreModule.cmake )

# Define include directories
INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/shared/fileio
  ${BCI2000_SRC_DIR}/shared/fileio/dat
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf
  ${BCI2000_SRC_DIR}/shared/modules/signalsource
)

INCLUDE_DIRECTORIES( ${BCI2000_SIGSRCINCDIRS} )
LINK_DIRECTORIES( ${BCI2000_SIGSRCLIBDIRS} )

SET( REGISTRY_NAME SigSrcRegistry )
FORCE_INCLUDE_OBJECT( ${REGISTRY_NAME} )

SET( LIBS ${LIBS} BCI2000FrameworkSigSrcModule )
