###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for the Viasys NicoletOne Amp
##              This module 

# Unfortunately, this lib being COM based, we can't do this with ANYTHING
# but Visual Studio.  Bleh.
IF( MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4 )

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
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/GenericFileWriter.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream_module.cpp
)
SET( HDR_PROJECT
  resource.h
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/GenericADC.h
)

# Define the library
LINK_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NicoletOne/lib/Libraries )
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/NicoletOne/lib/Include )

# Switch to dynamic linking
FOREACH( flag_var ${FLAG_VARS} )
   SET( ${flag_var} "${${flag_var}_DYNAMIC}" )
ENDFOREACH( flag_var )
SET( USE_PRECOMPILED_HEADERS OFF )

# We need to define wchar_t as not defined by the compiler.
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-" )

# Set this up as an MFC Project
ADD_DEFINITIONS( -D_AFXDLL )
SET( CMAKE_MFC_FLAG 2 )

SET( LIBS_ ${LIBS} )
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )
SET( LIBS ${LIBS_} )
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreLib/CoreLibFiles.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/SignalSourceCommons.cmake )

LINK_DIRECTORIES( ${BCI2000_SIGSRCLIBDIRS} )
BCI2000_ADD_TARGET( INFO "Signal source module" WINAPP ${EXECUTABLE_NAME} ${SRC_PROJECT} ${HDR_PROJECT} )

IF( NOT FAILED )
  BCI2000_ADD_TO_INVENTORY( SignalSource ${EXECUTABLE_NAME} )
  SET_PROPERTY( TARGET ${EXECUTABLE_NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=1" )
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${BCI2000_SIGSRCLIBS} )
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} winmm debug NicoletSDKd optimized NicoletSDK  )
ENDIF( NOT FAILED )

ENDIF()
