:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id$
:: Description: Recursively compares BCI2000 data files below two directories
::   given as arguments.
::
:: $BEGIN_BCI2000_LICENSE$
:: 
:: This file is part of BCI2000, a platform for real-time bio-signal research.
:: [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
rem @echo off
if [%2]==[] echo Please specify two directories to compare. && verify false 2> nul

set OLDPATH=%PATH%
PATH="..\..\..\tools\cmdline";%PATH%
for /r "%1" %%i in (*.dat) do bci_dat2stream --transmit-ds --raw < "%%i" | bci_stream2asc > "%%i.tmp~"
for /r "%2" %%i in (*.dat) do bci_dat2stream --transmit-ds --raw < "%%i" | bci_stream2asc > "%%i.tmp~"
PATH=%OLDPATH%

for /d %%i in ("%1\*") do ( set OUTFILE="%%~ni_Results.txt" && for /f %%j in ('dir /b "%1\%%~ni\*.tmp~"') do bci_datadiff "%2\%%~ni\%%~nj.tmp~" "%1\%%~ni\%%~nj.tmp~" "%%~ni_Results.txt" > nul ) || goto found_diffs
goto end

:found_diffs
echo Differences found. See the file %OUTFILE% for details.
verify false 2> nul

:end
