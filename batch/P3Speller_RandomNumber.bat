:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id$
::
:: File: P3Speller_RandomNumber.bat
:: Description: BCI2000 startup script for the WinNT shell.
::
:: (C) 2000-2007, BCI2000 Project
:: http://www.bci2000.org
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
cd ..\prog
start operat.exe
wait 0
start RandomNumber.exe AUTOSTART 127.0.0.1
start P3SignalProcessing.exe AUTOSTART 127.0.0.1
start P3Speller.exe AUTOSTART 127.0.0.1
