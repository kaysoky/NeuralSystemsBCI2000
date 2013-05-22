###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including the BCITestMain file.
## SETS:
##       SRC_EXTLIB - Required source files
##       HDR_EXTLIB - Required header files
##       INC_EXTLIB - Include directory
##       LIBDIR_EXTLIB - Library directory
##       LIBS_EXTLIB - required library

SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/shared/bcistream/BCITestMain.cpp
)

SET( INC_EXTLIB 
  ${BCI2000_SRC_DIR}/shared/bcistream
)

SET( EXTLIB_OK TRUE )
