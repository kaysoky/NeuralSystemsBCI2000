###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Find Qt4

SET( TRY_PRECOMP_QT
  "4.8.4"
  "4.7.0"
)

SET( QT5_MODULES
  Core
  Widgets
  GUI
)

SET( precompqt_ QT_${PROJECT_NAME} )

FUNCTION( CLEAR_QT_VARIABLES )

  GET_CMAKE_PROPERTY( vars VARIABLES )
  FOREACH( var ${vars} )
    IF( var MATCHES "QT_.*" )
      UNSET( ${var} CACHE )
    ENDIF()
  ENDFOREACH()

ENDFUNCTION()


FUNCTION( GET_EXTRACT_QT qtver arch qmake )

  SET( qtfile "qt/qt-${qtver}.${arch}" )
  SET( basedir "${PROJECT_BUILD_DIR}/extlib" )
  SET( qtdir "${basedir}/qt-${qtver}" )
  
  IF( NOT EXISTS "${qtdir}/${arch}" )
    SET( qttemp "${qtdir}/qttemp.exe" )
    FILE( REMOVE "${qttemp}" )
    UTILS_GET_EXTERNAL( "${qtfile}" qttemp )
    IF( NOT qttemp )
      SET( qmake_ ${qttemp} )
    ELSE()
      MESSAGE( STATUS "Setting up ${qtfile} ..." )
      EXECUTE_PROCESS( COMMAND "${qttemp}" -y "-o${basedir}" RESULT_VARIABLE sfxresult )
      IF( NOT sfxresult EQUAL 0 )
        MESSAGE( FATAL_ERROR "Could not extract ${qttemp}, error is: ${sfxresult}" )
      ENDIF()
    ENDIF()    
  ENDIF()

  IF( EXISTS "${qtdir}/${arch}" )
    SET( qmake_ "${qtdir}/${arch}/bin/qmake" )
    IF( arch MATCHES "win.*" )
      SET( qmake_ "${qmake_}.exe" )
    ENDIF()
    IF( NOT EXISTS "${qmake_}" )
      SET( qmake_ TRUE )
    ENDIF()
  ENDIF()
  SET( ${qmake} "${qmake_}" PARENT_SCOPE )  

ENDFUNCTION()


FUNCTION( GET_PRECOMP_QT qtver outResult )

  SET( result NOTFOUND )
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
    GET_EXTRACT_QT( ${qtver} "include" result )
    IF( result )
      GET_EXTRACT_QT( ${qtver} "${arch}-${cc}" result )
    ENDIF()

    IF( result )
      IF( NOT QT_QMAKE_EXECUTABLE OR NOT QT_QMAKE_EXECUTABLE STREQUAL qmake_ )
        CLEAR_QT_VARIABLES()
      ENDIF()

      SET( ENV{QMAKESPEC} "${qtarch}-${cc}" )
      SET( QT_QMAKE_EXECUTABLE "${result}" CACHE FILEPATH "" FORCE )
      MARK_AS_ADVANCED( FORCE QT_QMAKE_EXECUTABLE )
    ENDIF()
  ENDIF()
  SET( ${outResult} "${result}" PARENT_SCOPE )

ENDFUNCTION()


FUNCTION( FIND_PRECOMP_QT outResult )

  SET( ${outResult} NOTFOUND PARENT_SCOPE )
  SET( ${precompqt_} FALSE CACHE INTERNAL "" FORCE )
  MARK_AS_ADVANCED( FORCE ${precompqt_} )
  FOREACH( qtver_ ${TRY_PRECOMP_QT} )
    GET_PRECOMP_QT( ${qtver_} result )
    SET( ${outResult} "${result}" PARENT_SCOPE )
    IF( result )
      SET( ${precompqt_} TRUE CACHE INTERNAL "" FORCE )
      RETURN()
    ENDIF()
  ENDFOREACH()  

ENDFUNCTION()


FUNCTION( GET_QT_VERSION outVer )

  SET( ${outVer} NOTFOUND PARENT_SCOPE )
  IF( EXISTS ${QT_QMAKE_EXECUTABLE} )
    EXECUTE_PROCESS(
      COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_VERSION
      OUTPUT_VARIABLE ver
      RESULT_VARIABLE result
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    IF( result EQUAL 0 )
      SET( ${outVer} ${ver} PARENT_SCOPE )
    ENDIF()
  ENDIF()

ENDFUNCTION()


MACRO( QT_WRAP_CPP )
  IF( QT_IS4 )
    QT4_WRAP_CPP( ${ARGN} )
  ELSE()
    QT5_WRAP_CPP( ${ARGN} )
  ENDIF()
ENDMACRO()

MACRO( QT_WRAP_UI )
  IF( QT_IS4 )
    QT4_WRAP_UI( ${ARGN} )
  ELSE()
    QT5_WRAP_UI( ${ARGN} )
  ENDIF()
ENDMACRO()

MACRO( QT_ADD_RESOURCES )
  IF( QT_IS4 )
    QT4_ADD_RESOURCES( ${ARGN} )
  ELSE()
    QT5_ADD_RESOURCES( ${ARGN} )
  ENDIF()
ENDMACRO()


## Main

SET( result_ )
IF( USE_EXTERNAL_QT AND ${precompqt_} )
  CLEAR_QT_VARIABLES()
ENDIF()
IF( NOT USE_EXTERNAL_QT )
  FIND_PRECOMP_QT( result_ )
ENDIF()
LIST( GET TRY_PRECOMP_QT -1 qtmin_ )

# Try to find Qt4 or 5
IF( USE_EXTERNAL_QT AND NOT Qt5_DIR )
  FIND_PROGRAM( QT_QMAKE_EXECUTABLE qmake )
ENDIF()
IF( NOT USE_EXTERNAL_QT OR NOT QT_QMAKE_EXECUTABLE )
  FIND_PACKAGE( Qt4 ${qtmin} QUIET )
ENDIF()

# Determine version
GET_QT_VERSION( qtver_ )
IF( qtver_ VERSION_LESS 5.0 )
  SET( QT_IS4 TRUE )
  SET( compdir_ "${PROJECT_BUILD_DIR}/extlib/qt/compat" )
  FILE( MAKE_DIRECTORY ${compdir_} )
  FILE( WRITE "${compdir_}/QtWidgets" "#include <QtGui>\n" )
  INCLUDE_DIRECTORIES( ${compdir_} )
ELSE()
  IF( QT_QMAKE_EXECUTABLE AND NOT Qt5_DIR )
    GET_FILENAME_COMPONENT( Qt5_DIR ${QT_QMAKE_EXECUTABLE} PATH )
  ENDIF()
  IF( Qt5_DIR )
    FIND_PACKAGE( Qt5 COMPONENTS ${QT5_MODULES} OpenGL )
    IF( Qt5_FOUND )
      SET( QT_VERSION ${Qt5_VERSION} )
    ENDIF()
  ENDIF()
ENDIF()

IF( NOT qtver_ OR qtver_ VERSION_LESS qtmin_ )
  IF( "${result_}" STREQUAL CANTCONNECT-NOTFOUND )
    UTILS_FATAL_ERROR(
      "Pre-compiled Qt libraries could not be downloaded because no connection to ${PROJECT_DOMAIN} could be made.\n"
      "NOTE: If you need to configure ${PROJECT_NAME} without an internet connection, do a checkout "
      "from \"offline/trunk/\" rather than \"trunk/\"."
    )
  ELSEIF( "${result_}" STREQUAL NOTFOUND )
    UTILS_FATAL_ERROR(
      "There is no precompiled version of Qt available for your platform/compiler.\n"
      "You will need to install Qt >=${qtmin_} on your machine in order to build ${PROJECT_NAME}."
    )
  ELSE()
    UTILS_FATAL_ERROR( "Qt >=${qtmin_} must be available to build ${PROJECT_NAME}." )
  ENDIF()
ELSE()
  GET_FILENAME_COMPONENT( qtdir_ ${QT_QMAKE_EXECUTABLE} PATH )
  UTILS_CONFIG_STATUS( "Using Qt ${qtver_} (${qtdir_})" )
ENDIF()

SET( doc_ 
  "Set to ON to link ${PROJECT_NAME} against an existing Qt installation.
  Note that this may introduce various run-time dependencies which will be difficult
  to manage when deploying the resulting executables."
)
IF( ${precompqt_} )
  OPTION( USE_EXTERNAL_QT "${doc_}" OFF )
ELSE()
  SET( USE_EXTERNAL_QT ON CACHE BOOL "${doc_}" FORCE )
ENDIF()

