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
    MESSAGE( FATAL_ERROR "Could not find Matlab executable." )
  ENDIF( NOT MATLAB_EXECUTABLE )
  EXECUTE_PROCESS( COMMAND ${MATLAB_EXECUTABLE} -n RESULT_VARIABLE RESULT OUTPUT_VARIABLE MATLAB_CONFIG )
  IF( RESULT )
    MESSAGE( FATAL_ERROR "Could not obtain Matlab installation details." )
  ENDIF( RESULT )
  STRING( REGEX MATCH "MATLAB[ ]+=[ ]+([^\r\n]*).*LD_LIBRARY_PATH[ ]+=[ ]+([^\r\n]*)" IGNORED ${MATLAB_CONFIG} )
  SET( MATLAB_ROOT ${CMAKE_MATCH_1} )
  SET( LIBDIR_EXTLIB ${CMAKE_MATCH_2} )
  STRING( REPLACE ":" ";" LIBDIR_EXTLIB ${LIBDIR_EXTLIB} )
  FIND_PATH( MATLAB_INCLUDE_DIR "mex.h" ${MATLAB_ROOT}/extern/include )  
  SET( INC_EXTLIB ${MATLAB_INCLUDE_DIR} )
  SET( LIBS_EXTLIB mex mx eng )
  SET( EXTLIB_OK TRUE )

ENDIF( WIN32 )


