@set WD=%CD%
@set PYWD=%WD%\python
@set PARMS=%WD%\parms
@set PYPROG=%WD%\prog

cd ..\prog
@set PROG=%CD%

@set SESSION=002
@if [%1]==[] goto SKIPSESSIONARG
@set SESSION=%1
:SKIPSESSIONARG

::call portable.bat

@set SRC=gUSBampSource
@set LOGGERS=
@set OnConnect=-
@set OnSetConfig=-

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBampsBBAAA-Cap8-SMR.prm
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\eyetracker.prm      && set LOGGERS=%LOGGERS% --LogEyetracker=1
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\tng.prm 
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\triggerless8.prm 
@set OnConnect=%OnConnect% ; SET PARAMETER EpochAveragingPersistence 1.0
@set OnConnect=%OnConnect% ; SET PARAMETER ControlFilterCutoffHz     0.0
@set OnConnect=%OnConnect% ; SET PARAMETER SubjectSession        %SESSION%
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\condition%SESSION%.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\subject_attention.prm

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
:: visualizations off
:: auto-quit?
:: go button on operator?  or go with trialsperblock 1 for test?
:: remove driver version warning
:: inpout32.dll
:: double-click to run training blocks, double-click to run test blocks
:: DirectSound dependency, etc
:: simplify parameters
:: cheap headphones