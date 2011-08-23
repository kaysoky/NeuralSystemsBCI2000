###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Usage header for BasicFramework library

# Define include directories
IF( BORLAND )
INCLUDE_DIRECTORIES(
  ${VXLCORE_INCLUDE_DIR}
  ${BCI2000_SRC_DIR}/shared
  ${BCI2000_SRC_DIR}/shared/accessors
  ${BCI2000_SRC_DIR}/shared/bcistream
  ${BCI2000_SRC_DIR}/shared/config
  ${BCI2000_SRC_DIR}/shared/modules
  ${BCI2000_SRC_DIR}/shared/types
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/shared/utils/Expression
  ${BCI2000_SRC_DIR}/shared/fileio
)
ELSE( BORLAND )
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR} 
  ${BCI2000_SRC_DIR}/shared
  ${BCI2000_SRC_DIR}/shared/accessors
  ${BCI2000_SRC_DIR}/shared/bcistream
  ${BCI2000_SRC_DIR}/shared/config
  ${BCI2000_SRC_DIR}/shared/modules
  ${BCI2000_SRC_DIR}/shared/types
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/shared/utils/Expression
  ${BCI2000_SRC_DIR}/shared/fileio
)
ENDIF( BORLAND )

# Dependencies of BasicFramework
IF( WIN32 )
  SET( LIBS ${LIBS} ws2_32 winmm )
ENDIF() 
IF( NOT APPLE AND NOT WIN32 )
  SET( LIBS ${LIBS} rt pthread )
ENDIF()
SET( LIBS ${LIBS} BasicFramework )
