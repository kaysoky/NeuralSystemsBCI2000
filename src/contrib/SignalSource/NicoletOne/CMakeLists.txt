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

# DEBUG
MESSAGE( "-- Adding Signal Source Project: " ${EXECUTABLE_NAME} )

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

# Switch to dynamic linking
FOREACH( flag_var ${FLAG_VARS} )
   SET( ${flag_var} "${${flag_var}_DYNAMIC}" )
ENDFOREACH( flag_var )

# We need to define wchar_t as not defined by the compiler.
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-" )

# Set this up as an MFC Project
ADD_DEFINITIONS( -D_AFXDLL )
SET( CMAKE_MFC_FLAG 2 )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/MinimalFramework.cmake )

SET( BCI2000_SIGSRCSOURCES
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSEvent.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_module.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/GenericFileWriter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/AlignmentFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/DataIOFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/TransmissionFilter.cpp
)

SET( BCI2000_SIGSRCHEADERS
  ${BCI2000_SRC_DIR}/shared/fileio/GenericFileWriter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/AlignmentFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/DataIOFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/TransmissionFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.h
  ${BCI2000_SRC_DIR}/shared/utils/OSEvent.h
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.h
)

# Duplicate the content of cmake/SignalSourceModules.cmake to avoid 
# the MFC-incompatible Webcam logger
IF( WIN32 )
	
  SET( BCI2000_SIGSRCSOURCES
    ${BCI2000_SIGSRCSOURCES}
    ${BCI2000_SRC_DIR}/shared/modules/signalsource/JoystickLogger.cpp
    ${BCI2000_SRC_DIR}/shared/modules/signalsource/KeyLogger.cpp
  )
  SET( BCI2000_SIGSRCHEADERS
    ${BCI2000_SIGSRCHEADERS}
    ${BCI2000_SRC_DIR}/shared/modules/signalsource/JoystickLogger.h
    ${BCI2000_SRC_DIR}/shared/modules/signalsource/KeyLogger.h
  )
ENDIF( WIN32 )

# Add the required signal processing files into the framework for this project
SET( SRC_BCI2000_FRAMEWORK "${SRC_BCI2000_FRAMEWORK}" "${BCI2000_SIGSRCSOURCES}" )
SET( HDR_BCI2000_FRAMEWORK "${HDR_BCI2000_FRAMEWORK}" "${BCI2000_SIGSRCHEADERS}" )

SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\signalsource FILES ${BCI2000_SIGSRCSOURCES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\signalsource FILES ${BCI2000_SIGSRCHEADERS} )

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SRC_PROJECT} )
SOURCE_GROUP( Headers\\Project FILES ${HDR_PROJECT} )

# Setup Extlib Dependencies
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS )

# Add to our include directories
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/shared/modules/signalsource ${INC_EXTLIB} ${BCI2000_SIGSRCINCDIRS} )

# Add link directories for signal source modules
LINK_DIRECTORIES( ${BCI2000_SIGSRCLIBDIRS} )

# Add the executable to the project
ADD_EXECUTABLE( ${EXECUTABLE_NAME} WIN32 ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SRC_PROJECT} ${HDR_PROJECT} )

# Add Pre-processor defines
SET_PROPERTY( TARGET ${EXECUTABLE_NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=1" )

# Set the output directories
SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/prog )
SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/prog )
SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/prog )
IF( MSVC OR XCODE )
  SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES 
    PREFIX "../"
    IMPORT_PREFIX "../" 
  )
ENDIF( MSVC OR XCODE )

TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${LIBS} ${BCI2000_SIGSRCLIBS} )

# Link the Neuroscan Library
TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} debug NicoletSDKd optimized NicoletSDK  )

ENDIF( MSVC )


