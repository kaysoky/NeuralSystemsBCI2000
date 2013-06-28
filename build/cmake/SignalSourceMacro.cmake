###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating a signal source module

MACRO( BCI2000_ADD_SIGNAL_SOURCE_MODULE )
  UTILS_PARSE_ARGS( "NAME;SOURCES" ${ARGV} )

  UTILS_INCLUDE( frameworks/SigSrcModule )
  ADD_DEFINITIONS( -DMODTYPE=1 )
  BCI2000_ADD_CORE_MAIN( ${NAME} ${SOURCES} )
  BCI2000_ADD_TARGET( INFO "Signal source module" GUIAPP ${NAME} ${SOURCES} )
  IF( NOT FAILED )
    BCI2000_ADD_TO_INVENTORY( SignalSource ${NAME} )
    BCI2000_ADD_BCITEST( ${NAME} )
  ENDIF()

ENDMACRO()
