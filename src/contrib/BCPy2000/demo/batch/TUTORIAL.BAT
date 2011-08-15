cd ..\prog

call portable.bat
:: this is necessary so that BCI2000 can find Python:  see the comments in portable.bat

start Operator
start PythonSource
start PythonSignalProcessing
start PythonApplication       --PythonAppClassFile=Tutorial.py
