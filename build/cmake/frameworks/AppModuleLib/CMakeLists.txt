###########################################################################
## $Id: AppModuleFramework.cmake 3483 2011-08-23 18:24:55Z mellinger $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Builds a static library containing components specific to
##   application modules.

ADD_SUBDIRECTORY( test )

SET( NAME BCI2000FrameworkAppModule )

SET( SRC_BCI2000_FRAMEWORK 
  ${SRC_BCI2000_FRAMEWORK}

  ${BCI2000_SRC_DIR}/shared/gui/GraphObject.cpp
  ${BCI2000_SRC_DIR}/shared/gui/DisplayWindow.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GraphDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/gui/TextField.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GUI.cpp
  ${BCI2000_SRC_DIR}/shared/gui/Shapes.cpp

  ${BCI2000_SRC_DIR}/shared/modules/application/ApplicationBase.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/StimulusTask.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/FeedbackTask.cpp

  ${BCI2000_SRC_DIR}/shared/modules/application/audio/WavePlayer.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/audio/TextToSpeech.cpp

  ${BCI2000_SRC_DIR}/shared/modules/application/gui/ApplicationWindow.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/TextWindow.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/StatusBar.cpp

  ${BCI2000_SRC_DIR}/shared/modules/application/utils/TrialStatistics.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/BlockRandSeq.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/Localization.cpp

  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/Association.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/AudioStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/ImageStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/TextStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/VisualStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/SpeechStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/SoundStimulus.cpp

  ${BCI2000_SRC_DIR}/shared/modules/application/speller/SpellerCommand.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/speller/Speller.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/speller/AudioSpellerTarget.h
)

IF( WIN32 )
  SET( SRC_BCI2000_FRAMEWORK 
    ${SRC_BCI2000_FRAMEWORK}
    ${BCI2000_SRC_DIR}/shared/modules/application/audio/MidiPlayer.cpp
  )
ENDIF( WIN32 )

BCI2000_ADD_RESOURCE( EmptyWaveFile ${BCI2000_SRC_DIR}/shared/modules/application/audio/Empty.wav )

# Add global application files
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${BCI2000_APPSOURCES}
  ${BCI2000_APPSOURCES_EXT} 
  ${BCI2000_HIDSOURCES}
  ${HDR_BCI2000_FRAMEWORK}
  ${BCI2000_APPHEADERS}
  ${BCI2000_APPHEADERS_EXT}
  ${BCI2000_HIDHEADERS}
)

# Add Extlib dependencies
IF( WIN32 )
  BCI2000_INCLUDE( "SAPI" )
ENDIF( WIN32 )
BCI2000_INCLUDE( "PORTAUDIO" )
BCI2000_INCLUDE( "3DAPI" )

# Create the library project
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/AppModule.cmake )
BCI2000_ADD_REGISTRY( ${REGISTRY_NAME} SRC_BCI2000_FRAMEWORK DEPTARGETS )
BCI2000_ADD_TARGET( INFO "Framework library" STATIC_LIBRARY ${NAME} )

IF( FAILED )

  MESSAGE( WARNING "Could not satisfy dependencies of the Application library. Application modules will not build." )

ELSE( FAILED )

  TARGET_LINK_LIBRARIES( ${NAME} BCI2000FrameworkCore )
  ADD_DEPENDENCIES( ${NAME} ${DEPTARGETS} )

ENDIF( FAILED )
