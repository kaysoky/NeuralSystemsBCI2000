###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Contains a macro for creating a commandline application

MACRO( BCI2000_ADD_TOOLS_CMDLINE NAME SOURCES HEADERS REQUESTQT )
       
  # DEBUG
  MESSAGE( "-- Adding Commandline Project: " ${NAME} )
  #MESSAGE( "${NAME} sources: ${SOURCES}" )
  #MESSAGE( "${NAME} headers: ${HEADERS}" )
  
  # Generate the required framework
  INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/MinimalFramework.cmake )
  
  SET( SOURCES
    ${SOURCES}
    ${BCI2000_SRC_DIR}/core/Tools/cmdline/bci_tool.cpp
  )
  SET( HEADERS
    ${HEADERS}
    ${BCI2000_SRC_DIR}/core/Tools/cmdline/bci_tool.h
  )
  
  # Add in the appropriate error handling module
  SET( SRC_SHARED_BCISTREAM 
    ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_tool.cpp
  )
  SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES ${SRC_SHARED_BCISTREAM} )
  SET( SRC_BCI2000_FRAMEWORK
    ${SRC_BCI2000_FRAMEWORK}
    ${SRC_SHARED_BCISTREAM}
  )
  
  # Set the Project Source Groups
  SOURCE_GROUP( Source\\Project FILES ${SOURCES} )
  SOURCE_GROUP( Headers\\Project FILES ${HEADERS} )
  
  # Add in external required libraries
  BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS )
  
  # If we're building a Qt project, we need to automoc the sources, generating new files
  IF( BORLAND )
     SET( USEQT "FALSE" )
  ELSEIF( ${REQUESTQT} OR WIN32 )
	 SET( USEQT "TRUE" )  # on Windows, use Qt whether it was requested or not (temporary hack to avoid 23 link errors: unresolved externals from streamsock methods in SockStream.obj: _WSAStartup@8 thru _gethostbyname@4) 
  ELSE( BORLAND )
     SET( USEQT "FALSE" )
  ENDIF( BORLAND )
  IF( ${USEQT} )
    QT4_AUTOMOC( ${SOURCES} )  
    # Include Qt Modules specified elsewhere
    INCLUDE ( ${QT_USE_FILE} )
  ENDIF( ${USEQT} )
  
  # Add Pre-processor defines
  ADD_DEFINITIONS( 
    -DNO_PCHINCLUDES
  )
  
  # Add the executable to the project
  ADD_EXECUTABLE( ${NAME} ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SOURCES} ${HEADERS} )
  
  # Set the output directories
  SET_TARGET_PROPERTIES( ${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/cmdline )
  SET_TARGET_PROPERTIES( ${NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/cmdline )
  SET_TARGET_PROPERTIES( ${NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/cmdline )
  IF( MSVC OR XCODE )
    SET_TARGET_PROPERTIES( ${NAME} PROPERTIES 
    PREFIX "../"
    IMPORT_PREFIX "../" 
    )
  ENDIF( MSVC OR XCODE )
  
  # Link against the Qt/VCL Libraries
  IF( BORLAND )
    TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} ${LIBS} )
  ENDIF( BORLAND )
  IF( ${USEQT} )
    #MESSAGE( "**  ${NAME} uses Qt" )
    TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} ${LIBS} )
  ENDIF( ${USEQT} )

ENDMACRO( BCI2000_ADD_TOOLS_CMDLINE NAME SOURCES HEADERS )

################################################################################################

MACRO( BCI2000_ADD_CMDLINE_CONVERTER NAME )

  SET( SOURCES
    ${NAME}.cpp
    ${BCI2000_SRC_DIR}/core/Tools/cmdline/bci_tool.cpp
  )
  SET( HEADERS
    ${BCI2000_SRC_DIR}/core/Tools/cmdline/bci_tool.h
  )
  BCI2000_ADD_TOOLS_CMDLINE( ${NAME} "${SOURCES}" "${HEADERS}" FALSE )
  
ENDMACRO( BCI2000_ADD_CMDLINE_CONVERTER NAME )

################################################################################################

MACRO(PARSE_ARGUMENTS prefix arg_names option_names)
  SET(DEFAULT_ARGS)
  FOREACH(arg_name ${arg_names})    
    SET(${prefix}_${arg_name})
  ENDFOREACH(arg_name)
  FOREACH(option ${option_names})
    SET(${prefix}_${option} FALSE)
  ENDFOREACH(option)

  SET(current_arg_name DEFAULT_ARGS)
  SET(current_arg_list)
  FOREACH(arg ${ARGN})            
    SET(larg_names ${arg_names})    
    LIST(FIND larg_names "${arg}" is_arg_name)                   
    IF (is_arg_name GREATER -1)
      SET(${prefix}_${current_arg_name} ${current_arg_list})
      SET(current_arg_name ${arg})
      SET(current_arg_list)
    ELSE (is_arg_name GREATER -1)
      SET(loption_names ${option_names})    
      LIST(FIND loption_names "${arg}" is_option)            
      IF (is_option GREATER -1)
	     SET(${prefix}_${arg} TRUE)
      ELSE (is_option GREATER -1)
	     SET(current_arg_list ${current_arg_list} ${arg})
      ENDIF (is_option GREATER -1)
    ENDIF (is_arg_name GREATER -1)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(PARSE_ARGUMENTS)

################################################################################################

MACRO( BCI2000_ADD_CMDLINE_FILTER )

  PARSE_ARGUMENTS(
    CMDLINEFILTER
    "FROM;EXTRA_SOURCES;EXTRA_HEADERS;USING"
    ""
    ${ARGN}
  )
  LIST( GET CMDLINEFILTER_DEFAULT_ARGS 0 NAME )
  LIST( REMOVE_AT CMDLINEFILTER_DEFAULT_ARGS 0 )
  LIST( LENGTH CMDLINEFILTER_DEFAULT_ARGS NARGS )
  IF( ${NARGS} GREATER 0 )
    MESSAGE( "- WARNING: BCI2000_ADD_CMDLINE_FILTER is ignoring extraneous arguments: " ${CMDLINEFILTER_DEFAULT_ARGS} )
  ENDIF( ${NARGS} GREATER 0 )
    
  SET( MAINSTEM ${NAME} )
  IF( NOT ${CMDLINEFILTER_FROM} STREQUAL "" )
    SET( MAINSTEM ${CMDLINEFILTER_FROM}/${MAINSTEM} )
  ENDIF( NOT ${CMDLINEFILTER_FROM} STREQUAL "" )
  #MESSAGE( "want to build command-line filter ${NAME} from ${CMDLINEFILTER_FROM} based on ${MAINSTEM}.cpp" )
  
  SET( SOURCES
    ${MAINSTEM}.cpp
    ${CMDLINEFILTER_EXTRA_SOURCES}
    ${BCI2000_SRC_DIR}/core/Tools/cmdline/bci_tool.cpp
    ${BCI2000_SRC_DIR}/core/Tools/cmdline/bci_filtertool.cpp
  )
  SET( HEADERS
    ${MAINSTEM}.h
    ${CMDLINEFILTER_EXTRA_HEADERS}
    ${BCI2000_SRC_DIR}/core/Tools/cmdline/bci_tool.h
  )

  SET( USEQT FALSE )
  FOREACH( DEPENDENCY ${CMDLINEFILTER_USING} )
    IF( ${DEPENDENCY} STREQUAL QT )
      SET( USEQT TRUE )
    ELSE( ${DEPENDENCY} STREQUAL QT )
      BCI2000_USE( ${DEPENDENCY} )
    ENDIF( ${DEPENDENCY} STREQUAL QT )
  ENDFOREACH( DEPENDENCY )
  
  BCI2000_ADD_TOOLS_CMDLINE( ${NAME} "${SOURCES}" "${HEADERS}" ${USEQT} )

ENDMACRO( BCI2000_ADD_CMDLINE_FILTER )

################################################################################################
