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

IF( (USE_DYNAMIC_IMPORTS AND WIN32) OR (WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8) )

ADD_DEFINITIONS( -DGMOBILABPLUS )

# Set the final Source and headers
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus/gMOBIlabplus.imports.cpp
)

SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus/gMOBIlabplus.h
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus/gMOBIlabplus.imports.h
)

# Define the include directory
SET( INC_EXTLIB 
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus 
)

# Define where the library is
IF( BORLAND )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus/omf )
ENDIF( BORLAND )
IF( MSVC )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus/coff )
ENDIF( MSVC )
IF( MINGW )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlabPlus/mingw )
ENDIF( MINGW )

# Set Libs required
IF( USE_DYNAMIC_IMPORTS )
ELSEIF( MINGW )
SET( LIBS_EXTLIB libgMOBIlabplus.a )
ELSE()
SET( LIBS_EXTLIB gMOBIlabplus.lib )
ENDIF()

# Set success
SET( EXTLIB_OK TRUE )

ELSE()

  MESSAGE( "- WARNING: gMOBIlabPlus requires Windows.  This module will not build." )
  SET( EXTLIB_OK FALSE )

ENDIF()
