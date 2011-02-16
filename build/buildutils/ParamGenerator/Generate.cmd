:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id$
:: Description: A script to generate parameter files
::
:: (C) 2000-2010, BCI2000 Project
:: http://www.bci2000.org
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
if [%2]==[] echo Usage: Generate [type] [number of files] && goto end

for /L %%i in (2,1,%2) do (
  ParamGenerator %1 > ..\tests\parms\%1_%%i.prm
  echo Application float MinRunLength= 10s >> ..\tests\parms\%1_%%i.prm
  echo Application int NumberOfSequences= 1 >> ..\tests\parms\%1_%%i.prm
  echo P3TemporalFilter int EpochsToAverage= 1 >> ..\tests\parms\%1_%%i.prm
  echo Application intlist ToBeCopied= 1 1 >> ..\tests\parms\%1_%%i.prm
  echo Application string TextToSpell= T >> ..\tests\parms\%1_%%i.prm
  echo Source string SubjectSession= %%i >> ..\tests\parms\%1_%%i.prm
)

:end
