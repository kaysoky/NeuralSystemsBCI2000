###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000 and CMake

# Recurse down into all project subdirectories
ADD_SUBDIRECTORY( FieldTripBuffer )
ADD_SUBDIRECTORY( FIR )
ADD_SUBDIRECTORY( PeakDetector )
ADD_SUBDIRECTORY( SW )
ADD_SUBDIRECTORY( Coherence )
ADD_SUBDIRECTORY( CoherenceFFT )
ADD_SUBDIRECTORY( Statistics )
ADD_SUBDIRECTORY( HilbertSignalProcessing )

# Command line filters cannot be defined in their associated signal processing 
# modules' CMakeLists.txt, so they are defined here:
BCI2000_ADD_CMDLINE_FILTER( CoherenceFilter FROM Coherence )

SET( SIGPROC ../../shared/modules/signalprocessing )
INCLUDE_DIRECTORIES( Coherence )
INCLUDE_DIRECTORIES( ${SIGPROC} )
BCI2000_ADD_CMDLINE_FILTER(
  CoherenceFFTFilter FROM CoherenceFFT
  INCLUDING "FFT"
  EXTRA_SOURCES Coherence/CoherenceFilter.cpp
                ${SIGPROC}/FFTFilter.cpp
  EXTRA_HEADERS Coherence/CoherenceFilter.h
                ${SIGPROC}/FFTFilter.h
)

