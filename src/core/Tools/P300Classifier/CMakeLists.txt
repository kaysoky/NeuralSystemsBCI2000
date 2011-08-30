###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for P300Classifier

# Set the executable name
SET( EXECUTABLE_NAME P300Classifier )

# Set the project specific sources
SET( SRC_PROJECT
  main.cpp 
  CARfilter.cpp
  CDataFile.cpp
  Check_Consistency_Files.cpp
  CmdLine.cpp
  configdialog.cpp
  downsampling.cpp
  filter.cpp
  get_cmd_parameters.cpp
  GetP3Responses.cpp
  GetScore.cpp
  in_out_variable.cpp
  load_BCIdat.cpp
  nrerror.cpp
  P300_Classify.cpp
  pages.cpp
  ReadIniParameters.cpp
  solve_linear_equation.cpp
  StimulusPresentation_Classify.cpp
  SWLDA.cpp
  WritePRM.cpp
)
SET( HDR_PROJECT
  CARfilter.h
  CDataFile.h
  Check_Consistency_Files.h
  CmdLine.h
  configdialog.h
  downsampling.h
  filter.h
  get_cmd_parameters.h
  GetP3Responses.h
  GetScore.h
  in_out_variable.h
  load_BCIdat.h
  nrerror.h
  P300_Classify.h
  pages.h
  ReadIniParameters.h
  resource.h
  solve_linear_equation.h
  StimulusPresentation_Classify.h
  SWLDA.h
  WritePRM.h
)

# Wrap the Qt files
QT4_WRAP_CPP( GENERATED_MOC 
  configdialog.h
  pages.h
)
SET( GENERATED
  ${GENERATED_MOC}
)
QT4_ADD_RESOURCES( GENERATED_RCC
  configdialog.qrc
)
SET( GENERATED
  ${GENERATED}
  ${GENERATED_RCC}
)

BCI2000_INCLUDE( ALGLIB )

# Create the signal processing module
BCI2000_ADD_TOOLS_GUIAPP( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${GENERATED}"
)


