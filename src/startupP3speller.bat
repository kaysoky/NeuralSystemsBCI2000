start operator\operat.exe
wait 0
rem start EEGsource\DTADC\DT2000.exe AUTOSTART 127.0.0.1
start EEGsource\RandomNumber\RandomNumber.exe AUTOSTART 127.0.0.1
start SignalProcessing\P3\P3SignalProcessing.exe AUTOSTART 127.0.0.1
cd Application\P3Speller
start P3Speller.exe AUTOSTART 127.0.0.1
cd ..\..




