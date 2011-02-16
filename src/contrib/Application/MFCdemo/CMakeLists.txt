###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for the MFCDemo

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
)
SET( HDR_PROJECT
  MFCdemo.h
  MFCdemoDlg.h
  StdAfx.h
  TaskFilter.h
  resource.h
)

# Set Application Modules Here.
SET( SRC_APPLICATION 

)
SET( HDR_APPLICATION 

)

# Define extra include directories
SET( INCLUDES 

)

# Set this up as an AFX project
ADD_DEFINITIONS( -D_AFXDLL )
SET( CMAKE_MFC_FLAG 2 )

# Create the signal source module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_APPLICATION}" "${HDR_APPLICATION}"
  "${INCLUDES}"
)

ENDIF( MSVC )


