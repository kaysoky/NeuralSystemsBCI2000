@set WD=%CD%
@set PYWD=%WD%\python
@set PARMS=%WD%\parms

@set PROG=%WD%\..\..\..\..\..\..\..\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

@set PROG=%PYTHONHOME%\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog
:gotprog
@set PROG=%CD%

@set BREAKOUT=%WD%\..\..\..\..\..\..\private\Application\Games\Breakout
@if exist %BREAKOUT% goto gotbreakout
:gotbreakout



cd ..\prog
call portable.bat


@set LOGGERS=
@set OnConnect=-

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBampsBB-Cap16+Audio2.prm
:: Breakout parameters
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %BREAKOUT%\breakout\parms\Laptop.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %BREAKOUT%\breakout\parms\BCISYSTEM2_ExtendedDesktop.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\AudiostreamBreakoutGame.prm

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\eyetracker.prm      && set LOGGERS=%LOGGERS% --LogEyetracker=1

:: NB AudiostreamBreakoutGame sets some control-signal-filtering parameters.  Overwrite these here
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\tng.prm 
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\triggerless16.prm 
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\condition004.prm 

:: MUST ALSO LOAD WEIGHTS
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PROG%\..\data\EEG_201107_Audiostream\remember_to_load_some_weights.prm

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\subject_attention.prm
@set OnConnect=%OnConnect% ; SET PARAMETER SubjectSession 999

::@set OnConnect=%OnConnect% ; INSERT STATE Stream1 3 0 0 0
::@set OnConnect=%OnConnect% ; INSERT STATE Stream2 3 0 0 0
::@set OnConnect=%OnConnect% ; SETCONFIG
::@set OnSetConfig=- SET STATE Running 1

start              Operator                 --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"

start /D%BREAKOUT% GameBreakout

start              PythonSignalProcessing   --PythonSigWD=%WD%\python --PythonSigClassFile=Streaming.py

start              gUSBampSource %LOGGERS%
