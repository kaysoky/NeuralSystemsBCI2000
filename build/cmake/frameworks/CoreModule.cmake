###########################################################################
## $Id: CoreModuleFiles.cmake -1   $
## juergen.mellinger@uni-tuebingen.de
## Description: Includes common to all Core Modules

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )

IF( NOT BORLAND )
  INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} )
ENDIF( NOT BORLAND )
