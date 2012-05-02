@set WD=%CD%
@set PYWD=%WD%\..\python
@set PARMS=%WD%\..\parms

@set PROG=%WD%\..\..\..\..\..\..\..\..\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

@set PROG=%PYTHONHOME%\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog
:gotprog
@set PROG=%CD%


@set SESSION=002
@if [%1]==[] goto SKIPSESSIONARG
@set SESSION=%1
:SKIPSESSIONARG


cd ..\prog
call portable.bat

@set SRC=gUSBampSource
@set LOGGERS=
@set OnConnect=-
@set OnSetConfig=-

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBampsBB-Cap16+Audio2.prm
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\eyetracker.prm      && set LOGGERS=%LOGGERS% --LogEyetracker=1
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\tng.prm 
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\triggerless16.prm 
@set OnConnect=%OnConnect% ; SET PARAMETER EpochAveragingPersistence 1.0
@set OnConnect=%OnConnect% ; SET PARAMETER ControlFilterCutoffHz     0.0
@set OnConnect=%OnConnect% ; SET PARAMETER SubjectSession        %SESSION%
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\condition%SESSION%.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\subject_attention.prm

::@set SRC=Emotiv          && set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\epoc.prm ; LOAD PARAMETERFILE %parms%\SoundCard_2Channels.prm && set LOGGERS=
::@set SRC=SignalGenerator && set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\epoc.prm ; LOAD PARAMETERFILE %parms%\SoundCard_2Channels.prm && set LOGGERS=--EvaluateTiming=0

::@set LOGGERS=%LOGGERS% --LogWiimote=1

::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PROG%\..\data\20110801_8502_A_S002R03_weights.prm
::@set OnConnect=%OnConnect% ; SET PARAMETER ShowSignalTime 1
::@set OnConnect=%OnConnect% ; SET PARAMETER TrialsPerBlock 1
@set OnConnect=%OnConnect% ; SETCONFIG
::@set OnSetConfig=%OnSetConfig% ; SET STATE Running 1

start              Operator                 --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"
start              PythonApplication        --PythonAppWD=%PYWD% --PythonAppClassFile=TrialStructure.py
start              PythonSignalProcessing   --PythonSigWD=%PYWD% --PythonSigClassFile=Streaming.py
start              %SRC% %LOGGERS%
