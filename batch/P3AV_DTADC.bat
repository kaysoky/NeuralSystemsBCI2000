cd ..\prog
start operat.exe
wait 0
start DT2000.exe AUTOSTART 127.0.0.1
start P3SignalProcessing.exe AUTOSTART 127.0.0.1
start P3AVTask.exe AUTOSTART 127.0.0.1
cd ..