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
)
SET( HDR_PROJECT
  bci2000fileinfo.h
)

# Wrap the Qt files
QT4_WRAP_UI( GENERATED_UI bci2000fileinfo.ui )
QT4_WRAP_CPP( GENERATED_MOC bci2000fileinfo.h )
SET( GENERATED
  ${GENERATED_UI}
  ${GENERATED_MOC}
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
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${GENERATED}"
)


