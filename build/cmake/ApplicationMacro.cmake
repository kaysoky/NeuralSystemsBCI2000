###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating an application module

MACRO( BCI2000_ADD_APPLICATION_MODULE )
  UTILS_PARSE_ARGS( "NAME;SOURCES" ${ARGV} )

  INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/AppModule.cmake )
  BCI2000_ADD_CORE_MAIN( ${NAME} ${SOURCES} )
  SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" )
  BCI2000_ADD_TARGET( INFO "Application module" GUIAPP ${NAME} ${SOURCES} )
  IF( NOT FAILED )
    BCI2000_ADD_TO_INVENTORY( Application ${NAME} )
    BCI2000_ADD_BCITEST( ${NAME} )
  ENDIF()

ENDMACRO()
