###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the gUSBamp API
## SETS:
##       SRC_EXTLIB - Required source files for gUSBamp
##       HDR_EXTLIB - Required header files for gUSBamp
##       INC_EXTLIB - Include directory for gUSBamp
##       LIBDIR_EXTLIB - Library directory for gUSBamp
##       LIBS_EXTLIB - required library for gUSBamp
##       Also defines source groups for source files

IF( WIN32 )

# Set the Source and headers
SET( SRC_EXTLIB )
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/gtec/gUSBamp/gUSBamp.h
)

# Define the include directory
SET( INC_EXTLIB 
  ${BCI2000_SRC_DIR}/extlib/gtec/gUSBamp
)

# Define where the library is
IF( BORLAND )
  SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gUSBamp/omf )
ENDIF( BORLAND )
IF( MSVC )
  IF( CMAKE_CL_64 )
    SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gUSBamp/coff64 )
  ELSE()
    SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gUSBamp/coff )
  ENDIF()
ENDIF( MSVC )
IF( MINGW )
  SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gUSBamp/mingw )
ENDIF( MINGW )

# Set Libs required
IF( MINGW )
  SET( LIBS_EXTLIB libgUSBamp.a )
ELSE( MINGW )
  SET( LIBS_EXTLIB gUSBamp.lib )
ENDIF( MINGW )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\gtec\\gUSBamp FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\gtec\\gUSBamp FILES ${HDR_EXTLIB} )

# Set success
SET( EXTLIB_OK TRUE )

ELSE( WIN32 )

  MESSAGE( "- WARNING: gUSBamp libraries only exist for windows.  This module will not build." )
  SET( EXTLIB_OK FALSE )

ENDIF( WIN32 )
