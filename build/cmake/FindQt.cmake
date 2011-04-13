###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Find Qt4

IF( NOT BORLAND )

# set USE_STD_QT to TRUE if you want to use a Qt installation outside the
# BCI2000 source tree
SET( USE_STD_QT FALSE )

IF( USE_STD_QT )
  FIND_PACKAGE( Qt4 REQUIRED )
ELSE()
  INCLUDE( cmake/extlib/Qt4.cmake )
ENDIF()
INCLUDE( ${QT_USE_FILE} )

ENDIF( NOT BORLAND )