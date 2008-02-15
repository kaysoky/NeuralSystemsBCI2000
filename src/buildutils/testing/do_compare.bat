:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id$
:: Description: Recursively compares BCI2000 data files below two directories
::   given as arguments.
::
:: (C) 2000-2007, BCI2000 Project
:: http://www.bci2000.org
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
if [%2]==[] echo Please specify two directories to compare. && verify false 2> nul

set OLDPATH=%PATH%
PATH="..\..\..\tools\cmdline";%PATH%
for /r "%1" %%i in (*.dat) do bci_dat2stream --transmit-ds --raw < "%%i" | bci_stream2asc | grep -v+ "SourceTime|StimulusTime|State \{" > "%%i.tmp~"
for /r "%2" %%i in (*.dat) do bci_dat2stream --transmit-ds --raw < "%%i" | bci_stream2asc | grep -v+ "SourceTime|StimulusTime|State \{" > "%%i.tmp~"
PATH=%OLDPATH%
for /d %%i in ("%1\*") do fc /W /LB1 "%1\%%~ni\*.tmp~" "%2\%%~ni\*.tmp~" || goto found_diffs
goto end

:found_diffs
echo Differences found.
verify false 2> nul

:end
