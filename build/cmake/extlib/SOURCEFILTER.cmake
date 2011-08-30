###########################################################################
## $Id: GUSBAMP.cmake 3492 2011-08-24 16:10:28Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the SourceFilter
## SETS:
##       SRC_EXTLIB - Required source files
##       HDR_EXTLIB - Required header files
##       INC_EXTLIB - Include directories
##       LIBDIR_EXTLIB - Library directories
##       LIBS_EXTLIB - required libraries
##       Also defines source groups for source files


SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/SourceFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRFilterBase.cpp
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.cpp
)

SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/SourceFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/IIRFilterBase.h
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.h
  ${BCI2000_SRC_DIR}/extlib/math/IIRFilter.h
)

SET( INC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/math
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing
)

SET( LIBDIR_EXTLIB
)

SET( LIBS_EXTLIB
)

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\SourceFilter FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\SourceFilter FILES ${HDR_EXTLIB} )

# Set success
SET( EXTLIB_OK TRUE )
