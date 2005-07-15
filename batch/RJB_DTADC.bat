cd ..\prog
start operat.exe
wait 0
start DT2000.exe AUTOSTART 127.0.0.1
start ARSignalProcessing.exe AUTOSTART 127.0.0.1
start RJB.exe AUTOSTART 127.0.0.1
cd ..