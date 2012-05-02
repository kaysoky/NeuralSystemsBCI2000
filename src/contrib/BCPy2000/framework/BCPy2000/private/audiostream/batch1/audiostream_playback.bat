@set WD=%CD%
@set PYWD=%WD%\..\python
@set PARMS=%WD%\..\parms

@set PROG=%WD%\..\..\..\..\..\..\..\..\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

@set PROG=%PYTHONHOME%\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog
:gotprog
@set PROG=%CD%


@set PLAYBACK=%PROG%\..\data\EEG_201107_Audiostream\20110706_8503_A_002\20110706_8503_A_S002R02.dat
@if [%1]==[] goto SKIPARG
@set PLAYBACK=%1
:SKIPARG

cd ..\prog
call portable.bat


@set OnConnect=-

@set OnConnect=%OnConnect% ; SET PARAMETER SubjectSession 999
@set OnConnect=%OnConnect% ; SET PARAMETER PlayBackStates 1
@set OnConnect=%OnConnect% ; SET PARAMETER EnslavePython 1

::@set OnConnect=%OnConnect% ; SETCONFIG
::@set OnSetConfig=- SET STATE Running 1

start           Operator                 --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"
start           PythonSignalProcessing   --PythonSigWD=%WD%\..\python
start           PythonApplication        --PythonAppWD=%WD%\..\python
ping -n 3 127.0.0.1 >nul
start           FilePlayback             --PlaybackFileName=%PLAYBACK%
