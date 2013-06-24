###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Usage header for BCI2000FrameworkCore library

INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/shared
  ${BCI2000_SRC_DIR}/shared/accessors
  ${BCI2000_SRC_DIR}/shared/bcistream
  ${BCI2000_SRC_DIR}/shared/config
  ${BCI2000_SRC_DIR}/shared/modules
  ${BCI2000_SRC_DIR}/shared/filters
  ${BCI2000_SRC_DIR}/shared/types
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/shared/utils/Expression
  ${BCI2000_SRC_DIR}/shared/fileio
  ${BCI2000_SRC_DIR}/shared/fileio/dat
)

#SET( REGISTRY_NAME CoreRegistry )
#FORCE_INCLUDE_OBJECT( ${REGISTRY_NAME} )

SET( LIBS ${LIBS} BCI2000FrameworkCore )
IF( WIN32 )
  SET( LIBS ${LIBS} ws2_32 )
ENDIF()

IF( NOT DEFINED USE_PRECOMPILED_HEADERS OR USE_PRECOMPILED_HEADERS )
  UNSET( pchsrc_ )
  SET( CORE_PCH PCHIncludes.h )
  IF( MSVC )
    SET( pchsrc_ ${CMAKE_CURRENT_BINARY_DIR}/CorePCH.cpp )
    IF( NOT EXISTS ${pchsrc_} )
      FILE( WRITE "${pchsrc_}" "#include \"${CORE_PCH}\"" )
    ENDIF()
    SET( pchfile_ "$(OutDir)/$(TargetName)_Core.pch" )
    SET_SOURCE_FILES_PROPERTIES(
      ${pchsrc_} PROPERTIES
      COMPILE_FLAGS "/Yc\"${CORE_PCH}\" /Fp\"${pchfile_}\""
      OBJECT_OUTPUTS "\"${pchfile_}\""
    )
    SET( CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} /Yu\"${CORE_PCH}\" /FI\"${CORE_PCH}\" /Fp\"${pchfile_}\""
    )
  ELSEIF( 0 ) # COMPILER_IS_GCC_COMPATIBLE )
    SET( pchsrc_ ${BCI2000_SRC_DIR}/shared/config/${CORE_PCH} )
    IF( NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${CORE_PCH}" )
      FILE( WRITE "${CMAKE_CURRENT_BINARY_DIR}/${CORE_PCH}" "#error Not using precompiled header file." )
    ENDIF()
    SET_SOURCE_FILES_PROPERTIES(
      ${pchsrc_} PROPERTIES
      COMPILE_FLAGS "-x c++ -o \"${CMAKE_CURRENT_BINARY_DIR}/${CORE_PCH}.gch\""
      OBJECT_OUTPUTS "${CORE_PCH}.gch"
    )
    SET( CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -I \"${CMAKE_CURRENT_BINARY_DIR}\" -include ${CORE_PCH}"
    )
  ENDIF()
  IF( pchsrc_ )
    OPTION( USE_PRECOMPILED_HEADERS "Set to OFF in case of linking or runtime problems" ON )
    LIST( APPEND SRC_BCI2000_FRAMEWORK ${pchsrc_} )
    SOURCE_GROUP( "Generated\\BCI2000 Framework" FILES ${pchsrc_} )
    ADD_DEFINITIONS( -DPRECOMPILED_HEADERS=1 )
  ENDIF()
ENDIF()
