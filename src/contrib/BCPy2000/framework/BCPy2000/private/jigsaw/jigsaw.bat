start /MIN .\dist\jigsaw.exe
set JIGSAWDIR=%CD%
cd \BCIHomeSystemFiles\BCIAddons\games
cd ..\..\VA_BCI2000\prog
start operat.exe --OnConnect "-LOAD PARAMETERFILE %JIGSAWDIR%\jigsaw.prm"
rem start SignalGenerator.exe 127.0.0.1
rem ^for simulation purposes^
start gUSBampSource.exe 127.0.0.1
start P3SignalProcessing.exe 127.0.0.1
P3Speller.exe 127.0.0.1
taskkill /IM jigsaw.exe
taskkill /F /IM jigsaw.exe
exit 0