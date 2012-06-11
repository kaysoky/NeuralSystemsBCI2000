@set OPT=BUILD_WEBCAMLOGGER
@set /p ANS=    Include WebcamLogger in SignalSource framework (y/n)?
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=BUILD_DATAGLOVELOGGER
@set /p ANS=    Include DataGloveLogger in SignalSource framework (y/n)?
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=BUILD_EYETRACKERLOGGER
@set /p ANS=    Include EyetrackerLogger in SignalSource framework (y/n)?
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=BUILD_WIIMOTELOGGER
@set /p ANS=    Include WiimoteLogger in SignalSource framework (y/n)?
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=BUILD_GAZEMONITORFILTER
@set /p ANS=	Include GazeMonitorFilter in Application framework (y/n)?
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=BUILD_AUDIOEXTENSION
@set /p ANS=	Include AudioExtension in SignalSource framework (y/n)?
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )
