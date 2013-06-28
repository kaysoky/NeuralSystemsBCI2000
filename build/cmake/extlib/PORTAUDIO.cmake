###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including PortAudio in a project
## SETS:
##       SRC_EXTLIB - Required source files
##       HDR_EXTLIB - Required header files
##       INC_EXTLIB - Include directory for
##       LIBDIR_EXTLIB - Directory where the library resides
##       LIBS_EXTLIB - Required libraries

SET( SRC_EXTLIB )
SET( HDR_EXTLIB )
SET( INC_EXTLIB ${PROJECT_SRC_DIR}/extlib/portaudio/portaudio/include )
SET( LIBDIR_EXTLIB )
SET( LIBS_EXTLIB PortAudio )
SET( EXTLIB_OK TRUE )
