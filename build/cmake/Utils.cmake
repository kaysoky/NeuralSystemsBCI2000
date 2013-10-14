###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: CMake utility functions

### Defines and options
SET( PROJECT_UTILS_DIR ${CMAKE_CURRENT_LIST_DIR} )

# Determine source code revision
EXECUTE_PROCESS( COMMAND svn info "${BCI2000_ROOT_DIR}" RESULT_VARIABLE result_ OUTPUT_VARIABLE output_ )
IF( result_ EQUAL 0 )
  STRING( REGEX REPLACE ".*\nLast Changed Rev: *([^\n]+).*" "\\1" PROJECT_REVISION ${output_} )
  STRING( REGEX REPLACE ".*\nLast Changed Date: *([^\n\\(]+).*" "\\1" PROJECT_DATE ${output_} )
  STRING( STRIP "${PROJECT_DATE}" PROJECT_DATE )
  ADD_DEFINITIONS(
    -DPROJECT_REVISION="${PROJECT_REVISION}"
    -DPROJECT_DATE="${PROJECT_DATE}"
  )
ENDIF()

# Create project defines
SET( PROJECT_VERSION "${LATEST_RELEASE}" )
IF( PROJECT_REVISION )
  # Set patch number to difference between current revision, and latest release
  MATH( EXPR PROJECT_VER_PATCH "${PROJECT_REVISION} - ${LATEST_RELEASE_REVISION}" )
ENDIF()
IF( PROJECT_VER_PATCH )
  SET( PROJECT_VERSION "${PROJECT_VERSION}.${PROJECT_VER_PATCH}" )
ENDIF()

STRING( TOLOWER "${PROJECT_NAME}" domain_ )
SET( PROJECT_DOMAIN "${domain_}.org" CACHE STRING "Domain name of main project" )
SET( PROJECT_SEARCH_ENGINE "google" CACHE STRING "Name of search engine for searching project domain" )
MARK_AS_ADVANCED( PROJECT_DOMAIN PROJECT_SEARCH_ENGINE )
ADD_DEFINITIONS(
  -DPROJECT_NAME="${PROJECT_NAME}"
  -DPROJECT_DOMAIN="${PROJECT_DOMAIN}"
  -DPROJECT_VERSION="${PROJECT_VERSION}"
  -DWEBSEARCH_DOMAIN="www.${PROJECT_SEARCH_ENGINE}.com"
)

# Determine host and user name
IF( NOT CONFIG_BUILD_USER )
  SITE_NAME( site_ )
  IF( CMAKE_HOST_WIN32 )
    EXECUTE_PROCESS( COMMAND net config workstation RESULT_VARIABLE result_ OUTPUT_VARIABLE output_ )
    IF( result_ EQUAL 0 )
      SET( pat_ ".*\nFull Computer name.([^\n]+).*" )
      IF( output_ MATCHES ${pat_} )
        STRING( REGEX REPLACE  ${pat_} "\\1" site_ ${output_} )
        STRING( STRIP "${site_}" site_ )
      ENDIF()
    ENDIF()
  ENDIF()
  SET( user_ "$ENV{USER}" )
  IF( user_ STREQUAL "" )
    SET( user_ "$ENV{USERNAME}" )
  ENDIF()
  IF( user_ STREQUAL "" )
    SET( user_ "unknown" )
  ENDIF()
  SET( CONFIG_BUILD_USER "${user_}@${site_}" CACHE STRING "Build user ID, may be used to track down the origin of executables" )
  UNSET( site_ CACHE )
ENDIF()
ADD_DEFINITIONS(
  -DBUILD_USER="${CONFIG_BUILD_USER}"
)

# Conditionally emit a status message
OPTION( CONFIG_VERBOSE "Set to OFF to suppress status information during configuration" ON )
FUNCTION( UTILS_CONFIG_STATUS )
  IF( CONFIG_VERBOSE )
    MESSAGE( STATUS ${ARGV} )
  ENDIF()
ENDFUNCTION()

FUNCTION( UTILS_FATAL_ERROR )
  IF( CONFIG_VERBOSE )
    MESSAGE( ${ARGV} "\n" )
  ENDIF()
  MESSAGE( FATAL_ERROR ${ARGV} "\n" )
ENDFUNCTION()

### Functions

# Include a file from the utils directory
MACRO( UTILS_INCLUDE file_ )
  INCLUDE( ${PROJECT_UTILS_DIR}/${file_}.cmake )
ENDMACRO()

# Parse arguments into provided variable names
MACRO( UTILS_PARSE_ARGS args_ )

  SET( firstargs_ ${args_} )
  LIST( LENGTH firstargs_ pos_ )
  IF( ${ARGC} LESS pos_ ) 
    MESSAGE( FATAL_ERROR "Too few arguments: args_ = \"${args_}\", ARGN = \"${ARGN}\"" )
  ENDIF()
  MATH( EXPR pos_ "${pos_}-1" )
  LIST( GET firstargs_ ${pos_} lastarg_ )
  LIST( REMOVE_AT firstargs_ ${pos_} )

  SET( remargs_ ${ARGN} )
  FOREACH( arg_ ${firstargs_} )
    LIST( GET remargs_ 0 ${arg_} )
    LIST( REMOVE_AT remargs_ 0 )
  ENDFOREACH()
  SET( ${lastarg_} )
  FOREACH( arg_ ${remargs_} )
    LIST( APPEND ${lastarg_} ${arg_} )
  ENDFOREACH()

ENDMACRO()

# Automatically add header files
FUNCTION( UTILS_AUTOHEADERS ioList )

  FOREACH( file_ ${${ioList}} )
    IF( file_ MATCHES .*\\.\(c|cpp|cc\) )
      STRING( REGEX REPLACE \(c|cpp|cc\)$ h header_ "${file_}" )
      IF( EXISTS ${header_} OR EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${header_} )
        LIST( APPEND files_ "${header_}" )
      ENDIF()
    ENDIF()
    LIST( APPEND files_ "${file_}" )
  ENDFOREACH()
  IF( DEFINED files_ )
    LIST( REMOVE_DUPLICATES files_ )
    SET( ${ioList} ${files_} PARENT_SCOPE )
  ENDIF()
  
ENDFUNCTION()

# Automatically handle Qt preprocessing
FUNCTION( UTILS_AUTOMOC ioList outNeedqt )

  SET( generated )
  FOREACH( file_ ${${ioList}} )
    IF( file_ MATCHES .*\\.h )
      FILE( STRINGS ${file_} qobjects REGEX [^/]*Q_OBJECT.* )
      LIST( LENGTH qobjects qobjects )
      IF( qobjects GREATER 0 )
        QT_WRAP_CPP( generated ${file_} )
      ENDIF()
    ELSEIF( file_ MATCHES .*\\.ui )
      QT_WRAP_UI( generated ${file_} )
    ELSEIF( file_ MATCHES .*.\\qrc )
      QT_ADD_RESOURCES( generated ${file_} )
    ENDIF()
  ENDFOREACH()

  SET( needqt FALSE )
  IF( generated )
    SOURCE_GROUP( Generated\\Qt FILES ${generated} )
    SET( ${ioList} ${${ioList}} ${generated} PARENT_SCOPE )
    SET( needqt TRUE )
  ELSEIF( ${QT_QTCORE_INCLUDE_DIR} )
    UTILS_IS_INCLUDED( ${QT_QTCORE_INCLUDE_DIR} needqt )
  ENDIF()
  SET( ${outNeedqt} ${needqt} PARENT_SCOPE )
  
ENDFUNCTION()

# Work around IDE/CMake bugs
FUNCTION( UTILS_FIXUP_FILES listname_ )

  IF( MSVC ) # With the VS2010 generator, there are problems 
             # with non-compiled files appearing in multiple projects.
             # As a workaround, we construct globally unique paths for such files.
    GET_FILENAME_COMPONENT( proj_dir_ "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE )
    GET_FILENAME_COMPONENT( proj_dir_name_ "${CMAKE_CURRENT_SOURCE_DIR}" NAME )
    SET( files_ )
    FOREACH( file_ ${${listname_}} )
      IF( NOT file_ MATCHES  .*\\.\(h|c|cpp\) )
        GET_FILENAME_COMPONENT( file_ "${file_}" ABSOLUTE )
        FILE( RELATIVE_PATH rpath_ "${proj_dir_}" "${file_}" )
        FILE( TO_CMAKE_PATH "${rpath_}" rpath_ )
        SET( file_ "../${proj_dir_name_}/${rpath_}" )
      ENDIF()
      LIST( APPEND files_ "${file_}" )
    ENDFOREACH()      
    SET( ${listname_} ${files_} PARENT_SCOPE )
  ENDIF()

ENDFUNCTION()

# Check if a preprocessor define exists in the current directory
FUNCTION( UTILS_IS_DEFINITION inDef outResult )

  GET_DIRECTORY_PROPERTY( defs COMPILE_DEFINITIONS )  
  SET( ${outResult} FALSE PARENT_SCOPE )
  UTILS_REGEX_ESCAPE( inDef )
  FOREACH( def ${defs} )
    IF( def MATCHES "^${inDef}$" OR def MATCHES "^${inDef}=.*" )
      SET( ${outResult} TRUE PARENT_SCOPE )
      RETURN()
    ENDIF()
  ENDFOREACH()

ENDFUNCTION()

# Check whether a path is being included
FUNCTION( UTILS_IS_INCLUDED inPath outResult )

  SET( ${outResult} FALSE PARENT_SCOPE )
  IF( "${inPath}" STREQUAL "" )
    RETURN()
  ENDIF()
  
  GET_FILENAME_COMPONENT( path "${inPath}" ABSOLUTE )
  GET_DIRECTORY_PROPERTY( dirs INCLUDE_DIRECTORIES )
  FOREACH( dir ${dirs} )
    GET_FILENAME_COMPONENT( dir "${dir}" ABSOLUTE )
    IF( dir STREQUAL path )
      SET( ${outResult} TRUE PARENT_SCOPE )
      RETURN()
    ENDIF()
  ENDFOREACH()

ENDFUNCTION()

# Add a flag to an existing target
FUNCTION( UTILS_ADD_FLAG name_ flag_ )
  GET_PROPERTY( flags_ TARGET ${name_} PROPERTY COMPILE_FLAGS )
  SET_PROPERTY( TARGET ${name_} PROPERTY COMPILE_FLAGS "${flags_} ${flag_}" )
ENDFUNCTION()

# Escape special regex metacharacters with a backslash
FUNCTION( UTILS_REGEX_ESCAPE ioVar )
  STRING( REGEX REPLACE "([$^.[|*+?()]|])" "\\\\\\1" result "${${ioVar}}" )
  SET( ${ioVar} ${result} PARENT_SCOPE )
ENDFUNCTION()

# Conditionally add a subdirectory
FUNCTION( UTILS_MATCH_SUBDIR inExpr inDir )
  STRING( TOLOWER "${CMAKE_CURRENT_SOURCE_DIR}/${inDir}//" dir )
  FOREACH( expr ${${inExpr}} )
    STRING( TOLOWER ${expr} expr )
    IF( "${dir}" MATCHES ".*/${expr}/.*" )
      ADD_SUBDIRECTORY( ${inDir} )
      RETURN()
    ENDIF()
  ENDFOREACH()
ENDFUNCTION()


# Get an external file from a local cache, or from the net.
FUNCTION( UTILS_GET_EXTERNAL inFile ioTempVar )

  MESSAGE( STATUS "Looking for ${inFile} ..." )
  SET( cachedfile "${CMAKE_SOURCE_DIR}/download_cache/${inFile}" )
  SET( can_connect FALSE )
  IF( EXISTS "${cachedfile}" )
    UTILS_CONFIG_STATUS( "... found locally at ${cachedfile}" )
  ELSE()
    SET( urlbase "http://${PROJECT_DOMAIN}/externals" )
    SET( url "${urlbase}/${inFile}" )
    # Try a direct download
    FILE( DOWNLOAD "${urlbase}" "${cachedfile}" TIMEOUT 200 INACTIVITY_TIMEOUT 300 STATUS result )
    IF( result EQUAL 0 )
      SET( can_connect TRUE )
      FILE( DOWNLOAD "${url}" "${cachedfile}" TIMEOUT 200 INACTIVITY_TIMEOUT 300 STATUS result SHOW_PROGRESS )
    ELSE()
      EXECUTE_PROCESS( COMMAND svn list "${urlbase}" TIMEOUT 600 RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET  )
      IF( result EQUAL 0 )
        SET( can_connect TRUE )
        EXECUTE_PROCESS( COMMAND svn list "${url}" TIMEOUT 600 RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET  )
        IF( result EQUAL 0 )
          FILE( WRITE "${cachedfile}" "" ) # ensure parent directory
          UTILS_CONFIG_STATUS( "... found at ${url}, downloading ..." )
          EXECUTE_PROCESS( COMMAND svn cat "${url}" TIMEOUT 600 RESULT_VARIABLE result OUTPUT_FILE "${cachedfile}" ERROR_QUIET )
        ENDIF()
      ENDIF()
    ENDIF()
    IF( NOT result EQUAL 0 )
      FILE( REMOVE "${cachedfile}" )
      IF( can_connect )
        UTILS_CONFIG_STATUS( "... not found." )
      ELSE()
        UTILS_CONFIG_STATUS( "... connection failed." )
      ENDIF()
    ENDIF()
  ENDIF()

  IF( EXISTS "${cachedfile}" )
    EXECUTE_PROCESS( COMMAND "${CMAKE_COMMAND}" -E copy "${cachedfile}" "${${ioTempVar}}" )
  ELSEIF( can_connect )
    SET( ${ioTempVar} NOTFOUND PARENT_SCOPE )
  ELSE()
    SET( ${ioTempVar} CANTCONNECT-NOTFOUND PARENT_SCOPE )
  ENDIF()
  
ENDFUNCTION()
