SET STARTDIR=%CD%

cd ..\..\..\..\prog

SET OnConnect=-
SET OnConnect=%OnConnect% ; LOAD PARAMETERFILE %STARTDIR%\test.prm

start Operator --OnConnect "%OnConnect%"

start SignalGenerator

start HilbertSignalProcessing
::start DummySignalProcessing

start DummyApplication

cd %STARTDIR%
