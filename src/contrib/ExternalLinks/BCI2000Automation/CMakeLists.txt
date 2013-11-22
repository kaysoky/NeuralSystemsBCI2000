###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000Automation

BCI2000_INCLUDE( COM )

BCI2000_ADD_TARGET(
  WINDLL BCI2000Automation

  BCI2000Automation.cpp
  BCI2000Automation_i.c
  BCI2000Automation.rc
  CBCI2000Remote.cpp
  Resource.h
  ${BCI2000_SRC_DIR}/shared/utils/Lib/SockStream.cpp
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.cpp  
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp
  
  InstallAutomation.cmd
)
