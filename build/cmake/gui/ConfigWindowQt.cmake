###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for importing ConfigWindowQt
## SETS:
##       SRC_GUI - Required source files for the ConfigWindowQt
##       HDR_GUI - Required header files for the ConfigWindowQt
##       INC_GUI - Include directory for the ConfigWindowQt
##       MOC_GUI - Files which require MOCing
##       UI_GUI  - UI Files for wrapping
##       Also defines source groups for the ConfigWindowQt files

# Define the source files
SET( SRC_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ConfigWindow.cpp
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ParamDisplay.cpp
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ParsedComment.cpp
  ${BCI2000_SRC_DIR}/shared/gui/ExecutableHelp.cpp
)

# Define the headers
SET( HDR_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ConfigWindow.h
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ParamDisplay.h
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ParsedComment.h
  ${BCI2000_SRC_DIR}/shared/gui/ExecutableHelp.h
)

# Wrap MOCable sources for this GUI
SET( MOC_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ConfigWindow.h
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ParamDisplay.h
)

# Set UI Files which need wrapping
SET( UI_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/ConfigWindow.ui
)

# Define the include directory
SET( INC_GUI 
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt
  ${BCI2000_SRC_DIR}/shared/gui/                      
  ${BCI2000_SRC_DIR}/shared/fileio/dat                      
)

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\core\\Operator\\OperatorQt FILES ${SRC_GUI} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\core\\Operator\\OperatorQt FILES ${HDR_GUI} )

# Set success
SET( GUI_OK TRUE )
