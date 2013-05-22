###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Adds the CoreMain.cpp source file to a project if no
##   other main file exists.
##

MACRO( BCI2000_ADD_CORE_MAIN NAME )
  SET( sources ${ARGV} )
  LIST( REMOVE_AT sources 0 )
  LIST( FIND sources "${NAME}.cpp" IDX )
  IF( IDX EQUAL -1 )
    SET( CORE_MAIN ${BCI2000_SRC_DIR}/shared/modules/CoreMain.cpp )
    SET( SRC_BCI2000_FRAMEWORK
      ${SRC_BCI2000_FRAMEWORK}
      ${CORE_MAIN}
    )
  ENDIF()
ENDMACRO()  
