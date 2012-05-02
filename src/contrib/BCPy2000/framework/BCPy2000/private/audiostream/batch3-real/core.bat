@set WD=%CD%
@set PYWD=%WD%\..\python
@set PARMS=%WD%\..\parms
@set PYPROG=%WD%\..\prog

:: parasitizing the VA setup:  v3.x binaries for operator/SIG/APP will be packed in the local %PYPROG% subdir, but SRC will be in ../prog
@set PROG=%WD%\..\..\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

:: last fallback = E1001 setup: one 3.x prog directory only
@set PROG=%WD%\..\..\..\..\..\..\..\..\prog
@set PYPROG=%PROG%
call portable.bat
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

:gotprog

@set SUBJECT=TestSubject
@if [%1]==[] goto SKIPSUBJECTARG
@set SUBJECT=%1
:SKIPSUBJECTARG

@set CONDITION=002
@if [%2]==[] goto SKIPCONDITIONARG
@set CONDITION=%2
:SKIPCONDITIONARG

@set MODE=CALIB
@if [%3]==[] goto SKIPMODEARG
@set MODE=%3
:SKIPMODEARG

@set SRC=gUSBampSource
@set LOGGERS=
@set OnConnect=-
@set OnSetConfig=-

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBamp-Cap8-SMR.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\Cap8-Reversed-Lemo-Wiring.prm

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\realbase.prm 

@set OnConnect=%OnConnect% ; SET PARAMETER SubjectName           %SUBJECT%
@set OnConnect=%OnConnect% ; SET PARAMETER SubjectSession        %CONDITION%
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\condition%CONDITION%.prm

@if %MODE% == CALIB goto SKIPFREE
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\realfree.prm
:SKIPFREE

::@set SRC=Emotiv          && set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\epoc.prm && set LOGGERS=
::@set SRC=SignalGenerator && set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\epoc.prm && set LOGGERS=--EvaluateTiming=0

@set OnConnect=%OnConnect% ; SETCONFIG
::@set OnSetConfig=%OnSetConfig% ; SET STATE Running 1

start              %PYPROG%\Operator                 --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"
start              %PYPROG%\PythonApplication        --PythonAppWD=%WD%\python --PythonAppLog=- --PythonAppShell=1 --PythonAppClassFile=TrialStructure.py
start              %PYPROG%\PythonSignalProcessing   --PythonSigWD=%WD%\python --PythonSigLog=- --PythonSigShell=1 --PythonSigClassFile=Streaming.py
start              %PYPROG%\%SRC% %LOGGERS%
::start              %SRC% %LOGGERS%


:: datestamped logs
::     --PythonAppLog=%WD%\log\###-app.txt
::     --PythonSigLog=%WD%\log\###-sig.txt
