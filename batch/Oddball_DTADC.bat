cd ..\prog
start operat.exe
wait 0
start DT2000.exe AUTOSTART 127.0.0.1
start DummySignalProcessing.exe AUTOSTART 127.0.0.1
start Oddball.exe AUTOSTART 127.0.0.1
cd ..