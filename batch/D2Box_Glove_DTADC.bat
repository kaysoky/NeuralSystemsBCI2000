cd ..\prog
start operat.exe
wait 0
start DT2000.exe AUTOSTART 127.0.0.1
start ARSignalProcessing.exe AUTOSTART 127.0.0.1
start D2BoxGlove.exe AUTOSTART 127.0.0.1
cd ..
