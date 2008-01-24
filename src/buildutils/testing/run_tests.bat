:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id$
:: Description: BCI2000 testing script for WinNT. Automatically executes some
::   BCI2000 configurations, and compares resulting data files to set of
::   reference files. Assumes to be run from where it resides.
::
:: (C) 2000-2007, BCI2000 Project
:: http://www.bci2000.org
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
echo Testing BCI2000 executables:

echo Removing existing test data ...
rd /s /q data\test || echo.

cd ..\..\..\prog

echo Testing cursor task ...

start operat.exe --OnConnect "-LOAD PARAMETERFILE ..\src\buildutils\testing\CursorTask_test.prm; SETCONFIG" --OnSetConfig "-SET STATE Running 1" --OnSuspend "-QUIT"
start SignalGenerator.exe 127.0.0.1
start ARSignalProcessing.exe 127.0.0.1
start /wait CursorTask.exe 127.0.0.1


echo Testing stimulus presentation task ...

start operat.exe --OnConnect "-LOAD PARAMETERFILE ..\src\buildutils\testing\StimulusPresentation_test.prm; SETCONFIG" --OnSetConfig "-SET STATE Running 1" --OnSuspend "-QUIT"
start SignalGenerator.exe 127.0.0.1
start DummySignalProcessing.exe 127.0.0.1
start /wait StimulusPresentation.exe 127.0.0.1


echo Testing P3 speller task ...

start operat.exe --OnConnect "-LOAD PARAMETERFILE ..\src\buildutils\testing\P3Speller_test.prm; SETCONFIG" --OnSetConfig "-SET STATE Running 1" --OnSuspend "-QUIT"
start SignalGenerator.exe 127.0.0.1
start P3SignalProcessing.exe 127.0.0.1
start /wait P3Speller.exe 127.0.0.1

cd ..\src\buildutils\testing
echo Comparing data files ...
( do_compare data\ref data\test && echo Success. ) || ( echo Executable test failed. && verify false 2> nul )

