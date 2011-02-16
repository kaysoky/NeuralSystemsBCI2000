###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including the vAmp API
## SETS:
##       SRC_EXTLIB_VAMP - Required source files for vAmp
##       HDR_EXTLIB_VAMP - Required header files for vAmp
##       INC_EXTLIB_VAMP - Include directory for vAmp
##       LIBDIR_EXTLIB_VAMP - Library directory for vAmp
##       LIBS_EXTLIB_VAMP - required library for vAmp
##       Also defines source groups for source files

IF( WIN32 )

# Set the Source and headers
SET( SRC_EXTLIB_VAMP )
SET( HDR_EXTLIB_VAMP
  ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp/FirstAmp.h
)

# Define the include directory
SET( INC_EXTLIB_VAMP 
  ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp
)

# Define where the library is
IF( BORLAND )
SET( LIBDIR_EXTLIB_VAMP ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp )
ENDIF( BORLAND )
IF( MSVC )
SET( LIBDIR_EXTLIB_VAMP ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp/coff )
ENDIF( MSVC )
IF( MINGW )
SET( LIBDIR_EXTLIB_VAMP ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp/mingw )
ENDIF( MINGW )

# Set Libs required
IF( MINGW )
SET( LIBS_EXTLIB_VAMP libFirstAmp.a )
ELSE( MINGW )
SET( LIBS_EXTLIB_VAMP FirstAmp.lib )
ENDIF( MINGW )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\brainproducts\\vamp FILES ${SRC_EXTLIB_VAMP} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\brainproducts\\vamp FILES ${HDR_EXTLIB_VAMP} )

# Set success
SET( VAMP_OK TRUE )

ELSE( WIN32 )

  MESSAGE( "- WARNING: vAmp/FirstAmp libraries only exist for windows.  This module will not build." )
  SET( VAMP_OK FALSE )

ENDIF( WIN32 )
