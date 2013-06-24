###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for vAmpSource

BCI2000_INCLUDE( vAmp )
BCI2000_INCLUDE( Math )
BCI2000_USE( Qt )

BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  vAmpSource
  
  vAmpADC.cpp 
  vAmpChannelInfo.cpp
  vAmpThread.cpp
  vAmpDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GUI.h

  "${BCI2000_SRC_DIR}/extlib/brainproducts/vamp/FirstAmp.dll" 
)
