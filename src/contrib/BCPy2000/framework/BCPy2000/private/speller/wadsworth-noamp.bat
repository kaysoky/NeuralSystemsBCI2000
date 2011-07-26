@call GotoFullMontyBCI2000

cd ..\prog
call portable.bat

start           operat.exe  --OnConnect "-LOAD PARAMETERFILE %SPELLERDIR%\wadsworth.prm"

start           PythonSource.exe           --PythonSrcWD=%SPELLERDIR% --PythonSrcClassFile=BCI2000Tools/AudioSourceModule.py
::start           gUSBAmpSource.exe

start           PythonSignalProcessing.exe --PythonSigWD=%SPELLERDIR%

start /REALTIME /D%SPELLERDIR% PythonApplication.exe      --PythonAppWD=%SPELLERDIR%
