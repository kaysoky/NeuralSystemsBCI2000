###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for the MFCDemo

IF( BUILD_DEMOS )

# This being MFC, this app will only build in MSVC.
IF( MSVC )

# Set the executable name
SET( EXECUTABLE_NAME MFCDemo )

# Set the project specific sources
SET( SRC_PROJECT
  MFCdemo.cpp
  MFCdemoDlg.cpp
  WINMAIN.cpp
  TaskFilter.cpp 
  StdAfx.cpp
  resource.h
  MFCdemo.rc
)

# Set this up as an AFX project
ADD_DEFINITIONS( -D_AFXDLL )
SET( CMAKE_MFC_FLAG 2 )

# Switch to dynamic linking
SET( USE_PRECOMPILED_HEADERS OFF )
FOREACH( flag_var ${FLAG_VARS} )
   SET( ${flag_var} "${${flag_var}_DYNAMIC}" )
ENDFOREACH( flag_var )

# Include the required framework
SET( LIBS_ ${LIBS} )
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )
SET( LIBS ${LIBS_} )
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreLib/CoreLibFiles.cmake )

# Setup Global Application Module Contents
INCLUDE( ${BCI2000_CMAKE_DIR}/ApplicationCommons.cmake )

# Add the global and specific application files into the framework for this project
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${SRC_APPLICATION}
  ${BCI2000_APPSOURCES}
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream_module.cpp
)
SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${HDR_APPLICATION}
  ${BCI2000_APPHEADERS}
)

BCI2000_ADD_TARGET( INFO "MFC demo application module" WINAPP ${EXECUTABLE_NAME} ${SRC_PROJECT} )

IF( NOT FAILED )

  BCI2000_ADD_TO_INVENTORY( Application ${EXECUTABLE_NAME} )
  SET_PROPERTY( TARGET ${EXECUTABLE_NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=3" )
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} winmm )

  # Set the project build folder
  SET_PROPERTY( TARGET ${EXECUTABLE_NAME} PROPERTY FOLDER Demos/MFC )

ENDIF( NOT FAILED )

ENDIF( MSVC )

ENDIF( BUILD_DEMOS )
