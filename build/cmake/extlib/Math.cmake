###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the EXTLIB Math libs
## SETS:
##       SRC_EXTLIB_MATH - Required source files for the math library
##       HDR_EXTLIB_MATH - Required header files for the math library
##       INC_EXTLIB_MATH - Include directory for the math library
##       Also defines source groups for the math files

# Define the source files
SET( SRC_EXTLIB_MATH
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.cpp
)

# Define the headers
SET( HDR_EXTLIB_MATH
  ${BCI2000_SRC_DIR}/extlib/math/Detrend.h
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.h
  ${BCI2000_SRC_DIR}/extlib/math/IIRFilter.h
  ${BCI2000_SRC_DIR}/extlib/math/LinearPredictor.h
  ${BCI2000_SRC_DIR}/extlib/math/MEMPredictor.h
  ${BCI2000_SRC_DIR}/extlib/math/Polynomials.h
  ${BCI2000_SRC_DIR}/extlib/math/TransferSpectrum.h 
)

# Define the include directory
SET( INC_EXTLIB_MATH ${BCI2000_SRC_DIR}/extlib/math )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\math FILES ${SRC_EXTLIB_MATH} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\math FILES ${HDR_EXTLIB_MATH} )

# Set success
SET( MATH_OK TRUE )