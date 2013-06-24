###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating a signal source module

MACRO( BCI2000_ADD_SIGNAL_SOURCE_MODULE )
  UTILS_PARSE_ARGS( "NAME;SOURCES" ${ARGV} )

  INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/SigSrcModule.cmake )
  BCI2000_ADD_CORE_MAIN( ${NAME} ${SOURCES} )
  SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" )
  BCI2000_ADD_TARGET( INFO "Signal source module" GUIAPP ${NAME} ${SOURCES} )
  IF( NOT FAILED )
    BCI2000_ADD_TO_INVENTORY( SignalSource ${NAME} )
    BCI2000_ADD_BCITEST( ${NAME} )
  ENDIF()

ENDMACRO()
