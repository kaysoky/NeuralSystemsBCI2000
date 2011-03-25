###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Recurse down into all project subdirectories
IF( NOT BORLAND )
  ADD_SUBDIRECTORY( BCI2000FileInfoQt )
  ADD_SUBDIRECTORY( BCI2000ViewerQt )
  ADD_SUBDIRECTORY( BCI2000ExportQt )
  ADD_SUBDIRECTORY( BCI2000LauncherQt )
  ADD_SUBDIRECTORY( P300Classifier )
  ADD_SUBDIRECTORY( BCI2000Certification )
ENDIF()
ADD_SUBDIRECTORY( BCI2000CheckEnvironment )
ADD_SUBDIRECTORY( cmdline )


