###########################################################################
## $Id: SetupExtlibDependencies.cmake 3541 2011-09-13 15:33:18Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Macro that simplifies BCI2000 CMake files

# Counting targets
SET( ENV{TARGET_NUMBER} 0 )
FUNCTION( NEXT_TARGET outTargetNumber )
  MATH( EXPR target "$ENV{TARGET_NUMBER} + 1" )
  SET( ENV{TARGET_NUMBER} ${target} )
  SET( ${outTargetNumber} ${target} PARENT_SCOPE )
ENDFUNCTION()
  
# Debugging targets
SET( DebugCMake_Target 0 CACHE STRING "Target number to be debugged, 0 for none" )
SET( DebugCMake_DumpProperties
  "DIRECTORY\;COMPILE_DEFINITIONS;CMAKE\;VARIABLES"
  CACHE STRING "Properties to display when debugging"
)
MARK_AS_ADVANCED( DebugCMake_Target DebugCMake_DumpProperties )

MACRO( dump_debug_info_ )
  MESSAGE( "======= Dumping debug info for target #${target}: ${kind_} ${name_} =======" )
  FOREACH( _prop ${DebugCMake_DumpProperties} )
    MESSAGE( "======= ${_prop} =======" )
    LIST( GET _prop 0 _type )
    LIST( GET _prop 1 _name )
    IF( _type STREQUAL CMAKE )
      GET_CMAKE_PROPERTY( _vars ${_name} )
    ELSE()
      GET_PROPERTY( _vars ${_type} PROPERTY ${_name} )
    ENDIF()
    FOREACH( _var ${_vars}) 
      IF( DEFINED ${_var} )
        MESSAGE( "${_var}==${${_var}}" )
      ELSE()
        MESSAGE( "${_var}" )
      ENDIF()
    ENDFOREACH()
  ENDFOREACH()
  MESSAGE( "======= End of debug info for ${name_} =======" )
ENDMACRO()

# Add any kind of target
FUNCTION( BCI2000_ADD_TARGET )
  UTILS_PARSE_ARGS( "kind_;name_;sources_" ${ARGV} )
  IF( kind_ STREQUAL INFO )
    SET( info_ ${name_} )
    UTILS_PARSE_ARGS( "kind_;name_;sources_" ${sources_} )
  ENDIF()

  IF( kind_ STREQUAL WINDLL OR kind_ STREQUAL WINAPP )
    IF( NOT WIN32 )
      SET( FAILED TRUE PARENT_SCOPE )
      RETURN()
    ENDIF()
  ENDIF()
  IF( kind_ STREQUAL WINDLL AND CMAKE_SIZEOF_VOID_P EQUAL 8 )
    SET( name_ "${name_}64" )
  ENDIF()

  NEXT_TARGET( target )
  SET( debug OFF )
  IF( DebugCMake_Target EQUAL target OR DebugCMake_Target STREQUAL ${name_} )
    SET( debug ON )
  ENDIF()

  SET( failed_ FALSE )
  BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS failed_ )
  IF( failed_ )
  
    MESSAGE( "Target #${target}: ${name_}: not added" )
  
  ELSE()

    BCI2000_SETUP_RESOURCES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK )
    SET( sources_ ${sources_} ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} )
    
    BCI2000_AUTODEPEND( sources_ LIBS )
    UTILS_AUTOHEADERS( sources_ )
    BCI2000_AUTOSOURCEGROUPS( sources_ companions outputDir )
    UTILS_AUTOMOC( sources_ needqt_ )
    BCI2000_AUTOINCLUDE( sources_ )

    IF( NOT needqt_ )
      UTILS_IS_DEFINITION( USE_QT needqt_ )
    ENDIF()
    IF( NOT DEFINED outputDir )
      SET( outputDir ${PROJECT_OUTPUT_DIR} )
    ENDIF()
    
    IF( debug )
      dump_debug_info_()
    ENDIF()
    
    SET( build_config_ ${BUILD_CONFIG} )
    IF( needqt_ )
      IF( kind_ STREQUAL GUIAPP OR kind_ STREQUAL EXECUTABLE )
        SET( kind_ QTAPP )
      ENDIF()
      INCLUDE( ${QT_USE_FILE} )
      SET( build_config_ "${build_config_} Qt:${QT_VERSION}:{QT_QMAKE_EXECUTABLE}" )
    ENDIF()
    ADD_DEFINITIONS( -DBUILD_CONFIG=\"${build_config_}\" )

    IF( DEFINED info_ )
      SET( info_ "${info_}, " )
    ENDIF()
    UTILS_CONFIG_STATUS( "Target #${target}: ${name_} (${info_}${kind_})" )

    # Now, add the target
    IF( ADD_TARGET_HOOK )
      ADD_TARGET_HOOK()
    ENDIF()
    
    IF( kind_ STREQUAL QTAPP )
      ADD_EXECUTABLE( ${name_} WIN32 ${sources_} )
      UTILS_ADD_FLAG( ${name_} "-DUSE_QT" )
      TARGET_LINK_LIBRARIES( ${name_} ${QT_LIBRARIES} )

    ELSEIF( kind_ STREQUAL WINAPP OR kind_ STREQUAL GUIAPP )
      ADD_EXECUTABLE( ${name_} WIN32 ${sources_} )

    ELSEIF( kind_ STREQUAL EXECUTABLE )
      ADD_EXECUTABLE( ${name_} ${sources_} )

    ELSEIF( kind_ STREQUAL STATIC_LIBRARY )
      ADD_LIBRARY( ${name_} STATIC ${sources_} )
      SET( outputDir )

    ELSEIF( kind_ STREQUAL MODULE OR kind_ STREQUAL WINDLL )
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

    IF( outputDir )
      ADD_CUSTOM_COMMAND(
        TARGET ${name_}
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${name_}>" "${outputDir}"
        COMMENT "Copy to destination"
      )
    ENDIF()
    
    # Handle companion files
    FOREACH( companion ${companions} )
      SET( copyto "${PROJECT_OUTPUT_DIR}" )
      IF( companion MATCHES "\\.bat" )
        SET( copyto "${copyto}/../batch" )
      ENDIF()
      
      GET_FILENAME_COMPONENT( companion ${companion} ABSOLUTE )
      ADD_CUSTOM_COMMAND(
        TARGET ${name_}
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy "${companion}" "${copyto}"
      )
    ENDFOREACH()
  ENDIF()
  
  SET( FAILED "${failed_}" PARENT_SCOPE )

ENDFUNCTION()

# Automatically define source groups for framework files
FUNCTION( BCI2000_AUTOSOURCEGROUPS ioList outCompanions outTargetDir )

  GET_FILENAME_COMPONENT( proj_dir_ "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE )
  GET_FILENAME_COMPONENT( frame_dir_ "${PROJECT_SRC_DIR}" ABSOLUTE )
  SET( list_ ${${ioList}} )
  SET( companions )

  LIST( FIND list_ "OUTPUT_DIRECTORY" idx )
  IF( NOT idx EQUAL -1 )
    LIST( REMOVE_AT list_ ${idx} )
    LIST( GET list_ ${idx} outDir )
    LIST( REMOVE_AT list_ ${idx} )
    SET( ${outTargetDir} "${outDir}" PARENT_SCOPE )
  ENDIF()  

  UTILS_FIXUP_FILES( list_ )
  
  FOREACH( file_ ${list_} )
    
    SET( section_ Source )
    IF( file_ MATCHES .*\\.h )
      SET( section_ Headers )
    ELSEIF( file_ MATCHES .*\\.\(res|rc|rsrc|ui|qrc|def\) )
      SET( section_ Resources )
    ELSEIF( file_ MATCHES .*\\.\(bat|dll|so|cmd|exe|txt|htm|html|rtf|pdf|tex\) )
      SET( section_ Companions )
      LIST( APPEND companions ${file_} )
    ELSEIF( RESOURCE_FILES )
      LIST( FIND RESOURCE_FILES ${file_} pos )
      IF( NOT pos LESS 0 )
        SET( section_ Resources )
      ENDIF()
    ENDIF()

    SET( subsection Project )
    STRING( FIND ${file_} ${PROJECT_SRC_DIR} pos )
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
    UTILS_PARSE_ARGS( "group_;path_;ext_" ${rule_} )
    UTILS_REGEX_ESCAPE( path_ )
    UTILS_REGEX_ESCAPE( ext_ )
    SET( regex_ "${path_}/[^/]*${ext_}" )
    STRING( REPLACE "/" "\\" group_ "${group_}" )
    # Using SOURCE_GROUP( REGULAR_EXPRESSION ) allows override by SOURCE_GROUP( FILES )
    SOURCE_GROUP( "${group_}" REGULAR_EXPRESSION "${regex_}" )
  ENDFOREACH()
  
  SET( ${ioList} ${list_} PARENT_SCOPE )
  SET( ${outCompanions} ${companions} PARENT_SCOPE )
  
ENDFUNCTION()

# Add dependencies of individual source files
FUNCTION( BCI2000_AUTODEPEND  listname_ libsname_ )

  SET( newlibs_ )
  SET( newfiles_ )

  UTILS_IS_DEFINITION( DISABLE_BCITEST disabled_ )
  IF( NOT disabled_ )
    # This file will not do anything unless "main" is redefined by BCI2000_ADD_TEST
    #LIST( APPEND newfiles_ ${PROJECT_SRC_DIR}/shared/bcistream/BCITestMain.cpp )
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
    ${PROJECT_SRC_DIR}/shared
    ${PROJECT_SRC_DIR}/shared/config
    ${PROJECT_SRC_DIR}/shared/bcistream
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
