###########################################################################
## $Id: CMakeLists.txt 3497 2011-08-30 16:50:58Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for ObserverTest

BCI2000_INCLUDE( "MATH" )
BCI2000_INCLUDE( "STATISTICS" )

SET( SRC
  ObserverTest.cpp
  ../WindowObserver.cpp
)

SET( HDR
  ../WindowObserver.h
)

BCI2000_ADD_TOOLS_CMDLINE( 
  ObserverTest
  "${SRC}"
  "${HDR}"
  FALSE
)
SET_OUTPUT_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR} ObserverTest )
