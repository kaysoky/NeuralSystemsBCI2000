@set optpfx=EXTENSIONS_

@set OPT=%optpfx%WEBCAMLOGGER
@set /p "ANS=  Include WebcamLogger in SignalSource framework (y/n)? "
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=%optpfx%DATAGLOVELOGGER
@set /p "ANS=  Include DataGloveLogger in SignalSource framework (y/n)? "
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=%optpfx%EYETRACKERLOGGER
@set /p "ANS=  Include EyetrackerLogger in SignalSource framework (y/n)? "
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=%optpfx%WIIMOTELOGGER
@set /p "ANS=  Include WiimoteLogger in SignalSource framework (y/n)? "
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=%optpfx%AUDIOEXTENSION
@set /p "ANS=  Include AudioExtension in SignalSource framework (y/n)? "
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=%optpfx%GAZEMONITORFILTER
@set /p "ANS=  Include GazeMonitorFilter in Application framework (y/n)? "
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )
