###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000PresentationLink

UTILS_INCLUDE( Frameworks/LibTiny )
BCI2000_INCLUDE( COM )
BCI2000_ADD_TARGET( WINDLL BCI2000PresentationLink

  BCI2000PresentationLink.cpp
  BCI2000DataPortExtension.cpp
  BCI2000DataPort.cpp
  BCI2000ActiveDataPort.cpp
  BCI2000PCLLibrary.cpp
  BCI2000ControllerType.cpp
  BCI2000Controller.cpp
  Utils.cpp
  BCI2000PresentationLink_i.c
  BCI2000PresentationLink.rc
  Resource.h

  ${BCI2000_SRC_DIR}/shared/config/Version.h
  ${BCI2000_SRC_DIR}/shared/utils/VersionInfo.cpp

  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.cpp  
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp  

  ${BCI2000_SRC_DIR}/extlib/nbs/DataPortExtension/interfaces/DataPortExtension_i.c
  ${BCI2000_SRC_DIR}/extlib/nbs/PCLExtension/interface/PCLExtension_i.c
  ${BCI2000_SRC_DIR}/extlib/nbs/DataPortExtension/interfaces/DataPortExtension.h
  ${BCI2000_SRC_DIR}/extlib/nbs/PCLExtension/interface/PCLExtension.h

  InstallPresentationLink.cmd
)
