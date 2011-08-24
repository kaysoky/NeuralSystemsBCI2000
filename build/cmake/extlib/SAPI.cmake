###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the Microsoft Speech
## API in a project.
## SETS:
##       SRC_EXTLIB - Required source files for SAPI 5.1
##       HDR_EXTLIB - Required header files for SAPI 5.1
##       INC_EXTLIB - Include directory for SAPI 5.1
##       LIBDIR_EXTLIB - Library directory for SAPI 5.1
##       LIBS_EXTLIB - required library for SAPI 5.1
##       Also defines source groups for SAPI

IF( WIN32 )

# Define the source files
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/SAPILib/sapi.cpp
)

# Define the headers
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/SAPILib/sapi.h
)

# Define the include directory
SET( INC_EXTLIB ${BCI2000_SRC_DIR}/extlib/SAPILib )

# Define where the library is
IF( BORLAND )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/SAPILib )
ELSE( BORLAND )
IF( MSVC )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/SAPILib/lib )
ELSE( MSVC )
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/SAPILib/lib )
ENDIF( MSVC )
ENDIF( BORLAND )

# Set Libs required
SET( LIBS_EXTLIB sapi.lib )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\SAPILib FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\SAPILib FILES ${HDR_EXTLIB} )

# Set success
SET( EXTLIB_OK TRUE )

ELSE( WIN32 )

  MESSAGE( "- WARNING: Sapi is only supported on windows.  Building without TTS Support." )
  SET( EXTLIB_OK FALSE )

ENDIF( WIN32 )
