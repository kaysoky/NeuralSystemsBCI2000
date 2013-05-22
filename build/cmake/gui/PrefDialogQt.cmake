###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for importing PrefDialogQt
## SETS:
##       SRC_GUI - Required source files for the PrefDialogQt
##       HDR_GUI - Required header files for the PrefDialogQt
##       INC_GUI - Include directory for the PrefDialogQt
##       MOC_GUI - Files which require MOCing
##       UI_GUI  - UI Files for wrapping

# Define the source files
SET( SRC_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/PrefDialog.cpp
)

# Define the headers
SET( HDR_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/PrefDialog.h
)

# Wrap MOCable sources for this GUI
SET( MOC_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/PrefDialog.h
)

# Set UI Files which need wrapping
SET( UI_GUI
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt/PrefDialog.ui
)

# Define the include directory
SET( INC_GUI 
  ${BCI2000_SRC_DIR}/core/Operator/OperatorQt
)

# Set success
SET( GUI_OK TRUE )
