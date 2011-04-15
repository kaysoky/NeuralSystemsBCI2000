###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for testing and build utilities

ADD_EXECUTABLE( bci_datadiff bci_datadiff.cpp )
SET_TARGET_PROPERTIES( bci_datadiff PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/build/buildutils/tests )
IF (MSVC OR XCODE)
   SET_TARGET_PROPERTIES( bci_datadiff PROPERTIES PREFIX "../" )
ENDIF(MSVC OR XCODE)

ADD_SUBDIRECTORY( ParamGenerator )
ADD_SUBDIRECTORY( BootstrapCustomProjects )

MESSAGE( "-- Added Build Testing" )
