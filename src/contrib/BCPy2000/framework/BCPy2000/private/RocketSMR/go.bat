@set WD=%CD%
@set PYWD=%WD%\python
@set PARMS=%WD%\parms

@set PROG=%WD%\..\..\..\..\..\..\..\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

@set PROG=%PYTHONHOME%\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog
:gotprog
@set PROG=%CD%


@if [%1]==[] (
@set SESSION=006
) else (
@set SESSION=%1
)

@set NORMALIZEROUTPUT=%PARMS%\InitialNormalizerSettings%SESSION%.prm

:PythonSetup
::@set PY=C:\BCPy2000-FullMonty254-20100708\BCI2000\prog
::@call %PY%\portable.bat

:SignalSource
@set SRC=start gUSBampSource.exe
@set LOGGERS=

:SignalProcessing
@set SIG=start ARSignalProcessing_Hijacked.exe
::@set SIG=start PythonSignalProcessing.exe --PythonSigClassFile= --PythonSigWD=%PYWD%

:Application
@set APP=start PythonApplication.exe --PythonAppClassFile=BciApplication.py --PythonAppWD=%PYWD%

:Operator
@cd ..\prog
@set OnConnect=-

::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\BCISYSTEM2_ExtendedDesktop.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\SuppressVisualizations.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\SomeVisualizations.prm

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\Subject.prm
@set OnConnect=%OnConnect% ; SET PARAMETER SubjectSession        %SESSION%

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBamp-Cap16.prm               && set SRC=start gUSBampSource.exe
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\MOBILAB.prm               && set SRC=start gMOBILab.exe
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\SignalGenerator.prm       && set SRC=start SignalGenerator.exe --EvaluateTiming=0

@if %SESSION% == 009 goto SKIP_EEG_SignalProcessing
:EEG_SignalProcessing
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\EEG_IIRSignalProcessing.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\EEG_ARSignalProcessing.prm

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\Calibration.prm && set NORMALIZERSAVING=--NormalizerOutputPrm=%NORMALIZEROUTPUT%
goto SET_NORMALIZER
:SKIP_EEG_SignalProcessing
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\noCalibration.prm

:SET_NORMALIZER
@if not exist %NORMALIZEROUTPUT% goto SKIPNORMALIZEROUTPUT
:NORMALIZEROUTPUT
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %NORMALIZEROUTPUT%
:SKIPNORMALIZEROUTPUT

@if %SESSION% == 006 goto BCIEEG
goto PHYSICAL
:BCIEEG
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\EEG_SelectedChannels.prm
@goto SKIPPHYSICAL
:PHYSICAL
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\MouseHijack.prm         && set LOGGERS=%LOGGERS% --LogMouse=1
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\EyetrackerHijack.prm    && set LOGGERS=%LOGGERS% --LogEyetracker=1
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\JoystickHijack.prm      && set LOGGERS=%LOGGERS% --LogJoystick=1
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\TwoDataglovesHijack.prm && set LOGGERS=%LOGGERS% --LogDataGlove=1
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\TwoWiimotesHijack.prm   && set LOGGERS=%LOGGERS% --LogWiimote=1
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\OneWiimoteHijack.prm   && set LOGGERS=%LOGGERS% --LogWiimote=1
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\MouseDirect.prm         && set LOGGERS=%LOGGERS% --LogMouse=1
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\JoystickDirect.prm      && set LOGGERS=%LOGGERS% --LogJoystick=1
:SKIPPHYSICAL
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\Normalizer.prm

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\Design.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\LevelSettings.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\PaddleSize.prm

@if %SESSION% == 009 goto DirectControl
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\MuBetaControl.prm
goto SKIP_DirectControl
:DirectControl
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\DirectControl.prm
:SKIP_DirectControl

@set OnConnect=%OnConnect% ; SET PARAMETER VisualizeSource 1
set LOGGERS=%LOGGERS% --LogDataGlove=1

::@set OnConnect=%OnConnect% ; SETCONFIG
::@set OnSetConfig=- SET STATE Running 1
start Operator.exe --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"

%SRC% %LOGGERS%
%SIG% %NORMALIZERSAVING%
%APP%

::@dir *.exe && pause
