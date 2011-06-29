###########################################################################
## $Id: IncludeExtension.cmake 3069 2011-01-19 23:27:37Z jhill $
## Authors: griffin.milsap@gmail.com

# Add the GazeMonitorFilter to all application modules

SET( BCI2000_APPHEADERS
   ${BCI2000_APPHEADERS}
   ${BCI2000_EXTENSION_DIR}/GazeMonitorFilter.h
   ${BCI2000_SRC_DIR}/shared/modules/application/audio/WavePlayer.h
   ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/ImageStimulus.h
   ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/TextStimulus.h
   ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/VisualStimulus.h
)

SET( BCI2000_APPSOURCES
   ${BCI2000_APPSOURCES}
   ${BCI2000_EXTENSION_DIR}/GazeMonitorFilter.cpp
   ${BCI2000_SRC_DIR}/shared/modules/application/audio/WavePlayer.cpp
   ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/ImageStimulus.cpp
   ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/TextStimulus.cpp
   ${BCI2000_SRC_DIR}/shared/modules/application/stimuli/VisualStimulus.cpp
)

BCI2000_USE( "DSOUND" )