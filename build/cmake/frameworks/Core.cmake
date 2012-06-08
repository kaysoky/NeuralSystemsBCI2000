###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Usage header for BCI2000FrameworkCore library

# Define include directories
IF( BORLAND )
  INCLUDE_DIRECTORIES( ${VXLCORE_INCLUDE_DIR} )
ENDIF( BORLAND )

INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/shared
  ${BCI2000_SRC_DIR}/shared/accessors
  ${BCI2000_SRC_DIR}/shared/bcistream
  ${BCI2000_SRC_DIR}/shared/config
  ${BCI2000_SRC_DIR}/shared/modules
  ${BCI2000_SRC_DIR}/shared/filters
  ${BCI2000_SRC_DIR}/shared/types
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/shared/utils/Expression
  ${BCI2000_SRC_DIR}/shared/fileio
  ${BCI2000_SRC_DIR}/shared/fileio/dat
)

SET( LIBS ${LIBS} BCI2000FrameworkCore )
