cd ..\prog
start operat.exe
wait 0
start RandomNumber.exe AUTOSTART 127.0.0.1
start DummySignalProcessing.exe AUTOSTART 127.0.0.1
start P3AVTask.exe AUTOSTART 127.0.0.1
cd ..