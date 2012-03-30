###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}Core/" )

# Recurse down into all project subdirectories
ADD_SUBDIRECTORY( SignalProcessing )
ADD_SUBDIRECTORY( SignalSource )
ADD_SUBDIRECTORY( Application )
ADD_SUBDIRECTORY( Operator )

SET( BUILD_TOOLS
     FALSE CACHE BOOL
	 "Whether to build BCI2000 tools"
)

IF( BUILD_TOOLS )
  ADD_SUBDIRECTORY( Tools )
ENDIF( BUILD_TOOLS )
