###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Find Qt4

IF( WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4 AND ( MINGW OR MSVC90 OR MSVC10 ) )
  OPTION( USE_EXTERNAL_QT
    "Set to ON to dynamically link BCI2000 against an existing Qt installation.
    Note that this will introduce various run-time dependencies which may be difficult
    to manage when deploying the resulting executables."
    OFF
  )
ELSE()
  SET( USE_EXTERNAL_QT ON )
ENDIF()

IF( USE_EXTERNAL_QT AND QT_BCI2000 )
  UNSET( QT_QMAKE_EXECUTABLE CACHE )
  UNSET( QT_BCI2000 )
ENDIF()

IF( NOT USE_EXTERNAL_QT )
  IF( WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4 )
    SET( qtarch_ "win32" )
  ENDIF()
  IF( MINGW )
    SET( qtcc_ "g++" )
  ELSEIF( MSVC90 )
    SET( qtcc_ "msvc2008" )
  ELSEIF( MSVC10 )
    SET( qtcc_ "msvc2010" )
  ENDIF()
  IF( qtarch_ AND qtcc_ )
    SET( qtdir_ "${qtarch_}-${qtcc_}" )
    IF( NOT EXISTS ${BCI2000_SRC_DIR}/extlib/qt/${qtdir_} )
      MESSAGE( "-- Extracting BCI2000-specific version of qt" )
      EXECUTE_PROCESS( COMMAND ${BCI2000_SRC_DIR}/extlib/qt.${qarch_}.exe -y -o. )
    ENDIF()
    SET( qmake_ "${BCI2000_SRC_DIR}/extlib/qt/${qtdir_}/bin/qmake.exe" )
    IF( EXISTS ${qmake_} )
      SET( ENV{QMAKESPEC} "${qtdir_}" )
      UNSET( QT_QMAKE_EXECUTABLE CACHE )
      SET( QT_QMAKE_EXECUTABLE
        ${BCI2000_SRC_DIR}/extlib/qt/${qtdir_}/bin/qmake.exe
        CACHE FILEPATH INTERNAL
      )
      SET( QT_BCI2000 TRUE CACHE BOOL INTERNAL )
      MARK_AS_ADVANCED( QT_QMAKE_EXECUTABLE QT_BCI2000 )
    ENDIF()
  ENDIF()
ENDIF()

FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE( ${QT_USE_FILE} )
