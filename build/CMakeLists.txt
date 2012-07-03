###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

cmake_minimum_required( VERSION 2.8.2 )
cmake_policy( VERSION 2.8 )

# Define the project name
PROJECT( BCI2000 )

# Set the variable where to find the top level project sources
IF ( NOT DEFINED BCI2000_ROOT_DIR )
    SET ( BCI2000_ROOT_DIR ${BCI2000_SOURCE_DIR}/.. )
ENDIF( NOT DEFINED BCI2000_ROOT_DIR )

# Set directory paths
SET( BCI2000_SRC_DIR ${BCI2000_ROOT_DIR}/src )
SET( BCI2000_CMAKE_DIR ${BCI2000_SOURCE_DIR}/cmake )
SET( CMAKE_INCLUDE_CURRENT_DIR ON )

# Find required dependencies
INCLUDE( ${BCI2000_CMAKE_DIR}/FindDependencies.cmake )

# Setup required cmake macros
INCLUDE( ${BCI2000_CMAKE_DIR}/SetupBCI2000Macros.cmake )

# Setup build configurations
INCLUDE( ${BCI2000_CMAKE_DIR}/BuildConfigurations.cmake )

# Setup version info
SET( BCI2000_VER_MAJOR "3" )
SET( BCI2000_VER_MINOR "0" )
SET( BCI2000_VER_PATCH "5" )
SET( BCI2000_VERSION "${BCI2000_VER_MAJOR}.${BCI2000_VER_MINOR}.${BCI2000_VER_PATCH}" )
ADD_DEFINITIONS(
  -DBCI2000_VERSION_STRING="${BCI2000_VERSION}"
  )

# Tell the user what we're about to setup
MESSAGE( "-- Creating Project files for BCI2000 Version " ${BCI2000_VERSION} )

INCLUDE( ${BCI2000_CMAKE_DIR}/SignalSourceCommons.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/ApplicationCommons.cmake )
INCLUDE( "${BCI2000_SRC_DIR}/contrib/Extensions/Extensions.cmake" )
IF ( EXISTS ${BCI2000_SRC_DIR}/private AND NOT "$ENV{BCI2000_NO_PRIVATE}" )
  INCLUDE( "${BCI2000_SRC_DIR}/private/PrivateConfig.cmake" )
ENDIF ( EXISTS ${BCI2000_SRC_DIR}/private AND NOT "$ENV{BCI2000_NO_PRIVATE}" )
ADD_SUBDIRECTORY( ${BCI2000_CMAKE_DIR}/frameworks "${CMAKE_CURRENT_BINARY_DIR}/frameworks" )
ADD_SUBDIRECTORY( ${BCI2000_CMAKE_DIR}/extlib "${CMAKE_CURRENT_BINARY_DIR}/extlib" )
ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/shared/utils/Expression/test "${CMAKE_CURRENT_BINARY_DIR}/shared" )

# Recurse down into all project subdirectories

# Set output for all build types to the prog folder.
# This comes after static libraries have been created, so these 
# maintain build-type specific output directories
# (to avoid errors from linking to libraries with different build type)
SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" )

# src/private : stuff we use that is not released yet
IF ( EXISTS ${BCI2000_SRC_DIR}/private AND NOT "$ENV{BCI2000_NO_PRIVATE}" )
  ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/private "${CMAKE_CURRENT_BINARY_DIR}/private" )
ENDIF( EXISTS ${BCI2000_SRC_DIR}/private AND NOT "$ENV{BCI2000_NO_PRIVATE}" )

# the standard distro: build/buildutils, src/core and src/contrib
ADD_SUBDIRECTORY( buildutils "${CMAKE_CURRENT_BINARY_DIR}/buildtesting")
ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/core "${CMAKE_CURRENT_BINARY_DIR}/core" )
ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/contrib "${CMAKE_CURRENT_BINARY_DIR}/contrib" )

# src/custom : a convenient place for users' own development without causing svn chaos 
IF ( EXISTS ${BCI2000_SRC_DIR}/custom ) 
  ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/custom "${CMAKE_CURRENT_BINARY_DIR}/custom" )
ENDIF( EXISTS ${BCI2000_SRC_DIR}/custom )


INCLUDE( ${BCI2000_CMAKE_DIR}/BCI2000Package.cmake )

MARK_AS_ADVANCED( CMAKE_INSTALL_PREFIX )


IF( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE )
ENDIF( NOT CMAKE_BUILD_TYPE )
MARK_AS_ADVANCED( CMAKE_BUILD_TYPE )
