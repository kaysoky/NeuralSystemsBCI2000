###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}Contrib/" )


# Recurse down into all project subdirectories

SET( BUILD_MFC
     FALSE CACHE BOOL
	 "Whether to build modules that use MFC (leave this disabled unless you are using a full, paid-for version of Visual Studio)"
)

SET( BUILD_CONTRIB
     FALSE CACHE BOOL
	 "Whether to build contrib modules"
)
IF( BUILD_CONTRIB )
  ADD_SUBDIRECTORY( AppConnectorApplications )
  ADD_SUBDIRECTORY( OperatorLibApplications )
  ADD_SUBDIRECTORY( SignalSource )
  ADD_SUBDIRECTORY( SignalProcessing )
  ADD_SUBDIRECTORY( Application )
  ADD_SUBDIRECTORY( Extensions )
  ADD_SUBDIRECTORY( ExternalLinks )
ENDIF( BUILD_CONTRIB )


SET( BUILD_BCPY2000
     FALSE CACHE BOOL
	 "Whether to build BCPy2000 Python modules"
)
IF( BUILD_BCPY2000 )
  ADD_SUBDIRECTORY( BCPy2000 )
ENDIF( BUILD_BCPY2000 )
