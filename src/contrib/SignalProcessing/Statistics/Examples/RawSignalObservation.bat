:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id: CursorTask_SignalGenerator.bat 3618 2011-10-21 17:11:48Z mellinger $
:: Description: BCI2000 startup script for the WinNT shell.
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
cd ..\..\..\..\..\prog
start operator.exe ^
   --Title %~n0 ^
   --OnConnect "-LOAD PARAMETERFILE ..\parms\examples\CursorTask_SignalGenerator.prm;LOAD PARAMETERFILE ..\parms\fragments\feedback\CursorTask1D.prm"
start SignalGenerator.exe --LogMouse=1 --EvaluateTiming=0
start StatisticsSignalProcessing1.exe
start CursorTask.exe
cd ..
