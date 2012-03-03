###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the FFT lib
## SETS:
##       SRC_EXTLIB - Required source files for the fft library
##       HDR_EXTLIB - Required header files for the fft library
##       INC_EXTLIB - Include directory for the fft library
##       Also defines source groups for the fft files

# Set success
SET( EXTLIB_OK TRUE )

# Define the source files
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/fftlib/FFTLibWrap.cpp
)

# Define the headers
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/fftlib/FFTLibWrap.h
)

# Define the include directory
SET( INC_EXTLIB ${BCI2000_SRC_DIR}/extlib/fftlib )


# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\fftlib FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\fftlib FILES ${HDR_EXTLIB} )
