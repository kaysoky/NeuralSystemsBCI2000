###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the gUSBamp API
## SETS:
##       SRC_EXTLIB_GUSBAMP - Required source files for gUSBamp
##       HDR_EXTLIB_GUSBAMP - Required header files for gUSBamp
##       INC_EXTLIB_GUSBAMP - Include directory for gUSBamp
##       LIBDIR_EXTLIB_GUSBAMP - Library directory for gUSBamp
##       LIBS_EXTLIB_GUSBAMP - required library for gUSBamp
##       Also defines source groups for source files

IF( WIN32 )

# Set the Source and headers
SET( SRC_EXTLIB_GUSBAMP )
SET( HDR_EXTLIB_GUSBAMP
  ${BCI2000_SRC_DIR}/extlib/gtec/gUSBamp/gUSBamp.h
)

# Define the include directory
SET( INC_EXTLIB_GUSBAMP 
  ${BCI2000_SRC_DIR}/extlib/gtec/gUSBamp
)

# Define where the library is
IF( BORLAND )
SET( LIBDIR_EXTLIB_GUSBAMP ${BCI2000_SRC_DIR}/extlib/gtec/gUSBamp )
ENDIF( BORLAND )
IF( MSVC )
SET( LIBDIR_EXTLIB_GUSBAMP ${BCI2000_SRC_DIR}/extlib/gtec/gUSBamp/coff )
ENDIF( MSVC )
IF( MINGW )
SET( LIBDIR_EXTLIB_GUSBAMP ${BCI2000_SRC_DIR}/extlib/gtec/gUSBamp/mingw )
ENDIF( MINGW )

# Set Libs required
IF( MINGW )
SET( LIBS_EXTLIB_GUSBAMP libgUSBamp.a )
ELSE( MINGW )
SET( LIBS_EXTLIB_GUSBAMP gUSBamp.lib )
ENDIF( MINGW )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\gtec\\gUSBamp FILES ${SRC_EXTLIB_GUSBAMP} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\gtec\\gUSBamp FILES ${HDR_EXTLIB_GUSBAMP} )

# Set success
SET( GUSBAMP_OK TRUE )

ELSE( WIN32 )

  MESSAGE( "- WARNING: gUSBamp libraries only exist for windows.  This module will not build." )
  SET( GUSBAMP_OK FALSE )

ENDIF( WIN32 )
