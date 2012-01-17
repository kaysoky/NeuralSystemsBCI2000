###########################################################################
## $Id: MATH.cmake 3669 2011-11-23 16:54:53Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the EXTLIB Statistics libs
## SETS:
##       SRC_EXTLIB - Required source files for the statistics library
##       HDR_EXTLIB - Required header files for the statistics library
##       INC_EXTLIB - Include directory for the statistics library
##       Also defines source groups for the statistics files

# Define the source files
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/math/statistics/ObserverBase.cpp
  ${BCI2000_SRC_DIR}/extlib/math/statistics/PowerSumObserver.cpp 
  ${BCI2000_SRC_DIR}/extlib/math/statistics/HistogramObserver.cpp 
  ${BCI2000_SRC_DIR}/extlib/math/statistics/Histogram.cpp 
)

# Define the headers
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/math/statistics/StatisticalObserver.h 
  ${BCI2000_SRC_DIR}/extlib/math/statistics/ObserverBase.h 
  ${BCI2000_SRC_DIR}/extlib/math/statistics/PowerSumObserver.h 
  ${BCI2000_SRC_DIR}/extlib/math/statistics/HistogramObserver.h 
  ${BCI2000_SRC_DIR}/extlib/math/statistics/Histogram.h 
  ${BCI2000_SRC_DIR}/extlib/math/statistics/CombinedObserver.h 
)

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\math\\statistics FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\math\\statistics FILES ${HDR_EXTLIB} )


# Define include directories
SET( INC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/math/statistics
)

# Set success
SET( EXTLIB_OK TRUE )
