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


@set SESSION=001
@if [%1]==[] goto SKIPSESSIONARG
@set SESSION=%1
:SKIPSESSIONARG

@set DEMO=
@if [%2]==[] goto SKIPDEMOARG
@set DEMO=%2
:SKIPDEMOARG


cd ..\prog
call portable.bat


@set OnConnect=-

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBampsAA-Cap16+Audio2.prm
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBampsBBAAA-Cap16+Audio2.prm
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBampsBBAAA-SchalkCap64+Audio2.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\drifting.prm 

@if [%SESSION%] == [001] goto SKIPFIXED
:FIXED
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\fixed.prm 
:SKIPFIXED

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\audiostream_wadsworth_devel.prm
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
start          gUSBampSource
::start           PythonSource             --PythonSrcWD=%WD%\python --PythonSrcShell=1 --PythonSrcLog=%WD%\log\###-src.txt

::start           DummySignalProcessing
start           PythonSignalProcessing   --PythonSigWD=%WD%\python

::start           DummyApplication
start PythonApplication        --PythonAppWD=%WD%\python
