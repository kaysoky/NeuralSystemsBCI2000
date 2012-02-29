###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Build information for the command line tools in cmake

# Format conversion tools from the cmdline directory

BCI2000_ADD_CMDLINE_CONVERTER( bci_dat2stream )
BCI2000_ADD_CMDLINE_CONVERTER( bci_stream2asc )
BCI2000_ADD_CMDLINE_CONVERTER( bci_decimate   )
BCI2000_ADD_CMDLINE_CONVERTER( bci_prm2stream )
BCI2000_ADD_CMDLINE_CONVERTER( bci_stream2mat )
BCI2000_ADD_CMDLINE_CONVERTER( bci_stream2prm )
BCI2000_ADD_CMDLINE_CONVERTER( bci_stream2table )


# Filters residing in other directories.

SET( SIGPROC_DIR ${BCI2000_ROOT_DIR}/src/shared/modules/signalprocessing )

BCI2000_ADD_CMDLINE_FILTER( ARFilter              FROM ${SIGPROC_DIR} INCLUDING "MATH"
                            EXTRA_SOURCES ${SIGPROC_DIR}/ARSpectrum.cpp
                                          ${SIGPROC_DIR}/WindowingFilter.cpp
                                          ${SIGPROC_DIR}/SpectrumThread.cpp
                                          ${SIGPROC_DIR}/ThreadedFilter.cpp
                            EXTRA_HEADERS ${SIGPROC_DIR}/ARSpectrum.h
                                          ${SIGPROC_DIR}/WindowingFilter.h
                                          ${SIGPROC_DIR}/SpectrumThread.h
                                          ${SIGPROC_DIR}/ThreadedFilter.h )
BCI2000_ADD_CMDLINE_FILTER( SpectralEstimator     FROM ${SIGPROC_DIR} INCLUDING "MATH" "FFT"
                            EXTRA_SOURCES ${SIGPROC_DIR}/ARSpectrum.cpp
                                          ${SIGPROC_DIR}/FFTSpectrum.cpp
                                          ${SIGPROC_DIR}/WindowingFilter.cpp
                                          ${SIGPROC_DIR}/SpectrumThread.cpp
                                          ${SIGPROC_DIR}/ThreadedFilter.cpp
                            EXTRA_HEADERS ${SIGPROC_DIR}/ARSpectrum.h
                                          ${SIGPROC_DIR}/FFTSpectrum.cpp
                                          ${SIGPROC_DIR}/WindowingFilter.h
                                          ${SIGPROC_DIR}/SpectrumThread.h
                                          ${SIGPROC_DIR}/ThreadedFilter.h )
BCI2000_ADD_CMDLINE_FILTER( ComplexDemodulator    FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( ConditionalIntegrator FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( ExpressionFilter      FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( LinearClassifier      FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( LPFilter              FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( Normalizer            FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( P3TemporalFilter      FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( SpatialFilter         FROM ${SIGPROC_DIR}
                            EXTRA_SOURCES ${SIGPROC_DIR}/SpatialFilterGroup.cpp
                            EXTRA_HEADERS ${SIGPROC_DIR}/SpatialFilterGroup.h     )
BCI2000_ADD_CMDLINE_FILTER( StateTransform        FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( TransmissionFilter    FROM ${BCI2000_ROOT_DIR}/src/shared/modules/signalsource )
BCI2000_ADD_CMDLINE_FILTER( FFTFilter             FROM ${SIGPROC_DIR} INCLUDING "FFT" )


# The MatlabFilter must be listed last, else all filter executables will depend on libeng and libmx.

BCI2000_ADD_CMDLINE_FILTER( MatlabFilter          FROM ${BCI2000_ROOT_DIR}/src/core/SignalProcessing/Matlab INCLUDING "MATLAB"
                            EXTRA_SOURCES ${BCI2000_ROOT_DIR}/src/core/SignalProcessing/Matlab/MatlabWrapper.cpp
                            EXTRA_HEADERS ${BCI2000_ROOT_DIR}/src/core/SignalProcessing/Matlab/MatlabWrapper.h   )
