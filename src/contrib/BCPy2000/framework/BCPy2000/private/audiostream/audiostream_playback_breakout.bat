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



@set PLAYBACK=%PROG%\..\data\EEG_201107_Audiostream\20110706_8503_A_002\20110706_8503_A_S002R02.dat
@set PLAYBACK=%PROG%\..\data\EEG_201107_Audiostream\20110801_8502_A_999\20110801_8502_A_S999R10.dat
@set PLAYBACK=%PROG%\..\data\EEG_201107_Audiostream\20110802_8501_A_999\20110802_8501_A_S999R08.dat
@if [%1]==[] goto SKIPARG
@set PLAYBACK=%1
:SKIPARG

cd ..\prog
call portable.bat

@set OnConnect=-

@set OnConnect=%OnConnect% ; INSERT STATE Stream1 3 0 0 0
@set OnConnect=%OnConnect% ; INSERT STATE Stream2 3 0 0 0

@set OnConnect=%OnConnect% ; SET PARAMETER SubjectSession 555
::@set OnConnect=%OnConnect% ; SET PARAMETER PlayBackStates 0
::@set OnConnect=%OnConnect% ; SET PARAMETER EnslavePython 0
::@set OnConnect=%OnConnect% ; SET PARAMETER PlaybackReverseData 1
::@set OnConnect=%OnConnect% ; SET PARAMETER PlaybackSpeed 10

start              operat                   --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"

start /D%BREAKOUT% GameBreakout

start              PythonSignalProcessing   --PythonSigWD=%WD%\python

ping -n 3 127.0.0.1 >nul
start              FilePlayback             --PlaybackFileName=%PLAYBACK%

