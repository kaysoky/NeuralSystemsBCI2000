###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the Microsoft Speech
## API in a project.
## SETS:
##       SRC_EXTLIB_SAPI - Required source files for SAPI 5.1
##       HDR_EXTLIB_SAPI - Required header files for SAPI 5.1
##       INC_EXTLIB_SAPI - Include directory for SAPI 5.1
##       LIBDIR_EXTLIB_SAPI - Library directory for SAPI 5.1
##       LIBS_EXTLIB_SAPI - required library for SAPI 5.1
##       Also defines source groups for SAPI

IF( WIN32 )

# Define the source files
SET( SRC_EXTLIB_SAPI
  ${BCI2000_SRC_DIR}/extlib/SAPILib/sapi.cpp
)

# Define the headers
SET( HDR_EXTLIB_SAPI
  ${BCI2000_SRC_DIR}/extlib/SAPILib/sapi.h
)

# Define the include directory
SET( INC_EXTLIB_SAPI ${BCI2000_SRC_DIR}/extlib/SAPILib )

# Define where the library is
IF( BORLAND )
SET( LIBDIR_EXTLIB_SAPI ${BCI2000_SRC_DIR}/extlib/SAPILib )
ELSE( BORLAND )
IF( MSVC )
SET( LIBDIR_EXTLIB_SAPI ${BCI2000_SRC_DIR}/extlib/SAPILib/lib )
ELSE( MSVC )
SET( LIBDIR_EXTLIB_SAPI ${BCI2000_SRC_DIR}/extlib/SAPILib/lib )
ENDIF( MSVC )
ENDIF( BORLAND )

# Set Libs required
SET( LIBS_EXTLIB_SAPI sapi.lib )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\SAPILib FILES ${SRC_EXTLIB_SAPI} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\SAPILib FILES ${HDR_EXTLIB_SAPI} )

# Set success
SET( SAPI_OK TRUE )

ELSE( WIN32 )

  MESSAGE( "- WARNING: Sapi is only supported on windows.  Building without TTS Support." )
  SET( SAPI_OK FALSE )

ENDIF( WIN32 )
