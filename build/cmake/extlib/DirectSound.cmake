###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including DirectSound in a project
## SETS:
##       SRC_EXTLIB - Required source files for DirectSound
##       HDR_EXTLIB - Required header files for DirectSound
##       INC_EXTLIB - Include directory for DirectSound
##       LIBDIR_EXTLIB - Directory where the DSound library resides
##       LIBS_EXTLIB - Required libraries for DirectSound
##       Also defines source groups for the DirectSound files

IF( WIN32 )
IF( NOT BORLAND )

# Define the source files
SET( SRC_EXTLIB )

# Define the headers
SET( HDR_EXTLIB ${BCI2000_SRC_DIR}/extlib/dxsdk/include/dsound.h )

# Define the include directory
SET( INC_EXTLIB ${BCI2000_SRC_DIR}/extlib/dxsdk/include )

# Set the source groups
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\dxsdk\\include FILES ${HDR_EXTLIB} )

# Set where the libraries can be found
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/dxsdk/lib )

# Set the name of the library to link against within LIBDIR
SET( LIBS_EXTLIB dsound.lib )

ENDIF( NOT BORLAND )

# Set success
SET( EXTLIB_OK TRUE )

ELSE( WIN32 )

MESSAGE( "- WARNING: DirectSound required (currently) for audio." )  
MESSAGE( "- WARNING: Cannot build audio support for this application." )

# Set success
SET( EXTLIB_OK FALSE )

ENDIF( WIN32 )
