###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating a GUI based tool application

MACRO( BCI2000_ADD_TOOLS_GUIAPP ) 
  UTILS_PARSE_ARGS( "NAME;SOURCES" ${ARGV} )

  SET( SOURCES
    ${SOURCES}
    ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream_guiapp.cpp
    ${BCI2000_SRC_DIR}/shared/gui/AboutBox.cpp
    ${BCI2000_SRC_DIR}/shared/gui/ExecutableHelp.cpp
    ${BCI2000_SRC_DIR}/shared/utils/Settings.cpp
    ${BCI2000_SRC_DIR}/shared/gui/ColorListChooser.cpp
  )

  INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )
  SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/tools/${NAME} )
  BCI2000_ADD_TARGET( INFO Tool GUIAPP ${NAME} ${SOURCES} )
  BCI2000_ADD_TO_INVENTORY( Tool ${NAME} )

ENDMACRO()
