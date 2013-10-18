###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

cmake_minimum_required( VERSION 2.8.10 )
cmake_policy( VERSION 2.8 )
set( CMAKE_INCLUDE_CURRENT_DIR ON )

# Define the project name and version
PROJECT( BCI2000 )
SET( LATEST_RELEASE "3.05" )
SET( LATEST_RELEASE_REVISION "4230" )

GET_FILENAME_COMPONENT( PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE )
SET( PROJECT_SRC_DIR "${PROJECT_ROOT_DIR}/src" )
SET( PROJECT_OUTPUT_DIR "${PROJECT_ROOT_DIR}/prog" )
SET( PROJECT_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles )

INCLUDE( ${PROJECT_ROOT_DIR}/build/cmake/Utils.cmake )
UTILS_INCLUDE( BuildOptions )
UTILS_INCLUDE( FindQt )
UTILS_INCLUDE( SetupBCI2000Macros )
UTILS_INCLUDE( BuildConfigurations )

SET( BCI2000_ROOT_DIR "${PROJECT_ROOT_DIR}" )
SET( BCI2000_SRC_DIR "${PROJECT_SRC_DIR}" )
SET( BCI2000_BINARY_DIR "${PROJECT_BUILD_DIR}" )

# Tell the user what we're about to setup
MESSAGE( STATUS "Configuring project ${PROJECT_NAME} Version ${PROJECT_VERSION}" )

UTILS_INCLUDE( SignalSourceCommons )
UTILS_INCLUDE( ApplicationCommons )
INCLUDE( "${PROJECT_SRC_DIR}/contrib/Extensions/Extensions.cmake" )
IF( EXISTS ${PROJECT_SRC_DIR}/private AND BUILD_PRIVATE )
  INCLUDE( "${PROJECT_SRC_DIR}/private/PrivateConfig.cmake" )
ENDIF()
ADD_SUBDIRECTORY( ${PROJECT_ROOT_DIR}/build/cmake/frameworks "${PROJECT_BUILD_DIR}/frameworks" )
ADD_SUBDIRECTORY( ${PROJECT_ROOT_DIR}/build/cmake/extlib "${PROJECT_BUILD_DIR}/extlib" )
ADD_SUBDIRECTORY( ${PROJECT_SRC_DIR}/shared/utils/Expression/test "${PROJECT_BUILD_DIR}/shared" )

# Recurse down into all project subdirectories

# src/private : stuff we use that is not released yet
IF( EXISTS ${BCI2000_SRC_DIR}/private AND BUILD_PRIVATE )
  ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/private "${BCI2000_BINARY_DIR}/private" )
ENDIF()

# the standard distro: build/buildutils, src/core and src/contrib
ADD_SUBDIRECTORY( buildutils "${BCI2000_BINARY_DIR}/buildtesting")
ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/core "${BCI2000_BINARY_DIR}/core" )
ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/contrib "${BCI2000_BINARY_DIR}/contrib" )

# src/custom : a convenient place for users' own development without causing svn chaos 
IF ( EXISTS ${BCI2000_SRC_DIR}/custom )
  SET( saved_ ${DIR_NAME} )
  SET( DIR_NAME "${DIR_NAME}Custom/" )
  ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/custom "${BCI2000_BINARY_DIR}/custom" )
  SET( DIR_NAME ${saved_} )
ENDIF( EXISTS ${BCI2000_SRC_DIR}/custom )


UTILS_INCLUDE( BCI2000Package )

MARK_AS_ADVANCED( CMAKE_INSTALL_PREFIX )
