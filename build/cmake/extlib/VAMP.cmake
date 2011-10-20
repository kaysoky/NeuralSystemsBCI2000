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
##       Also defines source groups for source files

IF( WIN32 AND NOT CMAKE_CL_64 )

# Set the Source and headers
SET( SRC_EXTLIB )
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/brainproducts/vamp/FirstAmp.h
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
IF( MINGW )
SET( LIBS_EXTLIB libFirstAmp.a )
ELSE( MINGW )
SET( LIBS_EXTLIB FirstAmp.lib )
ENDIF( MINGW )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\brainproducts\\vamp FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\brainproducts\\vamp FILES ${HDR_EXTLIB} )

# Set success
SET( EXTLIB_OK TRUE )

ELSE()

  MESSAGE( "- WARNING: vAmp/FirstAmp libraries only exist for windows 32bit.  This module will not build." )
  SET( EXTLIB_OK FALSE )

ENDIF()
