###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for OperatorLibDemo

IF( BUILD_DEMOS )

# Set the executable name
SET( EXECUTABLE_NAME OperatorLibDemo )

# Set the project specific sources
SET( SRC_PROJECT
  main.cpp 
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream_tool.cpp
  ${BCI2000_SRC_DIR}/core/Operator/OperatorLib/BCI_OperatorLib.h
)

UTILS_INCLUDE( frameworks/Core )
BCI2000_ADD_TARGET( EXECUTABLE ${EXECUTABLE_NAME} ${SRC_PROJECT} )
IF( NOT FAILED )
  BCI2000_ADD_TO_INVENTORY( "Operator" ${EXECUTABLE_NAME} )  
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} OperatorLib )
  SET_PROPERTY( TARGET ${EXECUTABLE_NAME} PROPERTY FOLDER Demos/OperatorLib )
ENDIF()

ENDIF( BUILD_DEMOS )
