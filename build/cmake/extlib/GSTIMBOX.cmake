###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including the gSTIMbox API
## SETS:
##       SRC_EXTLIB - Required source files for gSTIMbox
##       HDR_EXTLIB - Required header files for gSTIMbox
##       INC_EXTLIB - Include directory for gSTIMbox
##       LIBDIR_EXTLIB - Library directory for gSTIMbox
##       LIBS_EXTLIB - required library for gSTIMbox
##       Also defines source groups for source files

IF( WIN32 )

# Set the Source and headers
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/gtec/gSTIMbox/gSTIMbox.imports.cpp
)
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/gtec/gSTIMbox/gSTIMbox.h
  ${BCI2000_SRC_DIR}/extlib/gtec/gSTIMbox/gSTIMbox.imports.h
)

# Define the include directory
SET( INC_EXTLIB 
  ${BCI2000_SRC_DIR}/extlib/gtec/gSTIMbox
)

# Define where the library is
IF( MSVC )
  IF( CMAKE_CL_64 )
    SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gSTIMbox/coff64 )
  ELSE()
    SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gSTIMbox/coff )
  ENDIF()
ENDIF( MSVC )
IF( MINGW )
  SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/gtec/gSTIMbox/mingw )
ENDIF( MINGW )

# Set Libs required
IF( DYNAMIC_IMPORTS )
ELSEIF( MINGW )
  SET( LIBS_EXTLIB libgSTIMbox.a )
ELSE()
  SET( LIBS_EXTLIB gSTIMbox.lib )
ENDIF()

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\gtec\\gSTIMbox FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\gtec\\gSTIMbox FILES ${HDR_EXTLIB} )

# Set success
SET( EXTLIB_OK TRUE )

ELSE()

  MESSAGE( "- WARNING: gSTIMbox requires Windows.  This module will not build." )
  SET( EXTLIB_OK FALSE )

ENDIF()
