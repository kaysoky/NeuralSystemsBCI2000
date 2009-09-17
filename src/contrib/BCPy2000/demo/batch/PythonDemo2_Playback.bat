cd ..\prog
start operat.exe                 --OnConnect "-LOAD PARAMETERFILE ..\parms\PythonDemo1_Triangle.prm ; LOAD PARAMETERFILE ..\parms\PythonDemo2_Playback.prm"
start PythonSource.exe           --PythonSrcClassFile=BCI2000Tools\PlaybackSourceModule.py
start PythonSignalProcessing.exe --PythonSigClassFile=
start PythonApplication.exe      --PythonAppClassFile=TriangleApplication.py
