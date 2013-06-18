###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

cmake_minimum_required( VERSION 2.8.2 )
cmake_policy( VERSION 2.8 )

# Define the project name and version
PROJECT( BCI2000 )
SET( PROJECT_VER_MAJOR "3" )
SET( PROJECT_VER_MINOR "0" )
SET( PROJECT_VER_PATCH "5" )

# Set the variable where to find the top level project sources
IF( NOT DEFINED BCI2000_ROOT_DIR )
  SET ( BCI2000_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. )
ENDIF()

# Set directory paths
SET( BCI2000_SRC_DIR ${BCI2000_ROOT_DIR}/src )
SET( BCI2000_CMAKE_DIR ${BCI2000_SOURCE_DIR}/cmake )
SET( BCI2000_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles )
SET( CMAKE_INCLUDE_CURRENT_DIR ON )

INCLUDE( ${BCI2000_CMAKE_DIR}/BuildOptions.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/FindDependencies.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/SetupBCI2000Macros.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/BuildConfigurations.cmake )

# Tell the user what we're about to setup
MESSAGE( STATUS "Creating Project files for ${PROJECT_NAME} Version ${PROJECT_VERSION} Rev ${PROJECT_REVISION}" )

INCLUDE( ${BCI2000_CMAKE_DIR}/SignalSourceCommons.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/ApplicationCommons.cmake )
INCLUDE( "${BCI2000_SRC_DIR}/contrib/Extensions/Extensions.cmake" )
IF( EXISTS ${BCI2000_SRC_DIR}/private AND BUILD_PRIVATE )
  INCLUDE( "${BCI2000_SRC_DIR}/private/PrivateConfig.cmake" )
ENDIF()
ADD_SUBDIRECTORY( ${BCI2000_CMAKE_DIR}/frameworks "${BCI2000_BINARY_DIR}/frameworks" )
ADD_SUBDIRECTORY( ${BCI2000_CMAKE_DIR}/extlib "${BCI2000_BINARY_DIR}/extlib" )
ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/shared/utils/Expression/test "${BCI2000_BINARY_DIR}/shared" )

# Recurse down into all project subdirectories

# Set output for all build types to the prog folder.
# This comes after static libraries have been created, so these 
# maintain build-type specific output directories
# (to avoid errors from linking to libraries with different build type)
SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" )

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


INCLUDE( ${BCI2000_CMAKE_DIR}/BCI2000Package.cmake )

MARK_AS_ADVANCED( CMAKE_INSTALL_PREFIX )
