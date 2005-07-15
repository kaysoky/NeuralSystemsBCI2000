cd ..\prog
start operat.exe
wait 0
start DASSource.exe AUTOSTART 127.0.0.1
start ARSignalProcessing.exe AUTOSTART 127.0.0.1
start Screen.exe AUTOSTART 127.0.0.1
cd ..
