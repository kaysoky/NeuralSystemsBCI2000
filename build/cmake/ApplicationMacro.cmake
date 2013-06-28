###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating an application module

MACRO( BCI2000_ADD_APPLICATION_MODULE )
  UTILS_PARSE_ARGS( "NAME;SOURCES" ${ARGV} )

  UTILS_INCLUDE( frameworks/AppModule )
  ADD_DEFINITIONS( -DMODTYPE=3 )
  BCI2000_ADD_CORE_MAIN( ${NAME} ${SOURCES} )
  BCI2000_ADD_TARGET( INFO "Application module" GUIAPP ${NAME} ${SOURCES} )
  IF( NOT FAILED )
    BCI2000_ADD_TO_INVENTORY( Application ${NAME} )
    BCI2000_ADD_BCITEST( ${NAME} )
  ENDIF()

ENDMACRO()
