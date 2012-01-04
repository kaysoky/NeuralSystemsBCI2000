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

BCI2000_ADD_CMDLINE_FILTER( ARFilter              FROM ${SIGPROC_DIR} INCLUDING "MATH" # "QT"  # uncomment the "QT" for a threaded speed-up at the expense of requiring the QT framework dylibs to be installed
                            EXTRA_SOURCES ${SIGPROC_DIR}/ARChannel.cpp
                                          ${SIGPROC_DIR}/ARGroup.cpp
                            EXTRA_HEADERS ${SIGPROC_DIR}/ARChannel.h
                                          ${SIGPROC_DIR}/ARGroup.h    )
BCI2000_ADD_CMDLINE_FILTER( ComplexDemodulator    FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( ConditionalIntegrator FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( ExpressionFilter      FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( LinearClassifier      FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( LPFilter              FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( Normalizer            FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( P3TemporalFilter      FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( SpatialFilter         FROM ${SIGPROC_DIR} # USING "QT"  # uncomment USING "QT" for a threaded speed-up at the expense of requiring the QT framework dylibs to be installed
                            EXTRA_SOURCES ${SIGPROC_DIR}/SpatialFilterCell.cpp
                                          ${SIGPROC_DIR}/SpatialFilterGroup.cpp
                            EXTRA_HEADERS ${SIGPROC_DIR}/SpatialFilterCell.h
                                          ${SIGPROC_DIR}/SpatialFilterGroup.h     )
BCI2000_ADD_CMDLINE_FILTER( StateTransform        FROM ${SIGPROC_DIR} )
BCI2000_ADD_CMDLINE_FILTER( TransmissionFilter    FROM ${BCI2000_ROOT_DIR}/src/shared/modules/signalsource )
BCI2000_ADD_CMDLINE_FILTER( FFTFilter             FROM ${SIGPROC_DIR} INCLUDING "FFT" )
# The MatlabFilter must be listed last or all filter executables will depend on libeng and libmx.
BCI2000_ADD_CMDLINE_FILTER( MatlabFilter          FROM ${BCI2000_ROOT_DIR}/src/core/SignalProcessing/Matlab USING "MATLAB"
                            EXTRA_SOURCES ${BCI2000_ROOT_DIR}/src/core/SignalProcessing/Matlab/MatlabWrapper.cpp
                            EXTRA_HEADERS ${BCI2000_ROOT_DIR}/src/core/SignalProcessing/Matlab/MatlabWrapper.h   )
