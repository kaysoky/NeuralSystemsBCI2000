###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}SignalProcessing/" )

# Recurse down into all project subdirectories
ADD_SUBDIRECTORY( AR )
ADD_SUBDIRECTORY( P3 )
ADD_SUBDIRECTORY( FFT )
ADD_SUBDIRECTORY( Matlab )
ADD_SUBDIRECTORY( Dummy )
ADD_SUBDIRECTORY( Spectral )

