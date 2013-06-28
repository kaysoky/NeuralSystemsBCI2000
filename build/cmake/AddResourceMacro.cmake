###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: A macro to add a binary file as a resource.

# Definition of resource list file:
SET( RESOURCE_NAMES )
SET( RESOURCE_FILES )

# Macro definitions
MACRO( BCI2000_ADD_RESOURCE RESNAME FILE )
  SET( RESOURCE_NAMES
    ${RESOURCE_NAMES}
    ${RESNAME}
  )
  SET( RESOURCE_FILES
    ${RESOURCE_FILES}
    ${FILE}
  )
ENDMACRO()


MACRO( BCI2000_SETUP_RESOURCES SOURCES HEADERS )

  SET( RESOURCE_INCLUDES ${CMAKE_CURRENT_BINARY_DIR} )
  SET( RESOURCES_INC "${CMAKE_CURRENT_BINARY_DIR}/Resources.inc" )

  INCLUDE_DIRECTORIES(
    ${RESOURCE_INCLUDES}
  )
  SET( RESOURCES_H
    ${PROJECT_SRC_DIR}/shared/config/Resources.h
  )
  SET_PROPERTY(
    SOURCE ${RESOURCES_INC}
    PROPERTY GENERATED TRUE
  )
  FILE( WRITE ${RESOURCES_INC} "// File contents created by BCI2000_ADD_RESOURCE -- re-run CMake to update this file\n" )
  SET( RES_CPPS )
  LIST( LENGTH RESOURCE_NAMES NRES )
  IF( NRES )
    MATH( EXPR NRES_1 "${NRES} - 1" )
    FOREACH( IDX RANGE ${NRES_1} )
      LIST( GET RESOURCE_NAMES IDX RESNAME )
      LIST( GET RESOURCE_FILES IDX FILE )
      FILE( RELATIVE_PATH RES_CPP ${PROJECT_SRC_DIR} ${FILE} )
      SET( RES_CPP ${CMAKE_CURRENT_BINARY_DIR}/${RES_CPP}.cpp )
      GET_FILENAME_COMPONENT( RES_CPP ${RES_CPP} ABSOLUTE)
      SET( RES_CPPS
        ${RES_CPPS}
        "${RES_CPP}"
      )
      IF( VERBOSE_CONFIG )
        MESSAGE( "--- Adding resource: " ${RESNAME} )
      ENDIF()
      FILE( APPEND ${RESOURCES_INC} "\#include \"${RES_CPP}\"\n" )
      ADD_CUSTOM_COMMAND(
        OUTPUT "${RES_CPP}"
        COMMAND ${BCI2000_ROOT_DIR}/build/buildutils/create_resource "${RESNAME}" "${FILE}" > "${RES_CPP}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS create_resource "${FILE}"
        VERBATIM
      )
    ENDFOREACH()
  ENDIF( NRES )
  SET( ${SOURCES}
    ${${SOURCES}}
    ${RES_CPPS}
  )
  SET( ${HEADERS}
    ${${HEADERS}}
    ${RESOURCES_INC}
    ${RESOURCES_H}
    ${RESOURCE_FILES}
  )
  SOURCE_GROUP( "Generated\\BCI2000 Framework" FILES ${RESOURCES_INC} )
  SOURCE_GROUP( "Generated\\BCI2000 Framework\\Resources" FILES ${RES_CPPS} )

ENDMACRO()
