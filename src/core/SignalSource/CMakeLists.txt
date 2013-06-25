###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}SignalSource/" )

# Recurse down into all project subdirectories
ADD_SUBDIRECTORY( SignalGenerator )
UTILS_MATCH_SUBDIR( BUILD_MODULES g.USBamp )
UTILS_MATCH_SUBDIR( BUILD_MODULES g.MOBIlab )
UTILS_MATCH_SUBDIR( BUILD_MODULES g.MOBIlabPlus )
