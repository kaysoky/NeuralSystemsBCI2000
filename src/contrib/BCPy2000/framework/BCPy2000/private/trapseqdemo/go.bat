@set WD=%CD%


@set PROG=%WD%\..\..\..\..\..\..\..\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

@set PROG=%PYTHONHOME%\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

:gotprog
@set PROG=%CD%



cd ..\prog
call portable.bat

start Operator               
start PythonSource           --PythonSrcWD=%WD%  --PythonSrcClassFile=BCI2000Tools/AudioSourceModule.py
start PythonSignalProcessing --PythonSigWD=%WD%  --PythonSigClassFile=TrapSequenceDemo.py
start PythonApplication      --PythonAppWD=%WD%  --PythonAppClassFile=TrapSequenceDemo.py
