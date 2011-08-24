###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including alglib in a project
## SETS:
##       SRC_EXTLIB - Required source files for the alglib
##       HDR_EXTLIB - Required header files for the alglib
##       INC_EXTLIB - Include directory for the alglib
##       Also defines source groups for the ALGLIB files

# Define the source files
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/alglib/ap.cpp
  ${BCI2000_SRC_DIR}/extlib/alglib/betacf.cpp
  ${BCI2000_SRC_DIR}/extlib/alglib/betai.cpp
  ${BCI2000_SRC_DIR}/extlib/alglib/blas.cpp
  ${BCI2000_SRC_DIR}/extlib/alglib/gammln.cpp
  ${BCI2000_SRC_DIR}/extlib/alglib/qr.cpp
  ${BCI2000_SRC_DIR}/extlib/alglib/reflections.cpp
  ${BCI2000_SRC_DIR}/extlib/alglib/remmean.cpp
  ${BCI2000_SRC_DIR}/extlib/alglib/stepcalc.cpp
  ${BCI2000_SRC_DIR}/extlib/alglib/stepnext.cpp
  ${BCI2000_SRC_DIR}/extlib/alglib/stepwisefit.cpp
  ${BCI2000_SRC_DIR}/extlib/alglib/tcdf.cpp
)

# Define the headers
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/alglib/ap.h
  ${BCI2000_SRC_DIR}/extlib/alglib/apvt.h
  ${BCI2000_SRC_DIR}/extlib/alglib/betacf.h
  ${BCI2000_SRC_DIR}/extlib/alglib/betai.h
  ${BCI2000_SRC_DIR}/extlib/alglib/blas.h
  ${BCI2000_SRC_DIR}/extlib/alglib/gammln.h
  ${BCI2000_SRC_DIR}/extlib/alglib/qr.h
  ${BCI2000_SRC_DIR}/extlib/alglib/reflections.h
  ${BCI2000_SRC_DIR}/extlib/alglib/remmean.h
  ${BCI2000_SRC_DIR}/extlib/alglib/stepcalc.h
  ${BCI2000_SRC_DIR}/extlib/alglib/stepnext.h
  ${BCI2000_SRC_DIR}/extlib/alglib/stepwisefit.h
  ${BCI2000_SRC_DIR}/extlib/alglib/tcdf.h
)

# Define the include directory
SET( INC_EXTLIB ${BCI2000_SRC_DIR}/extlib/alglib )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\alglib FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\alglib FILES ${HDR_EXTLIB} )

# Set success
SET( EXTLIB_OK TRUE )
