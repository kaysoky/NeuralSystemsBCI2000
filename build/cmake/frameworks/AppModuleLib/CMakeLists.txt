###########################################################################
## $Id: AppModuleFramework.cmake 3483 2011-08-23 18:24:55Z mellinger $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Builds a static library containing components specific to
##   application modules.

ADD_SUBDIRECTORY( test )

SET( NAME BCI2000FrameworkAppModule )

SET( SRC_BCI2000_FRAMEWORK 
  ${SRC_BCI2000_FRAMEWORK}

  ${PROJECT_SRC_DIR}/shared/gui/GraphObject.cpp
  ${PROJECT_SRC_DIR}/shared/gui/DisplayWindow.cpp
  ${PROJECT_SRC_DIR}/shared/gui/GraphDisplay.cpp
  ${PROJECT_SRC_DIR}/shared/gui/TextField.cpp
  ${PROJECT_SRC_DIR}/shared/gui/GUI.cpp
  ${PROJECT_SRC_DIR}/shared/gui/Shapes.cpp

  ${PROJECT_SRC_DIR}/shared/modules/application/ApplicationBase.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/StimulusTask.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/FeedbackTask.cpp

  ${PROJECT_SRC_DIR}/shared/modules/application/audio/WavePlayer.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/audio/TextToSpeech.cpp

  ${PROJECT_SRC_DIR}/shared/modules/application/gui/ApplicationWindow.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/gui/TextWindow.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/gui/StatusBar.cpp

  ${PROJECT_SRC_DIR}/shared/modules/application/utils/TrialStatistics.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/utils/BlockRandSeq.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/utils/Localization.cpp

  ${PROJECT_SRC_DIR}/shared/modules/application/stimuli/Association.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/stimuli/AudioStimulus.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/stimuli/ImageStimulus.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/stimuli/TextStimulus.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/stimuli/VisualStimulus.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/stimuli/SpeechStimulus.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/stimuli/SoundStimulus.cpp

  ${PROJECT_SRC_DIR}/shared/modules/application/speller/SpellerCommand.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/speller/Speller.cpp
  ${PROJECT_SRC_DIR}/shared/modules/application/speller/AudioSpellerTarget.h
)

IF( WIN32 )
  SET( SRC_BCI2000_FRAMEWORK 
    ${SRC_BCI2000_FRAMEWORK}
    ${PROJECT_SRC_DIR}/shared/modules/application/audio/MidiPlayer.cpp
  )
ENDIF( WIN32 )

BCI2000_ADD_RESOURCE( EmptyWaveFile ${PROJECT_SRC_DIR}/shared/modules/application/audio/Empty.wav )

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
UTILS_INCLUDE( frameworks/AppModule )
BCI2000_ADD_REGISTRY( ${REGISTRY_NAME} SRC_BCI2000_FRAMEWORK DEPTARGETS )
BCI2000_ADD_TARGET( INFO "Framework library" STATIC_LIBRARY ${NAME} )

IF( FAILED )

  MESSAGE( WARNING "Could not satisfy dependencies of the Application library. Application modules will not build." )

ELSE( FAILED )

  TARGET_LINK_LIBRARIES( ${NAME} BCI2000FrameworkCore )
  ADD_DEPENDENCIES( ${NAME} ${DEPTARGETS} )

ENDIF( FAILED )
