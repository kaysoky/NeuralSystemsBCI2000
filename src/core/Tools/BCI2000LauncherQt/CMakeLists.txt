###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000LauncherQt

# Set the executable name
SET( EXECUTABLE_NAME BCI2000Launcher )

# Set the project specific sources
SET( SRC_PROJECT
  main.cpp 
  MainWindow.cpp
)
SET( HDR_PROJECT
  MainWindow.h
)

# Wrap the Qt files
QT4_WRAP_UI( GENERATED_UI MainWindow.ui )
QT4_WRAP_CPP( GENERATED_MOC MainWindow.h )
SET( GENERATED
  ${GENERATED_UI}
  ${GENERATED_MOC}
)

BCI2000_USE_INVENTORY( SRC_PROJECT HDR_PROJECT )

# Create the signal processing module
BCI2000_ADD_TOOLS_GUIAPP( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${GENERATED}"
)
SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" ${EXECUTABLE_NAME} )
