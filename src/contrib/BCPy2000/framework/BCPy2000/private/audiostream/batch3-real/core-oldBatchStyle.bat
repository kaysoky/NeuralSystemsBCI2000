@set WD=%CD%
@set PYWD=%WD%\..\python
@set PARMS=%WD%\..\parms
@set LOCALPROG=%WD%\..\prog

:: parasitizing the VA setup:  v3.x binaries for operator/SIG/APP will be packed in the local %LOCALPROG% subdir, but SRC could be in the main ../../prog
@set PARENTPROG=%WD%\..\..\prog
@if exist %PARENTPROG% goto gotprog

:: full 3.x hierarchy (like E1001 setup)
@set PARENTPROG=%WD%\..\..\..\..\..\..\..\..\prog
::call %WD%\portable.bat C:\FullMonty254-20110710\App
@if exist %PARENTPROG% goto gotprog

:gotprog
@if not exist %PARENTPROG% set PARENTPROG=%LOCALPROG%
@if not exist %LOCALPROG%  set LOCALPROG=%PARENTPROG%
@cd %PARENTPROG%

@set SUBJECT=TestSubject
@if not [%1]==[] set SUBJECT=%1

@set CONDITION=002
@if not [%2]==[] set CONDITION=%2

@set MODE=CALIB
@if not [%3]==[] set MODE=%3

@set SRC=SignalGenerator
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

@set WEIGHTS=%PARENTPROG%\..\data\%SUBJECT%%MODE%%CONDITION%\ChosenWeights.prm
@if not exist %WEIGHTS% goto SkipWeights
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WEIGHTS%
:SkipWeights

@set OnConnect=%OnConnect% ; SETCONFIG
::@set OnSetConfig=%OnSetConfig% ; SET STATE Running 1

start              %LOCALPROG%\Operator                 --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"
::start              %SRC%                             AUTOSTART 127.0.0.1 --SignalSourceIP=127.0.0.1     %LOGGERS%
start              %LOCALPROG%\%SRC%                    AUTOSTART 127.0.0.1 --SignalSourceIP=127.0.0.1     %LOGGERS%
start              %LOCALPROG%\PythonSignalProcessing   AUTOSTART 127.0.0.1 --SignalProcessingIP=127.0.0.1 --PythonSigWD=%PYWD% --PythonSigClassFile=Streaming.py      --PythonSigLog=%PYWD%\..\log\###-sig.txt --PythonSigShell=1
start              %LOCALPROG%\PythonApplication        AUTOSTART 127.0.0.1 --ApplicationIP=127.0.0.1      --PythonAppWD=%PYWD% --PythonAppClassFile=TrialStructure.py --PythonAppLog=%PYWD%\..\log\###-app.txt --PythonAppShell=0

:: datestamped logs
::     --PythonAppLog=%PYWD%\..\log\###-app.txt
::     --PythonSigLog=%PYWD%\..\log\###-sig.txt
