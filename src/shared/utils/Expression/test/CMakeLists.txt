###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for Calculator

IF( BUILD_TESTS )

SET( SRC
  Calculator.cpp
)

SET( HDR
)

SET( DIR_NAME Tests/Expressions )
BCI2000_ADD_TOOLS_CMDLINE( 
  Calculator
  "${SRC}"
  "${HDR}"
  FALSE
)
SET_OUTPUT_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR} Calculator )

ENDIF( BUILD_TESTS )
