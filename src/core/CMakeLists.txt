###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Recurse down into all project subdirectories
ADD_SUBDIRECTORY( SignalProcessing )
ADD_SUBDIRECTORY( SignalSource )
ADD_SUBDIRECTORY( Application )
ADD_SUBDIRECTORY( Operator )
IF( BUILD_TOOLS )
  ADD_SUBDIRECTORY( Tools )
ENDIF()
