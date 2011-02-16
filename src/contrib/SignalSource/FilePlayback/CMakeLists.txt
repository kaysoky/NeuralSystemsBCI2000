###########################################################################
## $Id: CMakeLists.txt 2624 2009-12-02 02:06:03Z gmilsap $
## Authors: griffin.milsap@gmail.com
## Description: Build information for FilePlayback in Qt

# Set the executable name
SET( EXECUTABLE_NAME FilePlayback)

# Set the project specific sources
SET( SRC_PROJECT
  FilePlayback.cpp
  FilePlaybackADC.cpp
  ../../../shared/fileio/dat/BCI2000FileReader.cpp
)
SET( HDR_PROJECT
  FilePlaybackADC.h
  ../../../shared/fileio/dat/BCI2000FileReader.h
)

# Let the macro know we want to use the Math library in EXTLIB
BCI2000_USE( "MATH" )

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${INC_EXTLIB}"
)
