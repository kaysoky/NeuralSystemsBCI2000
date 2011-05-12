###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for testing and build utilities

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build/buildutils/tests )
ADD_EXECUTABLE( bci_datadiff bci_datadiff.cpp )

ADD_SUBDIRECTORY( ParamGenerator )
ADD_SUBDIRECTORY( BootstrapCustomProjects )

MESSAGE( "-- Added Build Testing" )
