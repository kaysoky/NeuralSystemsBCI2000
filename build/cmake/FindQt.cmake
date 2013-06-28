###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Find Qt4

SET( TRY_INTERNAL_QT
  "4.8.4"
  "4.7.0"
)

SET( internalqt_ QT_${PROJECT_NAME} )

FUNCTION( CLEAR_QT_VARIABLES )

  GET_CMAKE_PROPERTY( vars VARIABLES )
  FOREACH( var ${vars} )
    IF( var MATCHES "QT_.*" )
      UNSET( ${var} CACHE )
    ENDIF()
  ENDFOREACH()

ENDFUNCTION()


FUNCTION( DOWNLOAD_EXTRACT_QT qtver arch qmake )

  SET( qturl "http://${PROJECT_DOMAIN}/externals/qt/qt-${qtver}.${arch}" )
  SET( basedir "${PROJECT_BUILD_DIR}/extlib" )
  SET( qtdir "${basedir}/qt-${qtver}" )
  SET( qttemp "${qtdir}/qttemp.exe" )
  FILE( REMOVE "${qttemp}" )
  IF( NOT EXISTS "${qtdir}/${arch}" )

    MESSAGE( STATUS "Checking availability of qt-${qtver}.${arch} ..." )
    FILE( DOWNLOAD "${qturl}" "${qttemp}" TIMEOUT 20 INACTIVITY_TIMEOUT 300 STATUS result SHOW_PROGRESS )
    IF( NOT result EQUAL 0 )
      MESSAGE( STATUS "qt-${qtver}.${arch} is not available, you may need to manually install Qt on your machine." )
    ELSE()
      IF( NOT EXISTS "${qttemp}" )
        MESSAGE( FATAL_ERROR "Could not download file: ${qturl}" )
      ELSE()
        MESSAGE( STATUS "Extracting ..." )
        EXECUTE_PROCESS( COMMAND "${qttemp}" -y "-o${basedir}" RESULT_VARIABLE sfxresult )
        IF( NOT sfxresult EQUAL 0 )
          MESSAGE( FATAL_ERROR "Could not extract qt, error is: ${sfxresult}" )
        ENDIF()
      ENDIF()
    ENDIF()
    
  ENDIF()

  SET( qmake_ "${qtdir}/${arch}/bin/qmake" )
  IF( arch MATCHES "win.*" )
    SET( qmake_ "${qmake_}.exe" )
  ENDIF()
  SET( ${qmake} "${qmake_}" PARENT_SCOPE )  

ENDFUNCTION()


FUNCTION( GET_QT_FROM_SERVER qtver )

  IF( WIN32 )
    IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
      SET( arch "win32" )
    ELSE()
      SET( arch "win32-amd64" )
    ENDIF()
    SET( qtarch "win32" )
  ELSE()
    SET( arch ${CMAKE_SYSTEM_NAME} )
    STRING( TOLOWER ${arch} arch )
    SET( qtarch ${arch} )
  ENDIF()
  
  IF( COMPILER_IS_GCC_COMPATIBLE )
    SET( cc "g++" )
  ELSEIF( MSVC90 )
    SET( cc "msvc2008" )
  ELSEIF( MSVC10 )
    SET( cc "msvc2010" )
  ENDIF()

  IF( arch AND cc )
    SET( arch "${arch}-${cc}" )
    DOWNLOAD_EXTRACT_QT( ${qtver} "${arch}" qmake_ )

    IF( qmake_ AND EXISTS "${qmake_}" )
      IF( NOT QT_QMAKE_EXECUTABLE OR NOT QT_QMAKE_EXECUTABLE STREQUAL qmake_ )
        CLEAR_QT_VARIABLES()
      ENDIF()

      DOWNLOAD_EXTRACT_QT( ${qtver} "include" ignore )
      SET( ENV{QMAKESPEC} "${qtarch}-${cc}" )
      SET( QT_QMAKE_EXECUTABLE "${qmake_}" CACHE FILEPATH "" FORCE )
      SET( QT_BCI2000 TRUE CACHE INTERNAL "" FORCE )
      MARK_AS_ADVANCED( FORCE QT_QMAKE_EXECUTABLE ${internalqt_} )
    ENDIF()
  ENDIF()

ENDFUNCTION()


## Main

IF( USE_EXTERNAL_QT AND ${internalqt_} )
  CLEAR_QT_VARIABLES()
ENDIF()
IF( NOT USE_EXTERNAL_QT )
  SET( ${internalqt_} FALSE CACHE INTERNAL "" FORCE ) 
  FOREACH( qtver_ ${TRY_INTERNAL_QT} )
    IF( NOT ${internalqt_} )
      GET_QT_FROM_SERVER( ${qtver_} )
    ENDIF()
  ENDFOREACH()
ENDIF()

SET( doc_ 
  "Set to ON to link ${PROJECT_NAME} against an existing Qt installation.
  Note that this may introduce various run-time dependencies which will be difficult
  to manage when deploying the resulting executables."
)
IF( ${internalqt_} )
  OPTION( USE_EXTERNAL_QT "${doc_}" OFF )
ELSE()
  SET( USE_EXTERNAL_QT ON CACHE BOOL "${doc_}" FORCE )
ENDIF()

LIST( GET TRY_INTERNAL_QT -1 qtmin_ )
FIND_PACKAGE( Qt4 ${qtmin_} REQUIRED )
