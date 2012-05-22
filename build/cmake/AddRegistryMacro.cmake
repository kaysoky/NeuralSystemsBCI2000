###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: A macro to add a registry function to a target.
##   In BCI2000, a registry function is a function that references objects
##   created by the registration macros defined in src/config/BCIRegistry.h.
##   Such a function is needed with MSVC to make sure that registration
##   objects contained in a static library will not get stripped during
##   the link stage.
##
##   BCI2000_ADD_REGISTRY( RegistryName SourcesVar DependsVar )
##   A registry with given name is added to the list of source files in
##   SourcesVar, and additional dependencies are returned in the DependsVar
##   variable.

MACRO( BCI2000_ADD_REGISTRY NAME SOURCES DEPENDS )

  SET( REGISTRY_INC ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.inc )
  
  ADD_CUSTOM_COMMAND(
    OUTPUT ${REGISTRY_INC}
    COMMAND ${CMAKE_COMMAND} -E echo "// Generated file -- do not edit" > ${REGISTRY_INC}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${INVENTORY_INC} # rebuild on every CMake run (which re-creates the inventory)
    VERBATIM
  )
  FOREACH( CPP_FILE ${SRC_BCI2000_FRAMEWORK} )
    ADD_CUSTOM_COMMAND(
      OUTPUT ${REGISTRY_INC}
      COMMAND ${BCI2000_ROOT_DIR}/build/buildutils/extract_registry < ${CPP_FILE} >> ${REGISTRY_INC}
      DEPENDS ${CPP_FILE}
      APPEND
      VERBATIM
    )
  ENDFOREACH()

  SET( REGISTRY_CPP
    ${BCI2000_SRC_DIR}/shared/config/BCIRegistry.cpp
  )
  SET_PROPERTY( # Make sure the registry cpp file is re-compiled each time the registry file has changed.
    SOURCE ${REGISTRY_CPP}
    APPEND PROPERTY OBJECT_DEPENDS "${REGISTRY_INC};${INVENTORY_INC}"
  )
  SET( ${SOURCES}
    ${${SOURCES}}
    ${INVENTORY_INC}
    ${REGISTRY_INC}
    ${REGISTRY_CPP}
  )
  SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\config FILES ${REGISTRY_CPP} )
  SOURCE_GROUP( Generated FILES ${REGISTRY_INC} ${INVENTORY_INC} )

  ADD_DEFINITIONS( "-DREGISTRY_NAME=${NAME}" )

  SET( ${DEPENDS}
    ${${DEPENDS}}
    extract_registry
  )

ENDMACRO()


MACRO( FORCE_INCLUDE_OBJECT NAME )
  IF( MSVC )
    IF( CMAKE_CL_64 )
      SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /include:${NAME}" )
    ELSE()
      SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /include:_${NAME}" )
    ENDIF()
  ELSEIF( CMAKE_COMPILER_IS_GNUCXX )
    IF( WIN32 )
      SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-u,_${NAME}" )
    ELSE()
      SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-u,${NAME}" )
    ENDIF()
  ENDIF()
ENDMACRO()

