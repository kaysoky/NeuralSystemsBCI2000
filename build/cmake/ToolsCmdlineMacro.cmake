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
  INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )
  
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
  BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS FAILED )
  
  # If we're building a Qt project, we need to automoc the headers, generating new files
  SET( USEQT ${REQUESTQT} )
  IF( BORLAND )
     SET( USEQT "FALSE" )
  ENDIF( BORLAND )
  IF( ${USEQT} )
    QT4_AUTOMOC( ${HEADERS} )  
  ENDIF( ${USEQT} )
  
  # Set output directories
  SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/tools/cmdline" )

  IF( NOT FAILED )
    # Add the executable to the project
    ADD_EXECUTABLE( ${NAME} ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SOURCES} ${HEADERS} )
  
    # Add Pre-processor defines
    IF( ${USEQT} )
      SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DUSE_QT" )
    ENDIF( ${USEQT} )

    # Link against the Qt/VCL Libraries
    IF( BORLAND )
      TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} ${LIBS} )
    ELSEIF( ${USEQT} )
      MESSAGE( "-- (NB: ${NAME} is using Qt)" )
      TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} ${LIBS} )
    ELSE()
      TARGET_LINK_LIBRARIES( ${NAME} ${LIBS} )
    ENDIF()

    # Set the project build folder
    SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )
  ENDIF( NOT FAILED )

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
    "FROM;EXTRA_SOURCES;EXTRA_HEADERS;USING;INCLUDING"
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
    ${BCI2000_SRC_DIR}/core/Tools/cmdline/bci_tool.cpp
    ${BCI2000_SRC_DIR}/core/Tools/cmdline/bci_filtertool.cpp
    ${CMDLINEFILTER_EXTRA_SOURCES}
  )
  SET( HEADERS
    ${MAINSTEM}.h
    ${BCI2000_SRC_DIR}/core/Tools/cmdline/bci_tool.h
    ${CMDLINEFILTER_EXTRA_HEADERS}
  )

  SET( USEQT FALSE )
  FOREACH( DEPENDENCY ${CMDLINEFILTER_USING} ${CMDLINEFILTER_INCLUDING} )
    IF( ${DEPENDENCY} STREQUAL QT )
      SET( USEQT TRUE )
    ELSE( ${DEPENDENCY} STREQUAL QT )
      BCI2000_INCLUDE( ${DEPENDENCY} )
    ENDIF( ${DEPENDENCY} STREQUAL QT )
  ENDFOREACH( DEPENDENCY )
  
  BCI2000_ADD_TOOLS_CMDLINE( ${NAME} "${SOURCES}" "${HEADERS}" ${USEQT} )

ENDMACRO( BCI2000_ADD_CMDLINE_FILTER )

################################################################################################
