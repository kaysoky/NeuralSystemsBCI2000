@set WD=%CD%
@set PYWD=%WD%\python
@set PARMS=%WD%\parms
@set PYPROG=%WD%\prog

:: parasitizing the VA setup:  v3.x binaries for operator/SIG/APP will be packed in the local %PYPROG% subdir, but SRC will be in ../prog
@set PROG=%WD%\..\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

:: last fallback = E1001 setup: one 3.x prog directory only
@set PROG=%WD%\..\..\..\..\..\..\..\prog
@set PYPROG=%PROG%
call portable.bat
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

:gotprog


@set SESSION=002
@if [%1]==[] goto SKIPSESSIONARG
@set SESSION=%1
:SKIPSESSIONARG

@set MODE=CALIB
@if [%2]==[] goto SKIPMODEARG
@set MODE=%2
:SKIPMODEARG

@set SRC=gUSBampSource
@set LOGGERS=
@set OnConnect=-
@set OnSetConfig=-

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBamp-Cap8-SMR.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\realbase.prm 

@set OnConnect=%OnConnect% ; SET PARAMETER SubjectSession        %SESSION%
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\condition%SESSION%.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\real_subject.prm

@if %MODE% == CALIB goto SKIPFREE
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\realfree.prm
:SKIPFREE

::@set SRC=Emotiv          && set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\epoc.prm && set LOGGERS=
::@set SRC=SignalGenerator && set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\epoc.prm && set LOGGERS=--EvaluateTiming=0

::@set LOGGERS=%LOGGERS% --LogWiimote=1

::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PROG%\..\data\20110801_8502_A_S002R03_weights.prm
::@set OnConnect=%OnConnect% ; SET PARAMETER ShowSignalTime 1
::@set OnConnect=%OnConnect% ; SET PARAMETER TrialsPerBlock 1
@set OnConnect=%OnConnect% ; SETCONFIG
::@set OnSetConfig=%OnSetConfig% ; SET STATE Running 1

start              %PYPROG%\Operator                 --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"
start              %PYPROG%\PythonApplication        --PythonAppWD=%WD%\python --PythonAppLog=- --PythonAppShell=1 --PythonAppClassFile=TrialStructure.py
start              %PYPROG%\PythonSignalProcessing   --PythonSigWD=%WD%\python --PythonSigLog=- --PythonSigShell=1 --PythonSigClassFile=Streaming.py
::start              %PYPROG%\%SRC% %LOGGERS%
start              %SRC% %LOGGERS%


:: check normalizer works for non-zero signals (playback? epoc?)
:: online visual feedback (for onlookers)
:: shells off
:: datestamped logs
::     --PythonAppLog=%WD%\log\###-app.txt
::     --PythonSigLog=%WD%\log\###-sig.txt
:: visualizations off
:: auto-quit?
:: go button on operator?  or go with trialsperblock 1 for test?
:: remove driver version warning
:: inpout32.dll
:: double-click to run training blocks, double-click to run test blocks
:: DirectSound dependency, etc (? what about it ?)
:: simplify parameters (? how ?)
:: obtain cheap head- or ear-phones
