###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the EXTLIB Math libs
## SETS:
##       SRC_EXTLIB - Required source files for the math library
##       HDR_EXTLIB - Required header files for the math library
##       INC_EXTLIB - Include directory for the math library
##       Also defines source groups for the math files

# Define the source files
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.cpp
)

# Define the headers
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/math/Detrend.h
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.h
  ${BCI2000_SRC_DIR}/extlib/math/IIRFilter.h
  ${BCI2000_SRC_DIR}/extlib/math/LinearPredictor.h
  ${BCI2000_SRC_DIR}/extlib/math/MEMPredictor.h
  ${BCI2000_SRC_DIR}/extlib/math/Polynomials.h
  ${BCI2000_SRC_DIR}/extlib/math/TransferSpectrum.h
)


# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\math FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\math FILES ${HDR_EXTLIB} )

SET( SRC_EXTLIB
  ${SRC_EXTLIB}
)

SET( HDR_EXTLIB
  ${HDR_EXTLIB}
)

# Define include directories
SET( INC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/math
)

# Set success
SET( EXTLIB_OK TRUE )