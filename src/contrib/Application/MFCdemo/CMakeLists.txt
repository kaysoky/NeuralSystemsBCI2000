###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for the MFCDemo

# This being MFC, this app will only build in MSVC.
IF( MSVC )

# Set the executable name
SET( EXECUTABLE_NAME MFCDemo )

# DEBUG
MESSAGE( "-- Adding MFC Application Project: " ${EXECUTABLE_NAME} )


# Set the project specific sources
SET( SRC_PROJECT
  MFCdemo.cpp
  MFCdemoDlg.cpp
  WINMAIN.cpp
  TaskFilter.cpp 
  StdAfx.cpp
)
SET( HDR_PROJECT
  MFCdemo.h
  MFCdemoDlg.h
  StdAfx.h
  TaskFilter.h
  resource.h
)
SET( RSRC_PROJECT
  MFCdemo.rc
)

# Set Application components here.
SET( SRC_SHARED_MODULES 
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.cpp
)
SET( SRC_SHARED_BCISTREAM
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_module.cpp
)

SET( HDR_SHARED_MODULES 
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.h
)

SET( SRC_APPLICATION "${SRC_SHARED_MODULES}" "${SRC_SHARED_BCISTREAM}" )
SET( HDR_APPLICATION "${HDR_SHARED_MODULES}" )

# Set Source Groups
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules FILES ${SRC_SHARED_MODULES} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES ${SRC_SHARED_BCISTREAM} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules FILES ${HDR_SHARED_MODULES} )

# Set this up as an AFX project
ADD_DEFINITIONS( -D_AFXDLL )
SET( CMAKE_MFC_FLAG 2 )

# Switch to dynamic linking
FOREACH( flag_var ${FLAG_VARS} )
   SET( ${flag_var} "${${flag_var}_DYNAMIC}" )
ENDFOREACH( flag_var )

# Include the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )

# Setup Global Application Module Contents
INCLUDE( ${BCI2000_CMAKE_DIR}/ApplicationCommons.cmake )

# Add the global and specific application files into the framework for this project
SET( SRC_BCI2000_FRAMEWORK "${SRC_BCI2000_FRAMEWORK}" "${SRC_APPLICATION}" "${BCI2000_APPSOURCES}" )
SET( HDR_BCI2000_FRAMEWORK "${HDR_BCI2000_FRAMEWORK}" "${HDR_APPLICATION}" "${BCI2000_APPHEADERS}" )

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SRC_PROJECT} )
SOURCE_GROUP( Headers\\Project FILES ${HDR_PROJECT} )
SOURCE_GROUP( Resources\\Project FILES ${RSRC_PROJECT} )

# Add in external required libraries
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS )

# Add the executable to the project
ADD_EXECUTABLE( ${EXECUTABLE_NAME} WIN32 ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SRC_PROJECT} ${HDR_PROJECT} ${RSRC_PROJECT} )

# Add Pre-processor defines
SET_PROPERTY( TARGET ${EXECUTABLE_NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=3" )

TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${LIBS} )

ENDIF( MSVC )


