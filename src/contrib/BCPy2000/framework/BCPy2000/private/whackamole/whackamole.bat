start .\dist\whackamole.exe 6x6
set WHACKAMOLEDIR=%CD%
echo %WHACKAMOLEDIR%
cd \BCIHomeSystemFiles\BCIAddons\games
cd ..\..\VA_BCI2000\prog
start operat.exe --OnConnect "-LOAD PARAMETERFILE %WHACKAMOLEDIR%\dist\whackamole.prm"
rem start SignalGenerator.exe 127.0.0.1
rem ^for simulation purposes^
start gUSBampSource.exe 127.0.0.1
start P3SignalProcessing.exe 127.0.0.1
rem ..\..\BCIAddons\games\PySpeller\pyspeller.pyw
P3Speller.exe 127.0.0.1
taskkill /IM whackamole.exe
taskkill /IM whackamole.exe