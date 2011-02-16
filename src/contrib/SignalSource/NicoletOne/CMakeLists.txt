###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for the Viasys NicoletOne Amp
##              This module 

# Unfortunately, this lib being COM based, we can't do this with ANYTHING
# but Visual Studio.  Bleh.
IF( MSVC )

# Set the executable name
SET( EXECUTABLE_NAME NicoletOne )

# Set the project specific sources
SET( SRC_PROJECT
  NicoletOne.cpp
  WINMAIN.cpp
  LiveData.cpp
  SocketImplDR.cpp
  NicoletOneADC.cpp
  StdAfx.cpp
  NicoletThread.cpp
  Lock.cpp
)
SET( HDR_PROJECT
  NicoletOne.h
  StdAfx.h
  LiveData.h
  SocketImplDR.h
  NicoletOneADC.h
  resource.h
  NicoletThread.h
  Lock.h
)

# Define the library
LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NicoletOne/lib/Libraries )
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NicoletOne/lib/Include )

# We need to define wchar_t as not defined by the compiler.
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-" )

# Set this up as an MFC Project
ADD_DEFINITIONS( -D_AFXDLL )
SET( CMAKE_MFC_FLAG 2 )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${INC_EXTLIB}"
)

# Link the Neuroscan Library
TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} debug NicoletSDKd optimized NicoletSDK  )

ENDIF( MSVC )


