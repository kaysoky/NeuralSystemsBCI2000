cd ..\prog
start operat.exe                           --OnConnect "-LOAD PARAMETERFILE ..\parms\PythonDemo1_Triangle.prm"
start PythonSource.exe           127.0.0.1 --PythonSrcClassFile=TrefoilSource.py
start PythonSignalProcessing.exe 127.0.0.1 --PythonSigClassFile=
start PythonApplication.exe      127.0.0.1 --PythonAppClassFile=TriangleApplication.py
