start operator\operat.exe
wait 0
REM start EEGsource\DASSource\DASSource.exe AUTOSTART 127.0.0.1
REM start EEGsource\DTADC\DT2000.exe AUTOSTART 127.0.0.1
start EEGsource\RandomNumber\RandomNumber.exe AUTOSTART 127.0.0.1
start SignalProcessing\SWSignalProcessing\SignalProcessing.exe AUTOSTART 127.0.0.1
cd Application\SlowWaveApp
start SWApp.exe AUTOSTART 127.0.0.1
cd ..\..


