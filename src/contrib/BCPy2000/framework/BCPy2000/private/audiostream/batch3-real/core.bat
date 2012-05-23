@set WD=%CD%
set PYWD=%WD%\..\python
@set PARMS=%WD%\..\parms
@set PYPROG=%WD%\..\prog

:: parasitizing the VA setup:  v3.x binaries for operator/SIG/APP will be packed in the local %PYPROG% subdir, but SRC will be in ../prog
@set PROG=%WD%\..\..\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

:: last fallback = E1001 setup: one 3.x prog directory only
@set PROG=%WD%\..\..\..\..\..\..\..\..\prog
@set PYPROG=%PROG%
::call %WD%\portable.bat C:\FullMonty254-20110710\App
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

:gotprog

@set SUBJECT=TestSubject
@if not [%1]==[] set SUBJECT=%1

@set CONDITION=002
@if not [%2]==[] set CONDITION=%2

@set MODE=CALIB
@if not [%3]==[] set MODE=%3

@set SRC=gUSBampSource
@if not [%4]==[] set SRC=%4

@set MONTAGE=D
@if not [%5]==[] set MONTAGE=%5

@set LOGGERS=
@set OnConnect=-
@set OnSetConfig=-

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBamp-Cap8-SMR-REVERSED-WIRING.prm

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\realbase.prm 

@set OnConnect=%OnConnect% ; SET PARAMETER Storage:Session string SubjectName=    %SUBJECT%%MODE%
@set OnConnect=%OnConnect% ; SET PARAMETER Storage:Session string SubjectSession= %CONDITION%
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\condition%CONDITION%.prm

@if /i not %MONTAGE% == 16 goto Skip16
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBamp-Cap16.prm
:Skip16

@if /i not %MODE% == FREE goto SkipFreeParams
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\realfree.prm
:SkipFreeParams

@if /i not %SRC% == Emotiv goto SkipEmotivParams
@set SRC=Emotiv          && set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\epoc.prm && set LOGGERS=
:SkipEmotivParams

@if /i not %SRC% == SignalGenerator goto SkipSignalGeneratorParams
@set SRC=SignalGenerator && set LOGGERS=--EvaluateTiming=0
:SkipSignalGeneratorParams


@set OnConnect=%OnConnect% ; SETCONFIG
::@set OnSetConfig=%OnSetConfig% ; SET STATE Running 1

start              %PYPROG%\Operator                 --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"
start              %PYPROG%\PythonApplication        AUTOSTART 127.0.0.1 --ApplicationIP=127.0.0.1      --PythonAppWD=%PYWD% --PythonAppShell=0 --PythonAppClassFile=TrialStructure.py --PythonAppLog=%PYWD%\..\log\###-app.txt
start              %PYPROG%\PythonSignalProcessing   AUTOSTART 127.0.0.1 --SignalProcessingIP=127.0.0.1 --PythonSigWD=%PYWD% --PythonSigShell=1 --PythonSigClassFile=Streaming.py      --PythonSigLog=%PYWD%\..\log\###-sig.txt
start              %PYPROG%\%SRC%                    AUTOSTART 127.0.0.1 --SignalSourceIP=127.0.0.1     %LOGGERS%
::start              %SRC%                             AUTOSTART 127.0.0.1 --SignalSourceIP=127.0.0.1     %LOGGERS%

:: datestamped logs
::     --PythonAppLog=%PYWD%\..\log\###-app.txt
::     --PythonSigLog=%PYWD%\..\log\###-sig.txt
