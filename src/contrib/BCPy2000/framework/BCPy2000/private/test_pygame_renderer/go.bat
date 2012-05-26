
@set WD=%CD%

@cd ..\..\..\..\..\..\..\prog

@set OnConnect=-

@set OnConnect=%OnConnect% ; SET PARAMETER VisualizeSource 0
@set OnConnect=%OnConnect% ; SET PARAMETER VisualizeTiming 1
@set OnConnect=%OnConnect% ; SETCONFIG

@start Operator --OnConnect "%OnConnect%"
@start SignalGenerator --EvaluateTiming=1 --FileFormat=Null
@start DummySignalProcessing
@start PythonApplication --PythonAppWD=%WD%
