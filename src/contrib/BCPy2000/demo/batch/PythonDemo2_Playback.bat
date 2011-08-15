cd ..\prog

call portable.bat
:: this is necessary so that BCI2000 can find Python:  see the comments in portable.bat

start Operator               --OnConnect "-LOAD PARAMETERFILE ..\parms\PythonDemo1_Triangle.prm ; LOAD PARAMETERFILE ..\parms\PythonDemo2_Playback.prm"
start PythonSource           --PythonSrcClassFile=BCI2000Tools\PlaybackSourceModule.py
start PythonSignalProcessing --PythonSigClassFile=
start PythonApplication      --PythonAppClassFile=TriangleApplication.py
