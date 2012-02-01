:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id$
:: Description: Recursively compares BCI2000 data files below two directories
::   given as arguments.
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
setlocal
if [%2]==[] goto usage

set OLDPATH=%PATH%
PATH="..\..\..\tools\cmdline";%PATH%
for /d %%i in ("%1\*") do (
  echo Preprocessing %%~ni
  for /f %%j in ('dir /b "%1\%%~ni\*.dat"') do (
    bci_dat2stream --transmit-ds --raw < "%1\%%~ni\%%~nj.dat" | bci_stream2asc > "%1\%%~ni\%%~nj.tmp~"
    bci_dat2stream --transmit-ds --raw < "%2\%%~ni\%%~nj.dat" | bci_stream2asc > "%2\%%~ni\%%~nj.tmp~"
  )
)
PATH=%OLDPATH%

for /d %%i in ("%1\*") do (
  for /f %%j in ('dir /b "%1\%%~ni\*.tmp~"') do (
    echo Comparing %%~nj
    bci_datadiff "%2\%%~ni\%%~nj.tmp~" "%1\%%~ni\%%~nj.tmp~" "%%~ni_Results.txt" > nul || (
      echo Differences found. See the file "%%~ni_Results.txt" for details.
      set FOUND_DIFFS=1
    )
  )
)

if defined FOUND_DIFFS goto returnfalse
goto returntrue

:usage
echo Please specify two directories to compare.
goto returnfalse

:returnfalse
verify false 2> nul

:returntrue
