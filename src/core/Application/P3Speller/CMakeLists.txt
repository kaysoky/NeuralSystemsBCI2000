###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for P3Speller in Qt

# Set the executable name
SET( EXECUTABLE_NAME P3Speller )

# Set the project specific sources
SET( SRC_PROJECT
  P3Speller.cpp
  P3SpellerTask.cpp
)
SET( HDR_PROJECT
  P3SpellerTask.h
)

# Set required application source files
SET( SRC_APPLICATION_STIMULI
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/Association.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/AudioStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/ImageStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/TextStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/VisualStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/SpeechStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/SoundStimulus.cpp
)
SET( HDR_APPLICATION_STIMULI
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/Association.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/AudioStimulus.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/ImageStimulus.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/TextStimulus.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/VisualStimulus.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/SpeechStimulus.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/SoundStimulus.h
)
SET( SRC_APPLICATION_SPELLER
  ${BCI2000_SRC_DIR}/shared/modules/application/speller/SpellerCommand.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/speller/Speller.cpp
)
SET( HDR_APPLICATION_SPELLER
  ${BCI2000_SRC_DIR}/shared/modules/application/speller/SpellerCommand.h
  ${BCI2000_SRC_DIR}/shared/modules/application/speller/Speller.h
)
SET( SRC_APPLICATION_GUI
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/TextWindow.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/StatusBar.cpp
)
SET( HDR_APPLICATION_GUI
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/TextWindow.h
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/StatusBar.h
)
SET( SRC_APPLICATION_AUDIO
  ${BCI2000_SRC_DIR}/shared/modules/application/audio/WavePlayer.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/audio/TextToSpeech.cpp
)
SET( HDR_APPLICATION_AUDIO
  ${BCI2000_SRC_DIR}/shared/modules/application/audio/WavePlayer.h
  ${BCI2000_SRC_DIR}/shared/modules/application/audio/TextToSpeech.h
)
SET( SRC_APPLICATION
  ${BCI2000_SRC_DIR}/shared/modules/application/StimulusTask.cpp
)
SET( HDR_APPLICATION
  ${BCI2000_SRC_DIR}/shared/modules/application/StimulusTask.h
)

SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\stimuli FILES ${SRC_APPLICATION_STIMULI} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\speller FILES ${SRC_APPLICATION_SPELLER} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\gui FILES ${SRC_APPLICATION_GUI} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\audio FILES ${SRC_APPLICATION_AUDIO} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application FILES ${SRC_APPLICATION} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\stimuli FILES ${HDR_APPLICATION_STIMULI} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\speller FILES ${HDR_APPLICATION_SPELLER} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\gui FILES ${HDR_APPLICATION_GUI} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\audio FILES ${HDR_APPLICATION_AUDIO} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application FILES ${HDR_APPLICATION} )

SET( SRC_APPLICATION
  ${SRC_APPLICATION}
  ${SRC_APPLICATION_STIMULI}
  ${SRC_APPLICATION_SPELLER}
  ${SRC_APPLICATION_GUI}
  ${SRC_APPLICATION_AUDIO}
)
SET( HDR_APPLICATION
  ${HDR_APPLICATION}
  ${HDR_APPLICATION_STIMULI}
  ${HDR_APPLICATION_SPELLER}
  ${HDR_APPLICATION_GUI}
  ${HDR_APPLICATION_AUDIO}
)


# Define extra include directories
SET( INCLUDES
  ${BCI2000_SRC_DIR}/shared/modules/application/
  ${BCI2000_SRC_DIR}/shared/modules/application/audio
  ${BCI2000_SRC_DIR}/shared/modules/application/gui
  ${BCI2000_SRC_DIR}/shared/modules/application/speller
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli
  ${BCI2000_SRC_DIR}/shared/modules/application/utils
)

# Notify macro we need some extlib dependencies
BCI2000_USE( "SAPI" )
BCI2000_USE( "DSOUND" )

# Create the signal processing module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${SRC_APPLICATION}" "${HDR_APPLICATION}" 
  "${INCLUDES}"
)


