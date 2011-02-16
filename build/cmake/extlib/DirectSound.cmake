###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the 3D API in a project
## SETS:
##       SRC_EXTLIB_DSOUND - Required source files for DirectSound
##       HDR_EXTLIB_DSOUND - Required header files for DirectSound
##       INC_EXTLIB_DSOUND - Include directory for DirectSound
##       LIBDIR_EXTLIB_DSOUND - Directory where the DSound library resides
##       LIBS_EXTLIB_DSOUND - Required libraries for DirectSound
##       Also defines source groups for the DirectSound files

IF( WIN32 )
IF( NOT BORLAND )

# Define the source files
SET( SRC_EXTLIB_DSOUND )

# Define the headers
SET( HDR_EXTLIB_DSOUND ${BCI2000_SRC_DIR}/extlib/dxsdk/include/dsound.h )

# Define the include directory
SET( INC_EXTLIB_DSOUND ${BCI2000_SRC_DIR}/extlib/dxsdk/include )

# Set the source groups
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\dxsdk\\include FILES ${HDR_EXTLIB_DSOUND} )

# Set where the libraries can be found
SET( LIBDIR_EXTLIB_DSOUND ${BCI2000_SRC_DIR}/extlib/dxsdk/lib )

# Set the name of the library to link against within LIBDIR
SET( LIBS_EXTLIB_DSOUND dsound.lib )

ENDIF( NOT BORLAND )

# Set success
SET( DSOUND_OK TRUE )

ELSE( WIN32 )

MESSAGE( "- WARNING: DirectSound required (currently) for audio." )  
MESSAGE( "- WARNING: Cannot build audio support for this application." )

# Set success
SET( DSOUND_OK FALSE )

ENDIF( WIN32 )
