###########################################################################
## $Id: CMakeLists.txt 4373 2013-02-25 13:21:16Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}SignalSource/" )

# Recurse down into all project subdirectories
IF( WIN32 )
  # ADD_SUBDIRECTORY( actiCHamp ) # Doesn't compile
  ADD_SUBDIRECTORY( AmpServerPro )
  ADD_SUBDIRECTORY( BioRadio )
  ADD_SUBDIRECTORY( Biosemi )
  ADD_SUBDIRECTORY( B-Alert )
  ADD_SUBDIRECTORY( DASSource )
  ADD_SUBDIRECTORY( Emotiv )
  ADD_SUBDIRECTORY( Enobio )
  ADD_SUBDIRECTORY( gHIampSource )
  ADD_SUBDIRECTORY( Micromed )
  ADD_SUBDIRECTORY( ModularEEG )
  ADD_SUBDIRECTORY( Neuralynx )
  ADD_SUBDIRECTORY( NeuroscanAccessSDK )
  ADD_SUBDIRECTORY( NeuroSky )
  ADD_SUBDIRECTORY( NIDAQ-MX )
  ADD_SUBDIRECTORY( TMSi )
  ADD_SUBDIRECTORY( vAmp )
  IF( BUILD_MFC )
    ADD_SUBDIRECTORY( NicoletOne )
  ENDIF( BUILD_MFC )
ENDIF( WIN32 )

ADD_SUBDIRECTORY( RDAClient)
ADD_SUBDIRECTORY( Neuroscan )
ADD_SUBDIRECTORY( FieldTripBufferSource )
ADD_SUBDIRECTORY( FilePlayback )
ADD_SUBDIRECTORY( Blackrock )
# ADD_SUBDIRECTORY( Ripple ) # Doesn't link
