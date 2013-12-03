###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000LauncherQt

# Set the executable name
SET( EXECUTABLE_NAME BCI2000Launcher )

SET( SRC_PROJECT
  main.cpp 
  MainWindow.cpp
  MainWindow.ui
)
BCI2000_USE_INVENTORY( SRC_PROJECT )
BCI2000_ADD_TOOLS_GUIAPP(
  "${EXECUTABLE_NAME}"
  "${SRC_PROJECT}"
  OUTPUT_DIRECTORY "${BCI2000_ROOT_DIR}/prog"
)
