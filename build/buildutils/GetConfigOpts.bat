@echo Please answer some questions to choose which project files should be generated.
@echo BCI2000 core modules will always be built.
@set /p ANS=Build BCI2000 tools (y/n)?
@if /i %ANS%==y (set OPT2="-DBUILD_TOOLS:BOOL=TRUE") else (set OPT2="-DBUILD_TOOLS:BOOL=FALSE")
@set /p ANS=Build contrib modules (y/n)?
@if /i %ANS%==y (set OPT1="-DBUILD_CONTRIB:BOOL=TRUE") else (set OPT1="-DBUILD_CONTRIB:BOOL=FALSE")
@set /p ANS=Build BCPy2000 modules (y/n)?
@if /i %ANS%==y (set OPT4="-DBUILD_BCPY2000:BOOL=TRUE") else (set OPT4="-DBUILD_BCPY2000:BOOL=FALSE")

@if not "%1" == "IncludingMFC" goto SkipMFC
@set /p ANS=Build modules that use MFC (y/n)? (Choose "n" if you are using Visual Studio Express)
@if /i %ANS%==y (set OPT3="-DBUILD_MFC:BOOL=TRUE") else (set OPT3="-DBUILD_MFC:BOOL=FALSE")
:SkipMFC

