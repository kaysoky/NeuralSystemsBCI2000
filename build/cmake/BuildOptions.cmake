#################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: User-configurable build options

# Create project defines
SET( PROJECT_VERSION "${PROJECT_VER_MAJOR}.${PROJECT_VER_MINOR}.${PROJECT_VER_PATCH}" )
STRING( TOLOWER "${PROJECT_NAME}" domain_ )
SET( PROJECT_DOMAIN "${domain_}.org" CACHE STRING "Domain name of main project" )
SET( PROJECT_SEARCH_ENGINE "google" CACHE STRING "Name of search engine for searching project domain" )
ADD_DEFINITIONS(
  -DPROJECT_NAME="${PROJECT_NAME}"
  -DPROJECT_DOMAIN="${PROJECT_DOMAIN}"
  -DPROJECT_VERSION="${PROJECT_VERSION}"
  -DWEBSEARCH_DOMAIN="www.${PROJECT_SEARCH_ENGINE}.com"
)
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
# Determine host and user name
IF( NOT BUILD_USER )
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
  SET( BUILD_USER "${user_}@${site_}" CACHE STRING "Build user ID, may be used to track down the origin of executables" )
  UNSET( site_ CACHE )
ENDIF()
ADD_DEFINITIONS(
  -DBUILD_USER="${BUILD_USER}"
)

# General build options
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

OPTION( BUILD_TOOLS "Build ${PROJECT_NAME} tools" ON )
OPTION( BUILD_CONTRIB "Build contrib modules" ON )
OPTION( BUILD_DEMOS "Build demo projects" OFF )
OPTION( BUILD_ALL_TESTS "Build ${PROJECT_NAME} tests plus extlib tests" OFF )
OPTION( BUILD_TESTS "Build ${PROJECT_NAME} executable tests" OFF )
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
