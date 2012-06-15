@set WD=%CD%

@set DEST=%WD%\..\prog
@mkdir %DEST%

:: full svn checkout:  we're in src/contrib/BCPy2000/framework/BCPy2000/private/audiostream/batch
@set PROG=%WD%\..\..\..\..\..\..\..\..\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog

:: absolute path ahoy
@set PROG=C:\bci2000-svn\3.x\prog
@if exist %PROG% cd %PROG%
@if exist %PROG% goto gotprog


@echo could not find prog
@pause
@exit

:gotprog

copy BCI2000Shell.exe %DEST%\

copy Operator.exe %DEST%\
copy OperatorLib.dll %DEST%\

copy gUSBampSource.exe %DEST%\
copy Emotiv.exe %DEST%\
copy SignalGenerator.exe %DEST%\
copy tet.dll %DEST%\
copy ttime.dll %DEST%\

::copy SpectralSignalProcessing.exe %DEST%\
::copy CursorTask.exe %DEST%\

::copy DummySignalProcessing.exe %DEST%\
::copy DummyApplication.exe %DEST%\

copy PythonSource.exe %DEST%\
copy PythonSignalProcessing.exe %DEST%\
copy PythonApplication.exe %DEST%\

copy BCI2000RemoteLib.dll %DEST%\
copy BCI2000Remote.py %DEST%\

pause
