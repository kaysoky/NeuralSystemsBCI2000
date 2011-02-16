###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Recurse down into all project subdirectories
IF( BUILD_CONTRIB )
  ADD_SUBDIRECTORY( AppConnectorApplications )
  ADD_SUBDIRECTORY( OperatorLibApplications )
  ADD_SUBDIRECTORY( SignalSource )
  ADD_SUBDIRECTORY( SignalProcessing )
  ADD_SUBDIRECTORY( Application )
ENDIF( BUILD_CONTRIB )
IF( BUILD_BCPY2000 )
  ADD_SUBDIRECTORY( BCPy2000 )
ENDIF( BUILD_BCPY2000 )
