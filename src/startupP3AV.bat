start operator\operat.exe
wait 0
start EEGsource\RandomNumber\RandomNumber.exe AUTOSTART 127.0.0.1
start SignalProcessing\Dummy\DummySignalProcessing.exe AUTOSTART 127.0.0.1
cd Application\P3AV
start P3AVTask.exe AUTOSTART 127.0.0.1
cd ..\..



