###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}Contrib/" )

OPTION( BUILD_ALL_SOURCEMODULES "Build all available source modules" ON )
OPTION( BUILD_CONTRIB "Build contrib modules" ON )
IF( BUILD_CONTRIB )
  SET( BUILD_ALL_SOURCEMODULES ON )
ENDIF()

IF( BUILD_ALL_SOURCEMODULES )
  ADD_SUBDIRECTORY( SignalSource )
ENDIF()

IF( BUILD_CONTRIB )
  ADD_SUBDIRECTORY( AppConnectorApplications )
  ADD_SUBDIRECTORY( OperatorLibApplications )
  ADD_SUBDIRECTORY( SignalProcessing )
  ADD_SUBDIRECTORY( Application )
  ADD_SUBDIRECTORY( ExternalLinks )
  ADD_SUBDIRECTORY( Extensions )
ENDIF()

SET( BUILD_BCPY2000
     FALSE CACHE BOOL
   "Whether to build BCPy2000 Python modules"
)
IF( BUILD_BCPY2000 )
  ADD_SUBDIRECTORY( BCPy2000 )
ENDIF( BUILD_BCPY2000 )
