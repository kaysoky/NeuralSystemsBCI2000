###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

cmake_minimum_required( VERSION 2.8 )
cmake_policy( VERSION 2.8 )

# Set a library searching policy for linking purposes
#IF( COMMAND cmake_policy )
#  cmake_policy( SET CMP0003 NEW )
#ENDIF( COMMAND cmake_policy )

# Define the project name
PROJECT( BCI2000 )

# Set the variable where to find the top level project sources
IF ( NOT DEFINED BCI2000_ROOT_DIR )
    SET ( BCI2000_ROOT_DIR ${BCI2000_SOURCE_DIR}/.. )
ENDIF( NOT DEFINED BCI2000_ROOT_DIR )

# Setup Output Directories
SET ( CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${BCI2000_ROOT_DIR}/prog
  CACHE PATH
  "Single Directory for all Libraries"
)

# Setup Executable Output Directory
SET ( CMAKE_RUNTIME_OUTPUT_DIRECTORY
  ${BCI2000_ROOT_DIR}/prog
  CACHE PATH
  "Single Directory for all Executables."
)

# Setup Static Library Output Directory
SET ( CMAKE_ARCHIVE_OUTPUT_DIRECTORY
  ${BCI2000_ROOT_DIR}/prog
  CACHE PATH
  "Single Directory for all static libraries."
)

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
SET( BCI2000_VER_PATCH "0" )
SET( BCI2000_VERSION "${BCI2000_VER_MAJOR}.${BCI2000_VER_MINOR}.${BCI2000_VER_PATCH}" )
ADD_DEFINITIONS(
  -DBCI2000_VERSION_STRING="${BCI2000_VERSION}"
  )

# Tell the user what we're about to setup
MESSAGE( "-- Creating Project files for BCI2000 Version " ${BCI2000_VERSION} )

# Recurse down into all project subdirectories
IF ( EXISTS ${BCI2000_SRC_DIR}/private )
  INCLUDE( "${BCI2000_SRC_DIR}/private/PrivateConfig.cmake" )
  ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/private "${CMAKE_CURRENT_BINARY_DIR}/private" )
ENDIF( EXISTS ${BCI2000_SRC_DIR}/private )
ADD_SUBDIRECTORY( buildutils "${CMAKE_CURRENT_BINARY_DIR}/buildtesting")
ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/core "${CMAKE_CURRENT_BINARY_DIR}/core" )
ADD_SUBDIRECTORY( ${BCI2000_SRC_DIR}/contrib "${CMAKE_CURRENT_BINARY_DIR}/contrib" )



INCLUDE( ${BCI2000_CMAKE_DIR}/BCI2000Package.cmake )
