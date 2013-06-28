###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Modifies the framework and sets up GUIs to be imported

SET( dir_ ${PROJECT_SRC_DIR}/core/Operator/OperatorQt )

SET( BCI2000_PARAMETER_GUI

  ${dir_}/PrefDialog.ui
  ${dir_}/ConfigWindow.ui
  ${dir_}/EditMatrix.ui
  ${dir_}/ShowParameters.ui

  ${dir_}/PrefDialog.cpp
  ${dir_}/ConfigWindow.cpp
  ${dir_}/EditMatrix.cpp
  ${dir_}/ShowParameters.cpp
  ${dir_}/Preferences.cpp
  ${dir_}/OperatorUtils.cpp
  ${dir_}/ParsedComment.cpp
  ${dir_}/ParamDisplay.cpp

  ${PROJECT_SRC_DIR}/shared/gui/ExecutableHelp.cpp
)
