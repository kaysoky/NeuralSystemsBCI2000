###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the gMOBIlabPlus API
## SETS:
##       SRC_EXTLIB - Required source files for gMOBIlabPlus
##       HDR_EXTLIB - Required header files for gMOBIlabPlus
##       INC_EXTLIB - Include directory for gMOBIlabPlus
##       LIBDIR_EXTLIB - Library directory for gMOBIlabPlus
##       LIBS_EXTLIB - required library for gMOBIlabPlus
##       Also defines source groups for source files

IF( WIN32 AND NOT CMAKE_CL_64 )

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
SET( SRC_EXTLIB
  ${SRC_EXTLIB_GTEC}
)
SET( HDR_EXTLIB
  ${HDR_EXTLIB_GTEC}
  ${HDR_EXTLIB_GTEC_GMOBILABPLUS}
)

# Define the include directory
SET( INC_EXTLIB 
  ${BCI2000_SRC_DIR}/extlib/gtec
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus 
)

# Define where the library is
IF( BORLAND )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus )
ENDIF( BORLAND )
IF( MSVC )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus/coff )
ENDIF( MSVC )
IF( MINGW )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus/mingw )
ENDIF( MINGW )

# Set Libs required
IF( MINGW )
SET( LIBS_EXTLIB libgMOBIlabplus.a )
ELSE( MINGW )
SET( LIBS_EXTLIB gMOBIlabplus.lib )
ENDIF( MINGW )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\gtec FILES ${SRC_EXTLIB_GTEC} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\gtec FILES ${HDR_EXTLIB_GTEC} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\gtec\\gMOBIlabPlus FILES ${HDR_EXTLIB_GTEC_GMOBILABPLUS} )

# Set success
SET( EXTLIB_OK TRUE )

ELSE()

  MESSAGE( "- WARNING: gMOBIlabPlus libraries only exist for windows 32bit.  This module will not build." )
  SET( EXTLIB_OK FALSE )

ENDIF()
