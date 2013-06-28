###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating a GUI based tool application

MACRO( BCI2000_ADD_TOOLS_GUIAPP ) 
  UTILS_PARSE_ARGS( "NAME;SOURCES" ${ARGV} )

  SET( SOURCES
    ${SOURCES}
    ${PROJECT_SRC_DIR}/shared/bcistream/BCIStream_guiapp.cpp
    ${PROJECT_SRC_DIR}/shared/gui/AboutBox.cpp
    ${PROJECT_SRC_DIR}/shared/gui/ExecutableHelp.cpp
    ${PROJECT_SRC_DIR}/shared/utils/Settings.cpp
    ${PROJECT_SRC_DIR}/shared/gui/ColorListChooser.cpp
  )

  UTILS_INCLUDE( frameworks/Core )
  BCI2000_ADD_TARGET(
    INFO Tool
    GUIAPP ${NAME}
    ${SOURCES}
    OUTPUT_DIRECTORY ${PROJECT_ROOT_DIR}/tools/${NAME}
  )
  BCI2000_ADD_TO_INVENTORY( Tool ${NAME} )

ENDMACRO()
