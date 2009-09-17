cd ..\prog
start operat.exe                 --OnConnect "-LOAD PARAMETERFILE ..\parms\PythonDemo1_Triangle.prm"
start PythonSource.exe           --PythonSrcClassFile=TrefoilSource.py
start PythonSignalProcessing.exe --PythonSigClassFile=
start PythonApplication.exe      --PythonAppClassFile=TriangleApplication.py
