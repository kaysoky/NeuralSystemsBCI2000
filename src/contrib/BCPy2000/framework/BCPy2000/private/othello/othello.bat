start /MIN .\dist\othello.exe
set OTHELLODIR=%CD%
cd \BCIHomeSystemFiles\BCIAddons\games
cd ..\..\VA_BCI2000\prog
start operat.exe --OnConnect "-LOAD PARAMETERFILE %OTHELLODIR%\othello.prm"
rem start SignalGenerator.exe 127.0.0.1
rem ^for simulation purposes^
start gUSBampSource.exe 127.0.0.1
start P3SignalProcessing.exe 127.0.0.1
P3Speller.exe 127.0.0.1
taskkill /IM othello.exe
taskkill /F /IM othello.exe
exit 0