###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up a module independent BCI2000 Framework of source
##              files and include directories

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/BasicFramework.cmake )

# Define include directories
IF( BORLAND )
 INCLUDE_DIRECTORIES( ${VXLCORE_INCLUDE_DIR} )
ELSE( BORLAND )
 INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} )
ENDIF( BORLAND )

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

SET( LIBS ${LIBS} AppModuleFramework )

# Notify macro we need some extlib dependencies
BCI2000_USE( "SAPI" )
BCI2000_USE( "DSOUND" )

