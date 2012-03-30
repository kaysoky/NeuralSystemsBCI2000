###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for testing and build utilities

SET( DIR_NAME "${DIR_NAME}Build/BuildUtils/" )

ADD_SUBDIRECTORY( ParamGenerator )
ADD_SUBDIRECTORY( BootstrapCustomProjects )

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build/buildutils/tests )
ADD_EXECUTABLE( bci_datadiff bci_datadiff.cpp )

# Set the project build folder
SET_PROPERTY( TARGET bci_datadiff PROPERTY FOLDER "${DIR_NAME}" )

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build/buildutils )
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/shared/config )
ADD_EXECUTABLE( extract_registry extract_registry.cpp )

# Set the project build folder
SET_PROPERTY( TARGET extract_registry PROPERTY FOLDER "${DIR_NAME}" )

MESSAGE( "-- Added Build Utils" )
