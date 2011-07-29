@set WD=%CD%
@set PYWD=%WD%\python
@set PARMS=%WD%\parms

@set PROG=..\..\..\..\..\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

@set PROG=%PYTHONHOME%\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog
:gotprog

@set BREAKOUT=%WD%\..\..\..\..\..\..\private\Application\Games\Breakout
@if exist %BREAKOUT% goto gotbreakout
:gotbreakout




@set PLAYBACK=..\data\20110706_8503_A_002\20110706_8503_A_S002R02.dat
@if [%1]==[] goto SKIPARG
@set PLAYBACK=%1
:SKIPARG

cd ..\prog
call portable.bat


@set OnConnect=-

:: Breakout parameters
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %BREAKOUT%\breakout\parms\Laptop.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %BREAKOUT%\breakout\parms\BCISYSTEM2_ExtendedDesktop.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %BREAKOUT%\breakout\parms\GameAppearance.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %BREAKOUT%\breakout\parms\GameDifficulty.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %BREAKOUT%\breakout\parms\RewardScheme.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %BREAKOUT%\breakout\parms\LevelCheckerboard6x6.prm
@set OnConnect=%OnConnect% ; SET PARAMETER BallSpeed   0.2
@set OnConnect=%OnConnect% ; SET PARAMETER AudioVolume 0.0
@set OnConnect=%OnConnect% ; SET PARAMETER PaddleExpressionX 2*Signal(1,1)
@set OnConnect=%OnConnect% ; SET PARAMETER IntegratePaddleX 0

:: Signal-processing parameters
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBampsAA-Cap16+Audio2.prm
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\fixed.prm 
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\audiostream_wadsworth_devel.prm
:: MUST ALSO LOAD WEIGHTS
@set OnConnect=%OnConnect% ; INSERT STATE Stream1
@set OnConnect=%OnConnect% ; INSERT STATE Stream2
@set OnConnect=%OnConnect% ; SET PARAMETER ContinuousOutput 1
@set OnConnect=%OnConnect% ; SET PARAMETER EpochAveragingPersistence 0.5
@set OnConnect=%OnConnect% ; SET PARAMETER ControlFilterCutoffHz 4.0

:: Playback parameters
@set OnConnect=%OnConnect% ; SET PARAMETER SubjectSession 999
@set OnConnect=%OnConnect% ; SET PARAMETER PlayBackStates 0
@set OnConnect=%OnConnect% ; SET PARAMETER EnslavePython 0

@set OnConnect=%OnConnect% ; SETCONFIG
@set OnSetConfig=- SET STATE Running 1

start              operat                   --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"

::start              PythonApplication        --PythonAppWD=%WD%\python --PythonAppClassFile=DummyApplication.py
start /D%BREAKOUT% GameBreakout

start              PythonSignalProcessing   --PythonSigWD=%WD%\python

ping -n 3 127.0.0.1 >nul
start              FilePlayback             --PlaybackFileName=%PLAYBACK%
