###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for testing and build utilities

ADD_SUBDIRECTORY( ParamGenerator )
ADD_SUBDIRECTORY( BootstrapCustomProjects )

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build/buildutils/tests )
ADD_EXECUTABLE( bci_datadiff bci_datadiff.cpp )

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build/buildutils )
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/shared/config )
ADD_EXECUTABLE( extract_registry extract_registry.cpp )

MESSAGE( "-- Added Build Utils" )
