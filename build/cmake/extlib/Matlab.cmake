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