###########################################################################
## $Id$
## Authors:
## Description: Build information for the customFIRFiltering project
##              
##              
###########################################################################

ADD_SUBDIRECTORY( cmdline )

# Set the executable name
SET( EXECUTABLE_NAME HilbertSignalProcessing )

# Set the project specific sources
SET( SRC_PROJECT
  HilbertSignalProcessing.cpp            # You probably will not need to edit this file
  PipeDefinition.cpp         # but you should certainly edit this one
  CustomFIRFilter.cpp
  CustomIIRFilter.cpp
  CustomIIR.cpp
  HilbertFilter.cpp
  DiffFilter.cpp
)

SET( HDR_PROJECT
  CustomFIRFilter.h
  CustomIIRFilter.h
  CustomIIR.h
  HilbertFilter.h
  DiffFilter.h
)

# Use the BCI2000_INCLUDE macro if you need to link with frameworks from /src/extlib:
BCI2000_INCLUDE( "MATH" )

# We're done. Add the signal processing module to the Makefile or compiler project file:
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)


