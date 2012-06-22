###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000ViewerQt

# Set the executable name
SET( EXECUTABLE_NAME BCI2000Viewer )

# Set the project specific sources
SET( SRC_PROJECT
  main.cpp 
  BCI2000Viewer.cpp
  SignalWidget.cpp
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/DisplayFilter.cpp
)
SET( HDR_PROJECT
  BCI2000Viewer.h
  SignalWidget.h
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.h
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/DisplayFilter.h
)

SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\gui FILES
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.h
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\gui FILES
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.cpp
)

SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils\\DisplayFilter FILES
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/DisplayFilter.h
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils\\DisplayFilter FILES
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/DisplayFilter.cpp
)

INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering
)

# Tell the macro to link against the math libraries in extlib
BCI2000_INCLUDE( "MATH" )

# Wrap the Qt files
QT4_WRAP_UI( GENERATED_UI BCI2000Viewer.ui )
QT4_WRAP_CPP( GENERATED_MOC 
  BCI2000Viewer.h 
  SignalWidget.h
)
SET( GENERATED
  ${GENERATED_UI}
  ${GENERATED_MOC}
)

# Create the signal processing module
BCI2000_ADD_TOOLS_GUIAPP( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${GENERATED}"
)
