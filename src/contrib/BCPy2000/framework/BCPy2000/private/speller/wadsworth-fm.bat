@set PROG=..\..\..\..\..\..\..\BCI2000\prog
@if exist %PROG% goto SkipStandalone
:Standalone
@set PROG=.
@cd ..\prog
:SkipStandalone

call %PROG%\portable.bat

start           %PROG%\operat.exe  --OnConnect "-LOAD PARAMETERFILE ..\speller\wadsworth.prm"

::start           %PROG%\PythonSource.exe           --PythonSrcWD=..\speller --PythonSrcClassFile=BCI2000Tools/AudioSourceModule.py
start           %PROG%\gUSBAmpSource.exe

start           %PROG%\PythonSignalProcessing.exe --PythonSigWD=..\speller

start /REALTIME %PROG%\PythonApplication.exe      --PythonAppWD=..\speller
