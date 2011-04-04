###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the FFT lib
## SETS:
##       SRC_EXTLIB_FFT - Required source files for the fft library
##       HDR_EXTLIB_FFT - Required header files for the fft library
##       INC_EXTLIB_FFT - Include directory for the fft library
##       Also defines source groups for the fft files

# Set success
SET( FFT_OK TRUE )

# Define the source files
SET( SRC_EXTLIB_FFT
  ${BCI2000_SRC_DIR}/extlib/fftlib/FFTLibWrap.cpp
)

# Define the headers
SET( HDR_EXTLIB_FFT
  ${BCI2000_SRC_DIR}/extlib/fftlib/FFTLibWrap.h
)

# Define the include directory
SET( INC_EXTLIB_FFT ${BCI2000_SRC_DIR}/extlib/fftlib )

IF( APPLE )
  SET( INC_EXTLIB_FFT
    ${INC_EXTLIB_FFT}
    /sw/include
  )
ENDIF( APPLE )

IF( APPLE )
  SET( LIBDIR_EXTLIB_FFT /sw/lib )
ENDIF( APPLE )

# Set Libs required
IF( NOT WIN32 )
  SET( LIBS_EXTLIB_FFT libfftw3.a )
  
  FIND_LIBRARY( FFTW3_LIBRARY     NAMES fftw3     PATHS ${LIBDIR_EXTLIB_FFT} )
  IF( ${FFTW3_LIBRARY}   STREQUAL   "FFTW3_LIBRARY-NOTFOUND" )
    MESSAGE( "- WARNING: failed to find FFTW3 library" )
    SET( FFT_OK   FALSE )
  ENDIF( ${FFTW3_LIBRARY}  STREQUAL  "FFTW3_LIBRARY-NOTFOUND" )
  
ENDIF( NOT WIN32 )


# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\fftlib FILES ${SRC_EXTLIB_FFT} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\fftlib FILES ${HDR_EXTLIB_FFT} )
