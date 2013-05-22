###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including the vAmp API
## SETS:
##       SRC_EXTLIB - Required source files for vAmp
##       HDR_EXTLIB - Required header files for vAmp
##       INC_EXTLIB - Include directory for vAmp
##       LIBDIR_EXTLIB - Library directory for vAmp
##       LIBS_EXTLIB - required library for vAmp

IF( USE_DYNAMIC_IMPORTS OR (WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8) )

# Set the Source and headers
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp/FirstAmp.imports.cpp
)
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp/FirstAmp.h
  ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp/FirstAmp.imports.h
)

# Define the include directory
SET( INC_EXTLIB 
  ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp
)

# Define where the library is
IF( BORLAND )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp )
ENDIF( BORLAND )
IF( MSVC )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp/coff )
ENDIF( MSVC )
IF( MINGW )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp/mingw )
ENDIF( MINGW )

# Set Libs required
IF( USE_DYNAMIC_IMPORTS )
ELSEIF( MINGW )
SET( LIBS_EXTLIB libFirstAmp.a )
ELSE()
SET( LIBS_EXTLIB FirstAmp.lib )
ENDIF()

# Set success
SET( EXTLIB_OK TRUE )

ELSE()

  MESSAGE( "- WARNING: vAmp/FirstAmp libraries only exist for windows 32bit.  This module will not build." )
  SET( EXTLIB_OK FALSE )

ENDIF()
