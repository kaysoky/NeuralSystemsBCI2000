###########################################################################
## $Id$
## Authors:
## Description: Build information for the SoundcardSource source module
##              
##              
###########################################################################

UTILS_INCLUDE( extlib/PORTAUDIO )

BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  SoundcardSource
  PortAudioADC.cpp
  PortAudioStream.cpp
)
