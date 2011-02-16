###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the gMOBIlabPlus API
## SETS:
##       SRC_EXTLIB_GMOBILABPLUS - Required source files for gMOBIlabPlus
##       HDR_EXTLIB_GMOBILABPLUS - Required header files for gMOBIlabPlus
##       INC_EXTLIB_GMOBILABPLUS - Include directory for gMOBIlabPlus
##       LIBDIR_EXTLIB_GMOBILABPLUS - Library directory for gMOBIlabPlus
##       LIBS_EXTLIB_GMOBILABPLUS - required library for gMOBIlabPlus
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
SET( HDR_EXTLIB_GTEC_GMOBILABPLUS
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus/gMOBIlabplus.h
)

# Set the final Source and headers
SET( SRC_EXTLIB_GMOBILABPLUS
  ${SRC_EXTLIB_GTEC}
)
SET( HDR_EXTLIB_GMOBILABPLUS
  ${HDR_EXTLIB_GTEC}
  ${HDR_EXTLIB_GTEC_GMOBILABPLUS}
)

# Define the include directory
SET( INC_EXTLIB_GMOBILABPLUS 
  ${BCI2000_SRC_DIR}/extlib/gtec
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus 
)

# Define where the library is
IF( BORLAND )
SET( LIBDIR_EXTLIB_GMOBILABPLUS ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus )
ENDIF( BORLAND )
IF( MSVC )
SET( LIBDIR_EXTLIB_GMOBILABPLUS ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus/coff )
ENDIF( MSVC )
IF( MINGW )
SET( LIBDIR_EXTLIB_GMOBILABPLUS ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus/mingw )
ENDIF( MINGW )

# Set Libs required
IF( MINGW )
SET( LIBS_EXTLIB_GMOBILABPLUS libgMOBIlabplus.a )
ELSE( MINGW )
SET( LIBS_EXTLIB_GMOBILABPLUS gMOBIlabplus.lib )
ENDIF( MINGW )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\gtec FILES ${SRC_EXTLIB_GTEC} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\gtec FILES ${HDR_EXTLIB_GTEC} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\gtec\\gMOBIlabPlus FILES ${HDR_EXTLIB_GTEC_GMOBILABPLUS} )

# Set success
SET( GMOBILABPLUS_OK TRUE )

ELSE( WIN32 )

  MESSAGE( "- WARNING: gMOBIlabPlus libraries only exist for windows.  This module will not build." )
  SET( GMOBILABPLUS_OK FALSE )

ENDIF( WIN32 )
