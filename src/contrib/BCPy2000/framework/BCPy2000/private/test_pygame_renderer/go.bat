#! ../../../../../../../prog/BCI2000Shell
@cls & ..\..\..\..\..\..\..\prog\BCI2000Shell %0 %* #! && exit /b 0 || exit /b 1

Set environment WD ${canonical path ${parent directory $0}}
Change directory $BCI2000LAUNCHDIR
Show window; Set title ${Extract file base $0}
Reset system
Startup system localhost
Start executable           SignalGenerator       --local --EvaluateTiming=0 --FileFormat=Null
Start executable           DummySignalProcessing --local

if ${is path /usr/X11}
	Start executable xterm -e PythonApplication     --local --PythonAppWD=$WD
else
	Start executable          PythonApplication     --local --PythonAppWD=$WD
end

Wait for Connected
Set parameter VisualizeSource 0
Set parameter VisualizeTiming 1
Setconfig
