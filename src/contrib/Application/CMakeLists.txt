###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Recurse down into all project subdirectories
IF( BUILD_MFC )
  ADD_SUBDIRECTORY( MFCDemo )
ENDIF()