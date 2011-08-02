@set WD=%CD%
@set PYWD=%WD%\python
@set PARMS=%WD%\parms

@set PROG=..\..\..\..\..\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

@set PROG=%PYTHONHOME%\..\..\BCI2000\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog
:gotprog

@set BREAKOUT=%WD%\..\..\..\..\..\..\private\Application\Games\Breakout
@if exist %BREAKOUT% goto gotbreakout
:gotbreakout



cd ..\prog
call portable.bat


@set OnConnect=-

:: Signal-processing parameters
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\gUSBampsAA-Cap16+Audio2.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\fixed.prm 
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\audiostream_wadsworth_devel.prm

:: Breakout parameters
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %BREAKOUT%\breakout\parms\Laptop.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %BREAKOUT%\breakout\parms\BCISYSTEM2_ExtendedDesktop.prm
@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %PARMS%\AudiostreamBreakoutGame.prm

:: MUST ALSO LOAD WEIGHTS
::@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE C:\BCPy2000-FullMonty254-20100708\BCI2000\data\20110801_8502_A_002\20110801_8502_A_S002R03_weights.prm

@set OnConnect=%OnConnect% ; LOAD PARAMETERFILE %WD%\subject_attention.prm
@set OnConnect=%OnConnect% ; SET PARAMETER SubjectSession 999

@set OnConnect=%OnConnect% ; INSERT STATE Stream1 3 0 0 0
@set OnConnect=%OnConnect% ; INSERT STATE Stream2 3 0 0 0
::@set OnConnect=%OnConnect% ; SETCONFIG
::@set OnSetConfig=- SET STATE Running 1

start              operat                   --OnConnect "%OnConnect%" --OnSetConfig "%OnSetConfig%"

start /D%BREAKOUT% GameBreakout

start              PythonSignalProcessing   --PythonSigWD=%WD%\python

start          gUSBampSource
