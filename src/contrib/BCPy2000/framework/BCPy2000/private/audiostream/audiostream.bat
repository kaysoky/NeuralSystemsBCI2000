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

@set SESSION=002
@if [%1]==[] goto SKIPSESSIONARG
@set SESSION=%1
:SKIPSESSIONARG

@set DEMO=
@if [%2]==[] goto SKIPDEMOARG
@set DEMO=%2
:SKIPDEMOARG



cd ..\prog
::call portable.bat
@set LOGGERS=
@set OnConnect=-

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBampsBB-Cap16+Audio2.prm
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBampsBBAAA-Cap16+Audio2.prm
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBampsBBAAA-SchalkCap64+Audio2.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\drifting.prm 
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\audiostream_wadsworth_devel.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\eyetracker.prm      && set LOGGERS=%LOGGERS% --LogEyetracker=1
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\arse.prm

@if [%SESSION%] == [001] goto SKIPFIXED
@if [%SESSION%] == [002] goto FIXED
:FIXEDFAST
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\fixedfast.prm 
@goto SKIPFIXED
:FIXED
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\fixed.prm 
@goto SKIPFIXED
:SKIPFIXED

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\subject_attention.prm 
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\subject_perception.prm 
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PYWD%\ChannelVolumesDB.prm

if [%DEMO%] == [] goto SKIPDEMO
:DEMO
@set OnConnect=%OnConnect% ; SET PARAMETER SubjectName 2011XXXX_85XX_A_
@set OnConnect=%OnConnect% ; SET PARAMETER InteractiveVolumeAdjust 1
:SKIPDEMO

::@set OnConnect=%OnConnect% ; SETCONFIG
::@set OnSetConfig=- SET STATE Running 1

start           operat                   --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"
start           gUSBampSource %LOGGERS%
::start           PythonSource             --PythonSrcWD=%WD%\python --PythonSrcShell=1 --PythonSrcLog=%WD%\log\###-src.txt

::start           DummySignalProcessing
start           PythonSignalProcessing   --PythonSigWD=%WD%\python

::start           DummyApplication
::start           StimulusPresentation
start           PythonApplication        --PythonAppWD=%WD%\python
