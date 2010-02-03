:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id$
:: Description: BCI2000 startup script for the WinNT shell.
::
:: (C) 2000-2010, BCI2000 Project
:: http://www.bci2000.org
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
cd ..\prog
start operat.exe
start gUSBampSource.exe 127.0.0.1
start P3SignalProcessing.exe 127.0.0.1
start StimulusPresentation.exe 127.0.0.1
cd ..