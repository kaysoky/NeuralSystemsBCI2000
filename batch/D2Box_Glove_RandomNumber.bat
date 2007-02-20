:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id$
::
:: File: D2Box_Glove_RandomNumber.bat
:: Description: BCI2000 startup script for the WinNT shell.
::
:: (C) 2000-2007, BCI2000 Project
:: http://www.bci2000.org
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
cd ..\prog
start operat.exe
wait 0
start RandomNumber.exe AUTOSTART 127.0.0.1
start ARSignalProcessing.exe AUTOSTART 127.0.0.1
start D2BoxGlove.exe AUTOSTART 127.0.0.1
cd ..
