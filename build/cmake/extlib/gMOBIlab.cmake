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

IF( WIN32 )

# Define the source files
SET( SRC_EXTLIB_GTEC
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabThread.cpp
)

# Define the headers
SET( HDR_EXTLIB_GTEC
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabThread.h
)
SET( HDR_EXTLIB_GTEC_GMOBILAB
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlab/spa20a.h
)

# Set the final Source and headers
SET( SRC_EXTLIB
  ${SRC_EXTLIB_GTEC}
)
SET( HDR_EXTLIB
  ${HDR_EXTLIB_GTEC}
  ${HDR_EXTLIB_GTEC_GMOBILAB}
)

# Define the include directory
SET( INC_EXTLIB 
  ${BCI2000_SRC_DIR}/extlib/gtec
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlab 
)

# Define where the library is
IF( BORLAND )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlab )
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
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\gtec FILES ${SRC_EXTLIB_GTEC} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\gtec FILES ${HDR_EXTLIB_GTEC} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\gtec\\gMOBIlab FILES ${HDR_EXTLIB_GTEC_GMOBILAB} )

# Set success
SET( EXTLIB_OK TRUE )

ELSE( WIN32 )

  MESSAGE( "- WARNING: gMOBIlab libraries only exist for windows.  This module will not build." )
  SET( EXTLIB_OK FALSE )

ENDIF( WIN32 )
