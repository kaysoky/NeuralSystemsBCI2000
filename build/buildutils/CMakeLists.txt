###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for testing and build utilities

SET( DIR_NAME "${DIR_NAME}Build/BuildUtils/" )
SET( USE_PRECOMPILED_HEADERS OFF )
ADD_DEFINITIONS( -DDISABLE_BCITEST )

OPTION( BUILD_ALL_BUILDUTILS "Enables building of additional build utils" OFF )
IF( BUILD_ALL_BUILDUTILS )
  ADD_SUBDIRECTORY( ParamGenerator )
ENDIF()
ADD_SUBDIRECTORY( BootstrapCustomProjects )

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build/buildutils/tests )
BCI2000_ADD_TARGET( EXECUTABLE bci_datadiff bci_datadiff.cpp )

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build/buildutils )

# extract_registry
INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/shared/config
)
BCI2000_ADD_TARGET( EXECUTABLE extract_registry extract_registry.cpp )

# create_resource
BCI2000_ADD_TARGET( EXECUTABLE create_resource
  create_resource.cpp
  ${BCI2000_SRC_DIR}/shared/types/Blob.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIException.cpp
  ${BCI2000_SRC_DIR}/shared/utils/FileUtils.cpp
  ${BCI2000_SRC_DIR}/shared/utils/StringUtils.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Debugging.cpp
  ${BCI2000_SRC_DIR}/shared/utils/ClassName.cpp
)

IF( BUILD_ALL_BUILDUTILS )
  BCI2000_ADD_TARGET( EXECUTABLE fixup_msvc_lib fixup_msvc_lib.cpp )
  BCI2000_ADD_TARGET( EXECUTABLE coff_set_debug coff_set_debug.cpp )
  BCI2000_ADD_TARGET( EXECUTABLE cpp_demangle cpp_demangle.cpp )
  INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreModule.cmake )
  BCI2000_ADD_TARGET( EXECUTABLE dylib_exports dylib_exports.cpp )
  TARGET_LINK_LIBRARIES( dylib_exports ${LIBS} )
ENDIF()

MESSAGE( "-- Added Build Utils" )
