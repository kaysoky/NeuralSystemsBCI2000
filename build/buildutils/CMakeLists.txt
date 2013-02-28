###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for testing and build utilities

SET( DIR_NAME "${DIR_NAME}Build/BuildUtils/" )

SET( ALL_BUILDUTILS FALSE CACHE BOOL "Enables building of additional build utils" )
IF( ALL_BUILDUTILS )
  ADD_SUBDIRECTORY( ParamGenerator )
ENDIF()
ADD_SUBDIRECTORY( BootstrapCustomProjects )

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build/buildutils/tests )
ADD_EXECUTABLE( bci_datadiff bci_datadiff.cpp )
SET_PROPERTY( TARGET bci_datadiff PROPERTY FOLDER "${DIR_NAME}" )

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build/buildutils )
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/shared/config )
ADD_EXECUTABLE( extract_registry extract_registry.cpp )
SET_PROPERTY( TARGET extract_registry PROPERTY FOLDER "${DIR_NAME}" )

IF( ALL_BUILDUTILS )
  ADD_EXECUTABLE( fixup_msvc_lib fixup_msvc_lib.cpp )
  SET_PROPERTY( TARGET fixup_msvc_lib PROPERTY FOLDER "${DIR_NAME}" )

  ADD_EXECUTABLE( cpp_demangle cpp_demangle.cpp )
  SET_PROPERTY( TARGET cpp_demangle PROPERTY FOLDER "${DIR_NAME}" )
  IF( WIN32 )
    TARGET_LINK_LIBRARIES( cpp_demangle dbghelp )
  ENDIF()

  INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreModule.cmake )
  ADD_EXECUTABLE( dylib_exports dylib_exports.cpp )
  TARGET_LINK_LIBRARIES( dylib_exports ${LIBS} )
  SET_PROPERTY( TARGET dylib_exports PROPERTY FOLDER "${DIR_NAME}" )
ENDIF()

MESSAGE( "-- Added Build Utils" )
