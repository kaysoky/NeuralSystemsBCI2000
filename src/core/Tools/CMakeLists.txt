###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}Tools/" )


# Recurse down into all project subdirectories
IF( NOT BORLAND )
  ADD_SUBDIRECTORY( BCI2000FileInfoQt )
  ADD_SUBDIRECTORY( BCI2000ViewerQt )
  ADD_SUBDIRECTORY( BCI2000ExportQt )
  ADD_SUBDIRECTORY( BCI2000LauncherQt )
  ADD_SUBDIRECTORY( P300Classifier )
  IF( WIN32 )
    ADD_SUBDIRECTORY( BCI2000Certification )
  ENDIF( WIN32 )
ENDIF( NOT BORLAND )
ADD_SUBDIRECTORY( cmdline )
