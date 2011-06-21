@set WD=%CD%
@set PROG=..\..\..\..\..\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%

cd ..\prog
call portable.bat

start operat.exe                      --OnConnect "-LOAD PARAMETERFILE %WD%\parms\audiostream_soundcard_250Hz.prm ; LOAD PARAMETERFILE %WD%\parms\audiostream_EEG67+EOG3+SYNC2+VMRK_Quickamp_500Hz.prm ; SETCONFIG"
start PythonSource.exe                --PythonSrcWD=%WD%\python
start PythonSignalProcessing.exe      --PythonSigWD=%WD%\python
start PythonApplication.exe           --PythonAppWD=%WD%\python
