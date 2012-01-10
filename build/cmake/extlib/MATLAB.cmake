###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the EXTLIB Matlab libs
## SETS:
##       SRC_EXTLIB - Required source files for the matlab library
##       HDR_EXTLIB - Required header files for the matlab library
##       INC_EXTLIB - Include directory for the matlab library
##       Also defines source groups for the matlab files

# Define the source files
SET( SRC_EXTLIB )

IF( WIN32 )

  # Define the headers
  SET( HDR_EXTLIB
    ${BCI2000_SRC_DIR}/extlib/matlab/engine.h
    ${BCI2000_SRC_DIR}/extlib/matlab/mat.h
    ${BCI2000_SRC_DIR}/extlib/matlab/matrix.h
    ${BCI2000_SRC_DIR}/extlib/matlab/mex.h
    ${BCI2000_SRC_DIR}/extlib/matlab/mwdebug.h
  )

  # Define the include directory
  SET( INC_EXTLIB ${BCI2000_SRC_DIR}/extlib/matlab )

  # Set the source groups
  SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\matlab FILES ${SRC_EXTLIB} )
  SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\matlab FILES ${HDR_EXTLIB} )

  # Set success
  SET( EXTLIB_OK TRUE )

ELSE( WIN32 )

  FIND_PROGRAM( MATLAB_EXECUTABLE matlab /usr /var /opt /Applications )
  IF( NOT MATLAB_EXECUTABLE )
    RETURN()
  ENDIF( NOT MATLAB_EXECUTABLE )

  EXECUTE_PROCESS( COMMAND ${MATLAB_EXECUTABLE} -n RESULT_VARIABLE RESULT OUTPUT_VARIABLE MATLAB_CONFIG )
  IF( RESULT )
    RETURN()
  ENDIF( RESULT )

  STRING( REGEX MATCH "MATLAB[ ]+=[ ]+([^\r\n]*).*ARCH[ ]+=[ ]+([^\r\n]*)" IGNORED ${MATLAB_CONFIG} )
  SET( MATLAB_ROOT ${CMAKE_MATCH_1} )
  SET( MATLAB_ARCH ${CMAKE_MATCH_2} )
  SET( MATLAB_LIBDIRS "${MATLAB_ROOT}/bin/${MATLAB_ARCH}" )
  
  FIND_LIBRARY( MATLAB_MEX_LIBRARY mex ${MATLAB_LIBDIRS} NO_DEFAULT_PATH )
  FIND_LIBRARY( MATLAB_MX_LIBRARY mx ${MATLAB_LIBDIRS}  NO_DEFAULT_PATH )
  FIND_LIBRARY( MATLAB_ENG_LIBRARY eng ${MATLAB_LIBDIRS}  NO_DEFAULT_PATH )
  IF( NOT MATLAB_MX_LIBRARY )
    RETURN()
  ENDIF()

  SET( LIBS_EXTLIB
    ${MATLAB_MEX_LIBRARY}
    ${MATLAB_MX_LIBRARY}
    ${MATLAB_ENG_LIBRARY}
  )
  FIND_PATH( MATLAB_INCLUDE_DIR "mex.h" ${MATLAB_ROOT}/extern/include )
  SET( INC_EXTLIB ${MATLAB_INCLUDE_DIR} )

  SET( EXTLIB_OK TRUE )

ENDIF( WIN32 )


