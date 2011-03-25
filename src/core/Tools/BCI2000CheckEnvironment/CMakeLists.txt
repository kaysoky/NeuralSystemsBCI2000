###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000CheckEnvironment

# Set the executable name
SET( EXECUTABLE_NAME BCI2000CheckEnvironment )

IF( WIN32 )

# DEBUG
MESSAGE( "-- Adding Tool Project: " ${EXECUTABLE_NAME} )

# Set the project specific sources
SET( SOURCES
  main.cpp 
  MSVCRuntime.cpp
)
SET( HEADERS
  Globals.h
  MSVCRuntime.h
)


ADD_EXECUTABLE( ${EXECUTABLE_NAME} WIN32 ${SOURCES} ${HEADERS} )

IF( MSVC )
  SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES 
    PREFIX "../"
    IMPORT_PREFIX "../" 
  )
  # Make sure the program is built statically against the MSVC runtime
  FOREACH( flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    IF( ${flag_var} MATCHES "/MD" )
      STRING( REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}" )
    ENDIF( ${flag_var} MATCHES "/MD" )
  ENDFOREACH( flag_var )
  SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES
    LINK_FLAGS "/NODEFAULTLIB:msvcrt"
  )
ENDIF( MSVC )

SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/prog )
SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/prog )
SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/prog )

ENDIF( WIN32 )