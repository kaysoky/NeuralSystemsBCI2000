###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Sets up include directories and dependencies for 
##   SignalSource Modules using the SigSrcModule library

UTILS_INCLUDE( frameworks/Core )

# Define include directories
INCLUDE_DIRECTORIES(
  ${PROJECT_SRC_DIR}/shared/utils
  ${PROJECT_SRC_DIR}/shared/fileio
  ${PROJECT_SRC_DIR}/shared/fileio/dat
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf
  ${PROJECT_SRC_DIR}/shared/modules/signalsource
)

INCLUDE_DIRECTORIES( ${BCI2000_SIGSRCINCDIRS} )
LINK_DIRECTORIES( ${BCI2000_SIGSRCLIBDIRS} )

SET( REGISTRY_NAME SigSrcRegistry )
FORCE_INCLUDE_OBJECT( ${REGISTRY_NAME} )

SET( LIBS ${LIBS} BCI2000FrameworkSigSrcModule )
ADD_DEFINITIONS( -DIS_FIRST_MODULE ) 
