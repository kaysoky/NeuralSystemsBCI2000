###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000FileInfoQt

# Set the executable name
SET( EXECUTABLE_NAME BCI2000FileInfo )

# Set the project specific sources
SET( SRC_PROJECT
  main.cpp 
  bci2000fileinfo.cpp
  bci2000fileinfo.ui
)

# Tell the macro to link against a few external GUIs
BCI2000_IMPORT_GUI( "ConfigWindowQt" )
BCI2000_IMPORT_GUI( "EditMatrixQt" )
BCI2000_IMPORT_GUI( "ShowParametersQt" )
BCI2000_IMPORT_GUI( "PrefDialogQt" )
BCI2000_IMPORT_GUI( "PreferencesQt" )
BCI2000_IMPORT_GUI( "OperatorUtilsQt" )

# Create the signal processing module
BCI2000_ADD_TOOLS_GUIAPP( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}"
)
