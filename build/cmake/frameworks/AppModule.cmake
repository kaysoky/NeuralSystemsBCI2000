###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Sets up include directories and dependencies for 
##   Application Modules using the AppModule library

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreModule.cmake )

INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/shared
  ${BCI2000_SRC_DIR}/shared/accessors
  ${BCI2000_SRC_DIR}/shared/bcistream
  ${BCI2000_SRC_DIR}/shared/config
  ${BCI2000_SRC_DIR}/shared/modules
  ${BCI2000_SRC_DIR}/shared/types
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/shared/utils/Expression
  ${BCI2000_SRC_DIR}/shared/fileio
  ${BCI2000_SRC_DIR}/shared/gui
  ${BCI2000_SRC_DIR}/shared/modules/application
  ${BCI2000_SRC_DIR}/shared/modules/application/utils
  ${BCI2000_SRC_DIR}/shared/modules/application/audio
  ${BCI2000_SRC_DIR}/shared/modules/application/gui
  ${BCI2000_SRC_DIR}/shared/modules/application/human_interface_devices
  ${BCI2000_SRC_DIR}/shared/modules/application/speller
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli
)

BCI2000_USE( "SAPI" )
BCI2000_USE( "DSOUND" )

IF( MSVC )
  SET( REGISTRY_NAME AppRegistry )
  SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /include:_${REGISTRY_NAME}" )
ENDIF()

SET( LIBS ${LIBS} BCI2000FrameworkAppModule )
