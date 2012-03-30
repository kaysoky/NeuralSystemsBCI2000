###########################################################################
## $Id: AppModuleFramework.cmake 3483 2011-08-23 18:24:55Z mellinger $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Builds a static library containing components specific to
##   application modules.

SET( NAME BCI2000FrameworkAppModule )

MESSAGE( "---Adding Framework Library: ${NAME}" )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreModuleLib.cmake )

SET( SRC_BCI2000_SHARED_GUI 
  ${BCI2000_SRC_DIR}/shared/gui/GraphObject.cpp
  ${BCI2000_SRC_DIR}/shared/gui/DisplayWindow.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GraphDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/gui/TextField.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GUI.cpp
  ${BCI2000_SRC_DIR}/shared/gui/Shapes.cpp
)
SET( HDR_BCI2000_SHARED_GUI
  ${BCI2000_SRC_DIR}/shared/gui/GraphObject.h
  ${BCI2000_SRC_DIR}/shared/gui/DisplayWindow.h
  ${BCI2000_SRC_DIR}/shared/gui/GraphDisplay.h
  ${BCI2000_SRC_DIR}/shared/gui/TextField.h
  ${BCI2000_SRC_DIR}/shared/gui/GUI.h
  ${BCI2000_SRC_DIR}/shared/gui/Shapes.h
)
SET( SRC_BCI2000_SHARED_MODULES_APPLICATION
  ${BCI2000_SRC_DIR}/shared/modules/application/ApplicationBase.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/StimulusTask.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/FeedbackTask.cpp
)
SET( HDR_BCI2000_SHARED_MODULES_APPLICATION
  ${BCI2000_SRC_DIR}/shared/modules/application/ApplicationBase.h
  ${BCI2000_SRC_DIR}/shared/modules/application/StimulusTask.h
  ${BCI2000_SRC_DIR}/shared/modules/application/FeedbackTask.h
)

SET( SRC_BCI2000_SHARED_MODULES_APPLICATION_AUDIO
  ${BCI2000_SRC_DIR}/shared/modules/application/audio/WavePlayer.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/audio/TextToSpeech.cpp
)
SET( HDR_BCI2000_SHARED_MODULES_APPLICATION_AUDIO
  ${BCI2000_SRC_DIR}/shared/modules/application/audio/WavePlayer.h
  ${BCI2000_SRC_DIR}/shared/modules/application/audio/TextToSpeech.h
)
IF( WIN32 )
  SET( SRC_BCI2000_SHARED_MODULES_APPLICATION_AUDIO
	${SRC_BCI2000_SHARED_MODULES_APPLICATION_AUDIO}
	${BCI2000_SRC_DIR}/shared/modules/application/audio/MidiPlayer.cpp
  )
  SET( HDR_BCI2000_SHARED_MODULES_APPLICATION_AUDIO
	${HDR_BCI2000_SHARED_MODULES_APPLICATION_AUDIO}
	${BCI2000_SRC_DIR}/shared/modules/application/audio/MidiPlayer.h
  )
ENDIF( WIN32 )

SET( SRC_BCI2000_SHARED_MODULES_APPLICATION_GUI
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/ApplicationWindow.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/TextWindow.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/StatusBar.cpp
)
SET( HDR_BCI2000_SHARED_MODULES_APPLICATION_GUI
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/ApplicationWindow.h
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/TextWindow.h
  ${BCI2000_SRC_DIR}/shared/modules/application/gui/StatusBar.h
)
SET( SRC_BCI2000_SHARED_MODULES_APPLICATION_UTILS
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/TrialStatistics.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/BlockRandSeq.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/Localization.cpp
)
SET( HDR_BCI2000_SHARED_MODULES_APPLICATION_UTILS
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/TrialStatistics.h
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/BlockRandSeq.h
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/Localization.h
)
SET( SRC_BCI2000_SHARED_MODULES_APPLICATION_STIMULI
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/Association.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/AudioStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/ImageStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/TextStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/VisualStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/SpeechStimulus.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/SoundStimulus.cpp
)
SET( HDR_BCI2000_SHARED_MODULES_APPLICATION_STIMULI
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/Association.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/AudioStimulus.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/ImageStimulus.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/TextStimulus.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/VisualStimulus.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/SpeechStimulus.h
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/SoundStimulus.h
)
SET( SRC_BCI2000_SHARED_MODULES_APPLICATION_SPELLER
  ${BCI2000_SRC_DIR}/shared/modules/application/speller/SpellerCommand.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/speller/Speller.cpp
)
SET( HDR_BCI2000_SHARED_MODULES_APPLICATION_SPELLER
  ${BCI2000_SRC_DIR}/shared/modules/application/speller/AudioSpellerTarget.h
  ${BCI2000_SRC_DIR}/shared/modules/application/speller/SpellerCommand.h
  ${BCI2000_SRC_DIR}/shared/modules/application/speller/Speller.h
)

# Set the BCI2000 Source Groups
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\gui FILES ${SRC_BCI2000_SHARED_GUI} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application FILES ${SRC_BCI2000_SHARED_MODULES_APPLICATION} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\audio FILES ${SRC_BCI2000_SHARED_MODULES_APPLICATION_AUDIO} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\gui FILES ${SRC_BCI2000_SHARED_MODULES_APPLICATION_GUI} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\stimuli FILES ${SRC_BCI2000_SHARED_MODULES_APPLICATION_STIMULI} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\speller FILES ${SRC_BCI2000_SHARED_MODULES_APPLICATION_SPELLER} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\utils FILES ${SRC_BCI2000_SHARED_MODULES_APPLICATION_UTILS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\gui FILES ${HDR_BCI2000_SHARED_GUI} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application FILES ${HDR_BCI2000_SHARED_MODULES_APPLICATION} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\audio FILES ${HDR_BCI2000_SHARED_MODULES_APPLICATION_AUDIO} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\gui FILES ${HDR_BCI2000_SHARED_MODULES_APPLICATION_GUI} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\stimuli FILES ${HDR_BCI2000_SHARED_MODULES_APPLICATION_STIMULI} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\speller FILES ${HDR_BCI2000_SHARED_MODULES_APPLICATION_SPELLER} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\utils FILES ${HDR_BCI2000_SHARED_MODULES_APPLICATION_UTILS} )

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${SRC_BCI2000_SHARED_GUI}
  ${SRC_BCI2000_SHARED_MODULES_APPLICATION}
  ${SRC_BCI2000_SHARED_MODULES_APPLICATION_AUDIO}
  ${SRC_BCI2000_SHARED_MODULES_APPLICATION_GUI}
  ${SRC_BCI2000_SHARED_MODULES_APPLICATION_UTILS}
  ${SRC_BCI2000_SHARED_MODULES_APPLICATION_STIMULI}
  ${SRC_BCI2000_SHARED_MODULES_APPLICATION_SPELLER}
)

SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${HDR_BCI2000_SHARED_GUI}
  ${HDR_BCI2000_SHARED_MODULES_APPLICATION}
  ${HDR_BCI2000_SHARED_MODULES_APPLICATION_AUDIO}
  ${HDR_BCI2000_SHARED_MODULES_APPLICATION_GUI}
  ${HDR_BCI2000_SHARED_MODULES_APPLICATION_UTILS}
  ${HDR_BCI2000_SHARED_MODULES_APPLICATION_STIMULI}
  ${HDR_BCI2000_SHARED_MODULES_APPLICATION_SPELLER}
)

# Setup the extra application components
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\human_interface_devices FILES ${BCI2000_HIDSOURCES} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application FILES ${BCI2000_APPSOURCES} )
SOURCE_GROUP( Source\\Extensions FILES ${BCI2000_APPSOURCES_EXT} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\human_interface_devices FILES ${BCI2000_HIDHEADERS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application FILES ${BCI2000_APPHEADERS} )
SOURCE_GROUP( Headers\\Extensions FILES ${BCI2000_APPHEADERS_EXT} )

# Add global application files into the framework
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${BCI2000_APPSOURCES}
  ${BCI2000_APPSOURCES_EXT} 
  ${BCI2000_HIDSOURCES}
)
SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${BCI2000_APPHEADERS}
  ${BCI2000_APPHEADERS_EXT}
  ${BCI2000_HIDHEADERS}
)

# Add Extlib dependencies
IF( WIN32 )
  BCI2000_INCLUDE( "SAPI" )
  BCI2000_INCLUDE( "DSOUND" )
ENDIF( WIN32 )
BCI2000_INCLUDE( "3DAPI" )

# Create the library project
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/AppModule.cmake )
BCI2000_ADD_REGISTRY( ${REGISTRY_NAME} SRC_BCI2000_FRAMEWORK DEPTARGETS )
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK DEPLIBS FAILED )
IF( FAILED )
  MESSAGE( WARNING "Could not satisfy dependencies of the Application library. Application modules will not build." )
ELSE( FAILED )
  ADD_LIBRARY( ${NAME} STATIC ${GENERATED} ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} )
  TARGET_LINK_LIBRARIES( ${NAME} ${DEPLIBS} BCI2000FrameworkCore )
  ADD_DEPENDENCIES( ${NAME} ${DEPTARGETS} )
  # Add Pre-processor defines
  IF( NOT BORLAND )
    SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=3 -DUSE_QT" )
  ELSE( NOT BORLAND )
    SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=3" )
  ENDIF( NOT BORLAND )
  # Link against the Qt/VCL Libraries
  IF( BORLAND )
    TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} cp32mt.lib )
  ELSE( BORLAND )
    TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} )
  ENDIF( BORLAND )

  # Set the project build folder
  SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )
ENDIF( FAILED )

