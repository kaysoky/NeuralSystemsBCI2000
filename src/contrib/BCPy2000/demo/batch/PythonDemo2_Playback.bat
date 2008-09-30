cd ..\prog
start operat.exe                           --OnConnect "-LOAD PARAMETERFILE ..\parms\PythonDemo2_Playback.prm"
start PythonSource.exe           127.0.0.1 --PythonSrcClassFile=BCI2000Tools\PlaybackSourceModule.py
start PythonSignalProcessing.exe 127.0.0.1 --PythonSigClassFile=
start PythonApplication.exe      127.0.0.1 --PythonAppClassFile=TriangleApplication.py
