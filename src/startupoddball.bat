start operator\operat.exe
wait 0
start EEGsource\RandomNumber\RandomNumber.exe AUTOSTART 127.0.0.1
start SignalProcessing\AR\ARSignalProcessing.exe AUTOSTART 127.0.0.1
cd Application\Oddball
start Oddball.exe AUTOSTART 127.0.0.1


