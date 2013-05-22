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

IF( (USE_DYNAMIC_IMPORTS AND WIN32) OR (WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8) )

# Set the final Source and headers
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlab/spa20a.imports.cpp
)

SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlab/spa20a.h
  ${BCI2000_SRC_DIR}/extlib/gtec/gMOBIlab/spa20a.imports.h
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
IF( USE_DYNAMIC_IMPORTS )
ELSEIF( MINGW )
SET( LIBS_EXTLIB libspa20a.a )
ELSE()
SET( LIBS_EXTLIB spa20a.lib )
ENDIF()

# Set success
SET( EXTLIB_OK TRUE )

ELSE()

  MESSAGE( "- WARNING: gMOBIlab requires Windows.  This module will not build." )
  SET( EXTLIB_OK FALSE )

ENDIF()
