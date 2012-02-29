@set WD=%CD%

:: Hunt for a suitable prog directory
@set PROG=%WD%\..\audiostream\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

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

@SET OnConnect="-"
::@SET OnConnect=%OnConnect% ; SETCONFIG


@SET OnSetConfig="-"
@SET OnSetConfig=%OnSetConfig% ; SET STATE Running 1

start Operator               --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"
start PythonSource           --PythonSrcWD=%WD%  --PythonSrcClassFile=BCI2000Tools/AudioSourceModule.py
start PythonSignalProcessing --PythonSigWD=%WD%  --PythonSigClassFile=TrapSequenceDemo.py
start PythonApplication      --PythonAppWD=%WD%  --PythonAppClassFile=TrapSequenceDemo.py