:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id$
:: Description: BCI2000 startup script for the WinNT shell.
::
:: (C) 2000-2010, BCI2000 Project
:: http://www.bci2000.org
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
cd ..\prog
start operat.exe
start gMOBIlabPlus.exe 127.0.0.1
start ARSignalProcessing.exe 127.0.0.1
start CursorTask.exe 127.0.0.1
cd ..