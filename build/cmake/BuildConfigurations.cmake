###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets a number of default build options for makefile
## generators.

#
# Runs compiler with "-dumpversion" (taken from FindBoost.cmake, jm)
#
FUNCTION(COMPILER_DUMPVERSION _OUTPUT_VERSION)

  EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
    ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion
    OUTPUT_VARIABLE _bci_COMPILER_VERSION
  )

  SET(${_OUTPUT_VERSION} ${_bci_COMPILER_VERSION} PARENT_SCOPE)
ENDFUNCTION()

IF( MINGW )
  COMPILER_DUMPVERSION( GCC_VERSION )
  IF( NOT USE_STD_QT AND GCC_VERSION VERSION_LESS 4.0 )
    MESSAGE( FATAL_ERROR
      "When building against the Qt libraries in the BCI2000 source tree, "
      "a 4.x version of gcc is required. --- Your gcc version is " ${GCC_VERSION}
      " --- More information may be found at "
      "http://doc.bci2000.org/index.php/Programming_Reference:Build_System"
    )
  ELSE( NOT USE_STD_QT AND GCC_VERSION VERSION_LESS 4.0 )
    MESSAGE( "Your gcc version is " ${GCC_VERSION} )
  ENDIF( NOT USE_STD_QT AND GCC_VERSION VERSION_LESS 4.0 )
ENDIF( MINGW )


# This will confuse non g++ compilers
IF( CMAKE_COMPILER_IS_GNUCXX )

  # Set a default build type for single-configuration
  # CMake generators if no build type is set.
  IF( NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE RELEASE )
  ENDIF( NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE )

  # Select flags.
  SET( CMAKE_CXX_FLAGS 
      "${CMAKE_CXX_FLAGS} -include \"${BCI2000_SRC_DIR}/shared/config/gccprefix.h\""
  )

  SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
  SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
  
  IF( WIN32 )
    SET(CMAKE_EXE_LINKER_FLAGS 
    "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc"
    )
  ENDIF( WIN32 )

ENDIF( CMAKE_COMPILER_IS_GNUCXX )

# MSVC specific flags
IF( MSVC )
  SET( CMAKE_CXX_FLAGS 
    "${CMAKE_CXX_FLAGS} /EHsc /W3 /wd4355 /wd4800"
  )
  ADD_DEFINITIONS(
    -DNOMINMAX
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_NONSTDC_NO_WARNINGS
    -D_SCL_SECURE_NO_WARNINGS
  )
  # Adjust flags such that by default programs are built statically against the MSVC runtime;
  # save defaults as "_DYNAMIC" for use with MFC-based projects.
  # Support for statically linking MFC appears to be broken in CMake, see
  # http://www.cmake.org/Wiki/CMake_FAQ#How_to_use_MFC_with_CMake, so
  # we need to go back to dynamic linking for MFC.
  SET( CXX_FLAG_VARS
         CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
         CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
  )
  FOREACH( flag_var ${CXX_FLAG_VARS} )
    SET( ${flag_var}_DYNAMIC "${${flag_var}}" )
    IF( ${flag_var} MATCHES "/MD" )
      STRING( REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}" )
    ENDIF( ${flag_var} MATCHES "/MD" )
    SET( ${flag_var}_STATIC "${${flag_var}}" )
  ENDFOREACH( flag_var )

  SET( LINKER_FLAG_VARS
        CMAKE_EXE_LINKER_FLAGS
        CMAKE_SHARED_LINKER_FLAGS
        CMAKE_MODULE_LINKER_FLAGS 
  )
  FOREACH( flag_var ${LINKER_FLAG_VARS} )
     SET( ${flag_var}_DYNAMIC "${${flag_var}}" )
     SET( ${flag_var} "${${flag_var}} /NODEFAULTLIB:msvcrt /NODEFAULTLIB:msvcrtd" )
     SET( ${flag_var}_STATIC "${${flag_var}}" )
  ENDFOREACH( flag_var )
  
  SET( FLAG_VARS ${CXX_FLAG_VARS} ${LINKER_FLAG_VARS} )

ENDIF( MSVC )

# Build the compiler description string.
IF( MSVC )
  SET(
    COMPILER_NAME_ "MSVC"
    )
ELSEIF( BORLAND )
  SET(
    COMPILER_NAME_ "Borland"
    )
ELSEIF( CMAKE_COMPILER_IS_GNUCXX )
  COMPILER_DUMPVERSION( GCC_VERSION )
  IF( APPLE )
    SET( COMPILER_SUB "apple" )
  ELSEIF( MINGW )
    SET( COMPILER_SUB "mingw" )
  ELSEIF( CYGWIN )
    SET( COMPILER_SUB "cygwin" )
  ELSE()
    SET( COMPILER_SUB "unknown" )
  ENDIF()
  
  SET(
    COMPILER_NAME_ "gcc-${COMPILER_SUB}-${GCC_VERSION}"
    )
ELSE()
  SET(
    COMPILER_NAME_ "unknown"
    )
ENDIF()
ADD_DEFINITIONS(
  -DCOMPILER_NAME="${COMPILER_NAME_}"
  )
  
# Add global definitions.
ADD_DEFINITIONS( -DNO_PCHINCLUDES )
ADD_DEFINITIONS( -D_USE_MATH_DEFINES )
