###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the gMOBIlab API
## SETS:
##       SRC_EXTLIB - Required source files for gMOBIlab
##       HDR_EXTLIB - Required header files for gMOBIlab
##       INC_EXTLIB - Include directory for gMOBIlab
##       LIBDIR_EXTLIB - Library directory for gMOBIlab
##       LIBS_EXTLIB - required library for gMOBIlab
##       Also defines source groups for source files

IF( WIN32 AND NOT CMAKE_CL_64 )

# Set the final Source and headers
SET( SRC_EXTLIB
)

SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlab/spa20a.h
)

# Define the include directory
SET( INC_EXTLIB 
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlab 
)

# Define where the library is
IF( BORLAND )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlab/omf )
ENDIF( BORLAND )
IF( MSVC )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlab/coff )
ENDIF( MSVC )
IF( MINGW )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlab/mingw )
ENDIF( MINGW )

# Set Libs required
IF( MINGW )
SET( LIBS_EXTLIB libspa20a.a )
ELSE( MINGW )
SET( LIBS_EXTLIB spa20a.lib )
ENDIF( MINGW )

# Set the source groups
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\gtec\\gMOBIlab FILES ${HDR_EXTLIB} )

# Set success
SET( EXTLIB_OK TRUE )

ELSE()

  MESSAGE( "- WARNING: gMOBIlab libraries only exist for windows 32bit.  This module will not build." )
  SET( EXTLIB_OK FALSE )

ENDIF()
