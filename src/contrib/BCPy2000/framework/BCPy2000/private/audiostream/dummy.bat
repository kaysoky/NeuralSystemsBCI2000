@set WD=%CD%
@set PROG=..\..\..\..\..\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%

cd ..\prog
call portable.bat


@set OnConnect=-

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\parms\audiostream_EEG67+EOG3+SYNC2+VMRK_Quickamp_500Hz.prm 
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\parms\gUSBamps.prm 
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\parms\explore_pulses.prm 
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\parms\headphones001.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\subject_attention.prm 
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\subject_perception.prm 

@set OnConnect=%OnConnect% ; SETCONFIG

start           operat                   --OnConnect "%OnConnect%"
start           gUSBampSource
::start           PythonSource             --PythonSrcWD=%WD%\python --PythonSrcShell=1 --PythonSrcLog=%WD%\log\###-src.txt

::start           DummySignalProcessing
start           PythonSignalProcessing   --PythonSigWD=%WD%\python --PythonSigShell=1 --PythonSigLog=%WD%\log\###-sig.txt --PythonSigClassFile=

::start           DummyApplication
start /REALTIME PythonApplication        --PythonAppWD=%WD%\python --PythonAppShell=1 --PythonAppLog=%WD%\log\###-app.txt --PythonAppClassFile=
