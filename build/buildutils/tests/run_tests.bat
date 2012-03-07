:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id$
:: Description: BCI2000 testing script for WinNT. Automatically executes some
::   BCI2000 configurations, and compares resulting data files to set of
::   reference files. Assumes to be run from where it resides.
::
:: $BEGIN_BCI2000_LICENSE$
:: 
:: This file is part of BCI2000, a platform for real-time bio-signal research.
:: [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
:: 
:: BCI2000 is free software: you can redistribute it and/or modify it under the
:: terms of the GNU General Public License as published by the Free Software
:: Foundation, either version 3 of the License, or (at your option) any later
:: version.
:: 
:: BCI2000 is distributed in the hope that it will be useful, but
::                         WITHOUT ANY WARRANTY
:: - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
:: A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
:: 
:: You should have received a copy of the GNU General Public License along with
:: this program.  If not, see <http://www.gnu.org/licenses/>.
:: 
:: $END_BCI2000_LICENSE$
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
echo Testing BCI2000 executables:

echo Removing existing test data ...
rd /s /q data\test 2>&1 > nul || echo.

cd ..\..\..\prog

echo Testing cursor task ...

start Operator.exe ^
  --OnConnect "-LOAD PARAMETERFILE ..\build\buildutils\tests\CursorTask_test.prm; SETCONFIG" ^
  --OnSetConfig "-SET STATE Running 1" ^
  --OnSuspend "-QUIT"
start SignalGenerator.exe --TestLoggerFrequency=100Hz --TestLoggerEmitDebug=0
start ARSignalProcessing.exe
start /wait CursorTask.exe


echo Testing stimulus presentation task ...

start Operator.exe ^
  --OnConnect "-LOAD PARAMETERFILE ..\build\buildutils\tests\StimulusPresentation_test.prm; SETCONFIG" ^
  --OnSetConfig "-SET STATE Running 1" ^
  --OnSuspend "-QUIT"
start SignalGenerator.exe --TestLoggerFrequency=100Hz --TestLoggerEmitDebug=0
start DummySignalProcessing.exe 
start /wait StimulusPresentation.exe 


echo Testing P3 speller task ...

start Operator.exe ^
  --OnConnect "-LOAD PARAMETERFILE ..\build\buildutils\tests\P3Speller_test.prm; SETCONFIG" ^
  --OnSetConfig "-SET STATE Running 1" ^
  --OnSuspend "-QUIT"
start SignalGenerator.exe --TestLoggerFrequency=100Hz --TestLoggerEmitDebug=0
start P3SignalProcessing.exe 
start /wait P3Speller.exe 


cd ..\build\buildutils\tests
echo Processing data files ...
( process_test_data data\test data\ref && echo Success. ) || ( echo Executable test failed. && verify false 2> nul )

:end
