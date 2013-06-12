#################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: User-configurable build options

OPTION( VERBOSE_CONFIG "Set to ON to receive more detailed information during configuration" OFF )

OPTION( USE_DYNAMIC_IMPORTS "Set to OFF in case of problems with imports from 3rd party DLLs" ON )
IF( USE_DYNAMIC_IMPORTS )
  ADD_DEFINITIONS( -DDYNAMIC_IMPORTS )
ENDIF()
IF( CMAKE_SYSTEM_PROCESSOR MATCHES ".*86" AND CMAKE_SIZEOF_VOID_P EQUAL 4 )
  OPTION( USE_SSE2 "Set to OFF when building for older processors" ON )
ELSE()
  SET( USE_SSE2 OFF )
ENDIF()
OPTION( USE_PRECOMPILED_HEADERS "Set to OFF in case of linking or runtime problems" ON )

OPTION( BUILD_TOOLS "Build BCI2000 tools" ON )
OPTION( BUILD_CONTRIB "Build contrib modules" ON )
IF( MSVC )
  OPTION( BUILD_MFC "Build modules that use MFC" OFF )
  OPTION( BUILD_USE_SOLUTION_FOLDERS "Enable target group folders in VS (non-express)" ${BUILD_MFC} )
ELSE()
  SET( BUILD_MFC OFF )
ENDIF()
OPTION( BUILD_DEMOS "Build demo projects" OFF )
OPTION( BUILD_ALL_TESTS "Build BCI2000 tests plus extlib tests" OFF )
OPTION( BUILD_TESTS "Build BCI2000 executable tests" OFF )
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
