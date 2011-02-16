###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for importing ShowParametersQt
## SETS:
##       SRC_GUI - Required source files for the ShowParametersQt
##       HDR_GUI - Required header files for the ShowParametersQt
##       INC_GUI - Include directory for the ShowParametersQt
##       MOC_GUI - Files which require MOCing
##       UI_GUI  - UI Files for wrapping
##       Also defines source groups for the ShowParametersQt files

# Define the source files
SET( SRC_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ShowParameters.cpp
)

# Define the headers
SET( HDR_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ShowParameters.h
)

# Wrap MOCable sources for this GUI
SET( MOC_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ShowParameters.h
)

# Set UI Files which need wrapping
SET( UI_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ShowParameters.ui
)

# Define the include directory
SET( INC_GUI 
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt
  ${BCI2000_SRC_DIR}/shared/gui/                      
)

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\core\\Operator\\OperatorQt FILES ${SRC_GUI} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\core\\Operator\\OperatorQt FILES ${HDR_GUI} )

# Set success
SET( GUI_OK TRUE )
