#################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: User-configurable build options

# General build options
OPTION( USE_DYNAMIC_IMPORTS "Set to OFF in case of problems with imports from 3rd party DLLs" ON )
IF( USE_DYNAMIC_IMPORTS )
  ADD_DEFINITIONS( -DDYNAMIC_IMPORTS )
ENDIF()
IF( CMAKE_SYSTEM_PROCESSOR MATCHES ".*86" AND CMAKE_SIZEOF_VOID_P EQUAL 4 )
  OPTION( USE_SSE2 "Set to OFF when building for older processors" ON )
ELSE()
  SET( USE_SSE2 OFF )
ENDIF()

SET( vars_
  USE_DYNAMIC_IMPORTS
  USE_SSE2
  USE_PRECOMPILED_HEADERS
)  
FOREACH( var_ ${vars_} )
  SET( BUILD_CONFIG "${BUILD_CONFIG} ${var_}:${${var_}}" )
ENDFOREACH()

# Whether to include certain projects
OPTION( BUILD_DEMOS "Build demo projects" OFF )
OPTION( BUILD_ALL_TESTS "Build ${PROJECT_NAME} tests plus extlib tests" OFF )
OPTION( BUILD_TESTS "Build ${PROJECT_NAME} executable tests" OFF )

SET( mod_ "core" )
IF( BUILD_CONTRIB )
  LIST( APPEND mod_ "contrib" )
ENDIF()
IF( BUILD_MODULES )
  LIST( APPEND mod_ "SignalSource" )
ENDIF()
IF( BUILD_BCPY2000 )
  LIST( APPEND mod_ "BCPy2000" )
ENDIF()
SET( BUILD_MODULES "${mod_}" CACHE STRING "List of module subdirectory matches" )

SET( BUILD_DISTRIBUTION lean CACHE STRING "Type of distribution: core, contrib, lean" )
IF( BUILD_DISTRIBUTION STREQUAL core )
  SET( BUILD_MODULES core )
ELSEIF( BUILD_DISTRIBUTION STREQUAL contrib )
  SET( BUILD_MODULES core;contrib )
ENDIF()

IF( BUILD_ALL_TESTS )
  SET( BUILD_TESTS ON )
ENDIF()
IF( BUILD_TESTS )
  ENABLE_TESTING()
  ADD_CUSTOM_TARGET( RUN_TESTS_VERBOSE ctest -VV --output-on-failure )
  SET_PROPERTY( TARGET RUN_TESTS_VERBOSE PROPERTY FOLDER Tests )
  ADD_TEST(
    NAME BCI2000Executables
    COMMAND BCI2000Shell ${BCI2000_ROOT_DIR}/build/buildutils/tests/RunTests.bciscript 0
  )
  SET( BUILD_DEMOS ON )
ELSE()
  ADD_DEFINITIONS( -DDISABLE_BCITEST )
ENDIF()

# Buildtests for external libraries
SET( buildtests_ SNDFILE PORTAUDIO )
FOREACH( test_ ${buildtests_} )
  OPTION( ${test_}_BUILD_TESTS "Enable build tests for the ${test_} library" OFF )
  MARK_AS_ADVANCED( FORCE ${test_}_BUILD_TESTS )
  IF( BUILD_ALL_TESTS )
    SET( ${test_}_BUILD_TESTS ON )
  ENDIF()
  IF( ${test_}_BUILD_TESTS )
    ENABLE_TESTING()
  ENDIF()
ENDFOREACH()

# Handling of src/private
IF( EXISTS ${BCI2000_SRC_DIR}/private )
  IF( $ENV{BCI2000_NO_PRIVATE} )
    SET( onoff_ OFF )
  ELSE()
    SET( onoff_ ON )
  ENDIF()
  OPTION( BUILD_PRIVATE "Build contents of src/private directory" ${onoff_} )
ENDIF()

# MSVC specific
IF( MSVC )
  OPTION( BUILD_MFC "Build modules that use MFC" OFF )
ELSE()
  SET( BUILD_MFC OFF )
ENDIF()

SET( folders_ ON )
IF( MSVC AND MSVC_VERSION LESS 1600 AND NOT BUILD_MFC )
  SET( folders_ OFF ) # VS2008 express will not open a folders-enabled solution,
                      # VS2010 == 1600 will ignore folders
ENDIF()
OPTION( BUILD_USE_SOLUTION_FOLDERS "Enable target group folders (does not work with non-express VS)" ${folders_} )

