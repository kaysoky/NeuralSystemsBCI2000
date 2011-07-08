@set WD=%CD%
@set PYWD=%WD%\python
@set PARMS=%WD%\parms

@set PROG=..\..\..\..\..\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto start

@set PROG=%PYTHONHOME%\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto start
:start


@set PLAYBACK=..\data\20110706_8503_A_002\20110706_8503_A_S002R02.dat
@if [%1]==[] goto SKIPARG
@set PLAYBACK=%1
:SKIPARG

cd ..\prog
call portable.bat


@set OnConnect=-

@set OnConnect=%OnConnect% ; SET PARAMETER SubjectSession 003
@set OnConnect=%OnConnect% ; SET PARAMETER PlayBackStates 1
@set OnConnect=%OnConnect% ; SET PARAMETER EnslavePython 1

::@set OnConnect=%OnConnect% ; SETCONFIG
::@set OnSetConfig=- SET STATE Running 1

start           operat                   --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"
start           PythonSignalProcessing   --PythonSigWD=%WD%\python
start           PythonApplication        --PythonAppWD=%WD%\python
ping -n 3 127.0.0.1 >nul
start           FilePlayback             --PlaybackFileName=%PLAYBACK%
