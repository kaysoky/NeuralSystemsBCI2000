###########################################################################
## $Id$
## Authors: jezhill@gmail.com
## Description: Build information BCI2000 Framework extensions

OPTION( BUILD_DATAGLOVELOGGER "Include DataGloveLogger in SignalSource framework" OFF )
OPTION( BUILD_WEBCAMLOGGER "Include WebcamLogger in SignalSource framework" OFF )
OPTION( BUILD_EYETRACKERLOGGER "Include EyetrackerLogger in SignalSource framework" OFF )
OPTION( BUILD_WIIMOTELOGGER "Include WiimoteLogger in SignalSource framework" OFF )
OPTION( BUILD_GAZEMONITORFILTER "Include GazeMonitorFilter in Application framework" OFF )
OPTION( BUILD_AUDIOEXTENSION "Include AudioExtension in SignalSource framework" OFF )

INCLUDE_EXTENSION( DataGloveLogger   "${BCI2000_SRC_DIR}/contrib/Extensions/DataGloveLogger" )
INCLUDE_EXTENSION( WebcamLogger      "${BCI2000_SRC_DIR}/contrib/Extensions/WebcamLogger" )
INCLUDE_EXTENSION( EyetrackerLogger  "${BCI2000_SRC_DIR}/contrib/Extensions/EyetrackerLogger" )
INCLUDE_EXTENSION( WiimoteLogger     "${BCI2000_SRC_DIR}/contrib/Extensions/WiimoteLogger" )
INCLUDE_EXTENSION( GazeMonitorFilter "${BCI2000_SRC_DIR}/contrib/Extensions/GazeMonitorFilter" )
INCLUDE_EXTENSION( AudioExtension    "${BCI2000_SRC_DIR}/contrib/Extensions/AudioExtension" ) 
