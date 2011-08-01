cd \BCIHomeSystemFiles\BCIAddons\games\PySpeller
rem ^rem that when no longer working from the desktop^
cd ..\..\..\VA_BCI2000\prog
rem start operat.exe --OnConnect "-LOAD PARAMETERFILE ..\..\BCIAddons\games\PySpeller\pyspeller.prm"
start operat.exe --OnConnect "-LOAD PARAMETERFILE C:\BCI2000\3.X\SRC\CONTRIB\BCPY2000\FRAMEWORK\BCPY2000\PRIVATE\WHACKA~1\WHACKA~1.PRM"
rem ^switch the above two lines (unrem the upper and rem the lower) when no longer working from the desktop^
start SignalGenerator.exe 127.0.0.1
rem ^for simulation purposes^
rem start gUSBampSource.exe 127.0.0.1
start P3SignalProcessing.exe 127.0.0.1
rem ..\..\BCIAddons\games\PySpeller\pyspeller.pyw
P3Speller.exe 127.0.0.1
taskkill /IM pythonw.exe