###########################################################################
## $Id: SetupExtlibDependencies.cmake 3541 2011-09-13 15:33:18Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Macro that simplifies BCI2000 CMake files

FUNCTION( BCI2000_ADD_TARGET )

  BCI2000_PARSE_ARGS( "kind_;name_;sources_" ${ARGV} )
  SET( failed_ FALSE )
  BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS failed_ )

  IF( NOT failed_ )

    BCI2000_SETUP_RESOURCES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK )
    SET( sources_ ${sources_} ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} )

    BCI2000_AUTODEPEND( sources_ LIBS )
    BCI2000_AUTOHEADERS( sources_ )
    BCI2000_AUTOSOURCEGROUPS( sources_ )
    BCI2000_AUTOMOC( sources_ )
    BCI2000_AUTOINCLUDE( sources_ )
    BCI2000_FIXUP_FILES( sources_ )
    
    IF( ADD_TARGET_HOOK )
      ADD_TARGET_HOOK()
    ENDIF()
    IF( VERBOSE_CONFIG )
      MESSAGE( "--- Adding ${kind_}: " ${name_} )
    ENDIF()
    
    IF( kind_ STREQUAL QTAPP )
      ADD_EXECUTABLE( ${name_} WIN32 ${sources_} )
      BCI2000_ADD_FLAG( ${name_} "-DUSE_QT" )
      TARGET_LINK_LIBRARIES( ${name_} ${QT_LIBRARIES} )

    ELSEIF( kind_ STREQUAL WINAPP )
      ADD_EXECUTABLE( ${name_} WIN32 ${sources_} )

    ELSEIF( kind_ STREQUAL EXECUTABLE )
      ADD_EXECUTABLE( ${name_} ${sources_} )

    ELSEIF( kind_ STREQUAL STATIC_LIBRARY )
      ADD_LIBRARY( ${name_} STATIC ${sources_} )

    ELSEIF( kind_ STREQUAL MODULE )
      ADD_LIBRARY( ${name_} MODULE ${sources_} )
      SET_PROPERTY( TARGET "${name_}" PROPERTY PREFIX "" )

    ELSEIF( kind_ STREQUAL DLL )
      ADD_LIBRARY( ${name_} SHARED ${sources_} )
      SET_PROPERTY( TARGET "${name_}" PROPERTY PREFIX "" )
      IF( MINGW )
        SET_PROPERTY( TARGET ${name_} APPEND PROPERTY
          LINK_FLAGS "-Wl,--add-stdcall-alias"
        )
      ENDIF()
      IF( APPLE )
        SET_PROPERTY( TARGET ${name_} APPEND PROPERTY
          LINK_FLAGS "-install_name @executable_path/${name_}.dylib"
        )
      ENDIF()

    ELSE()
      MESSAGE( SEND_ERROR "Unknown target kind: ${kind_}" )
      SET( failed_ TRUE )

    ENDIF()

  ENDIF()

  IF( NOT failed_ )
    TARGET_LINK_LIBRARIES( ${name_} ${LIBS} )
    SET_PROPERTY( TARGET ${name_} PROPERTY FOLDER ${DIR_NAME} )
  ENDIF()
  
  SET( FAILED "${failed_}" PARENT_SCOPE )

ENDFUNCTION()

# Parse arguments into provided variable names
MACRO( BCI2000_PARSE_ARGS args_ )

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

# Automatically define source groups for framework files
FUNCTION( BCI2000_AUTOSOURCEGROUPS listname_ )

  GET_FILENAME_COMPONENT( proj_dir_ "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE )
  GET_FILENAME_COMPONENT( frame_dir_ "${BCI2000_SRC_DIR}" ABSOLUTE )

  FOREACH( file_ ${${listname_}} )

    SET( section_ Source )
    IF( file_ MATCHES .*\\.h )
      SET( section_ Headers )
    ELSEIF( file_ MATCHES .*\\.\(res|rc|rsrc|ui|def\) )
      SET( section_ Resources )
    ELSEIF( RESOURCE_FILES )
      LIST( FIND RESOURCE_FILES ${file_} pos )
      IF( NOT pos LESS 0 )
        SET( section_ Resources )
      ENDIF()
    ENDIF()
    
    SET( subsection Project )
    STRING( FIND ${file_} ${BCI2000_SRC_DIR} pos )
    GET_FILENAME_COMPONENT( file_ ${file_} ABSOLUTE )
    FILE( TO_CMAKE_PATH "${file_}" ${file_} )
    GET_FILENAME_COMPONENT( path_ ${file_} PATH )
    FILE( TO_CMAKE_PATH "${path_}" path_ )
    GET_FILENAME_COMPONENT( ext_ ${file_} EXT )
    IF( pos LESS 0 )
      # Either not a framework file, or defined relative to project
      FILE( RELATIVE_PATH rpath_ "${proj_dir_}" "${path_}" )
      FILE( TO_CMAKE_PATH "${rpath_}" rpath_ )
    ELSE()
      FILE( RELATIVE_PATH rpath_ "${frame_dir_}" "${path_}" )
      FILE( TO_CMAKE_PATH ${rpath_} rpath_ )
      IF( rpath_ MATCHES \(shared|core\)\(/|$\).* )
        SET( subsection "BCI2000 Framework" )
      ELSEIF( rpath_ MATCHES extlib/.* )
        IF( rpath_ MATCHES extlib/\(3DAPI|com|fftlib|math\)\(/|$\).* )
          SET( subsection "BCI2000 Framework" )
        ELSE()
          SET( subsection "External Libraries" )
          STRING( REPLACE "extlib/" "" rpath_ "${rpath_}" )
        ENDIF()
      ELSEIF( rpath_ MATCHES contrib/Extensions/.* )
        SET( subsection Extensions )
        SET( rpath_ "" )
      ELSEIF( rpath_ MATCHES contrib/.* )
        SET( subsection "BCI2000 Contrib" )
        FILE( RELATIVE_PATH rpath_ "${frame_dir_}/contrib" "${path_}" )
        FILE( TO_CMAKE_PATH "${rpath_}" rpath_ )
      ENDIF()
    ENDIF()
    IF( NOT "${rpath_}" STREQUAL "" )
      SET( rpath_ "/${rpath_}" )
      STRING( REPLACE "/.." "" rpath_ "${rpath_}" )
    ENDIF()
    SET( group_ "${section_}/${subsection}${rpath_}" )
    LIST( APPEND rules_ "${group_}|${path_}|${ext_}" )

  ENDFOREACH()

  LIST( REMOVE_DUPLICATES rules_ )
  FOREACH( rule_ ${rules_} )
    STRING( REPLACE "|" ";" rule_ ${rule_} )
    SET( rule_ ${rule_} )
    BCI2000_PARSE_ARGS( "group_;path_;ext_" ${rule_} )
    REGEX_ESCAPE_STRING( path_ "${path_}" )
    REGEX_ESCAPE_STRING( ext_ "${ext_}" )
    SET( regex_ "${path_}/[^/]*${ext_}" )
    STRING( REPLACE "/" "\\" group_ "${group_}" )
    # Using SOURCE_GROUP( REGULAR_EXPRESSION ) allows override by SOURCE_GROUP( FILES )
    SOURCE_GROUP( "${group_}" REGULAR_EXPRESSION "${regex_}" )
  ENDFOREACH()
  
ENDFUNCTION()

# Automatically add header files
FUNCTION( BCI2000_AUTOHEADERS listname_ )

  SET( newlist_ ${${listname_}} )
  FOREACH( file_ ${${listname_}} )
    IF( file_ MATCHES .*\\.\(c|cpp\) )
      STRING( REGEX REPLACE \(c|cpp\)$ h header_ "${file_}" )
      IF( EXISTS ${header_} OR EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${header_} )
        LIST( APPEND newlist_ "${header_}" )
      ENDIF()
    ENDIF()
  ENDFOREACH()
  LIST( REMOVE_DUPLICATES newlist_ )
  SET( ${listname_} ${newlist_} PARENT_SCOPE )
  
ENDFUNCTION()

# Automatically handle Qt preprocessing
FUNCTION( BCI2000_AUTOMOC listname_ )

  FOREACH( file_ ${${listname_}} )
    IF( file_ MATCHES .*\\.h )
      FILE( STRINGS ${file_} qobjects REGEX [^/]*Q_OBJECT.* )
      LIST( LENGTH qobjects qobjects )
      IF( qobjects GREATER 0 )
        QT4_WRAP_CPP( generated ${file_} )
      ENDIF()
    ELSEIF( file_ MATCHES .*\\.ui )
      QT4_WRAP_UI( generated ${file_} )
    ENDIF()
  ENDFOREACH()
  IF( generated )
    SOURCE_GROUP( Generated\\Qt FILES ${generated} )
    SET( ${listname_} ${${listname_}} ${generated} PARENT_SCOPE )
  ENDIF()

ENDFUNCTION()

# Add dependencies of individual source files
FUNCTION( BCI2000_AUTODEPEND  listname_ libsname_ )

  SET( newlibs_ )
  SET( newfiles_ )

  GET_DIRECTORY_PROPERTY( defs_ COMPILE_DEFINITIONS )
  LIST( FIND defs_ "DISABLE_BCITEST" idx_ )
  IF( idx_ LESS 0 ) # BCITEST enabled
    # This file will not do anything unless "main" is redefined by BCI2000_ADD_TEST
    #LIST( APPEND newfiles_ ${BCI2000_SRC_DIR}/shared/bcistream/BCITestMain.cpp )
  ENDIF()

  FOREACH( file_ ${${listname_}} )
    IF( WIN32 AND file_ MATCHES .*/SockStream\\.cpp )
      LIST( APPEND newlibs_ ws2_32 )
    ENDIF()
  ENDFOREACH()
  SET( ${listname_} ${${listname_}} ${newfiles_} PARENT_SCOPE )
  SET( ${libsname_} ${${libsname_}} ${newlibs_} PARENT_SCOPE )

ENDFUNCTION()

# Automatically add proper include directories (to work reliably, must operate on absolute paths)
FUNCTION( BCI2000_AUTOINCLUDE listname_ )

  SET( paths_
    ${BCI2000_SRC_DIR}/shared
    ${BCI2000_SRC_DIR}/shared/config
    ${BCI2000_SRC_DIR}/shared/bcistream
  )
  FOREACH( file_ ${${listname_}} )
    IF( file_ MATCHES .*\\.\(h|c|cpp\) )
      GET_FILENAME_COMPONENT( path_ ${file_} PATH )
      SET( paths_ ${paths_} ${path_} )
    ENDIF()
  ENDFOREACH()
  IF( paths_ )
    LIST( REMOVE_DUPLICATES paths_ )
    INCLUDE_DIRECTORIES( ${paths_} )
  ENDIF()

ENDFUNCTION()

# Work around IDE/CMake bugs
FUNCTION( BCI2000_FIXUP_FILES listname_ )

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


FUNCTION( BCI2000_ADD_FLAG name_ flag_ )

  GET_PROPERTY( flags_ TARGET ${name_} PROPERTY COMPILE_FLAGS )
  SET_PROPERTY( TARGET ${name_} PROPERTY COMPILE_FLAGS "${flags_} ${flag_}" )

ENDFUNCTION()


# Escape special regex metacharacters with a backslash
MACRO(REGEX_ESCAPE_STRING _OUT _IN)
  string(REGEX REPLACE "([$^.[|*+?()]|])" "\\\\\\1" ${_OUT} "${_IN}")
ENDMACRO()

# Debugging:
# SET( DEBUG 1 ) in a directory's CMakeLists.txt will write debug files when projects are created
IF( NOT ADD_TARGET_HOOK )
  MACRO( ADD_TARGET_HOOK )
    IF( DEBUG )
      set( debug_file_ "${CMAKE_CURRENT_SOURCE_DIR}/cmake-debug.txt" )
      file( APPEND "${debug_file_}" "==== Target: $name_ ============================\n" )
      get_cmake_property(_variableNames VARIABLES)
      foreach (_variableName ${_variableNames})
        file( APPEND "${debug_file_}" "${_variableName}=${${_variableName}}\n" )
      endforeach()
      file( APPEND "${debug_file_}" "================================\n" )
    ENDIF( DEBUG )
  ENDMACRO( ADD_TARGET_HOOK )
ENDIF()
