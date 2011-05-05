@set CMAKEOPTS=
@type CMakeCache.txt >NUL 2>NUL && echo Using config options from existing CMakeCache.txt && echo (to be asked config questions again, remove the cache file or run cmake -i) && goto End

@echo Please answer some questions to choose which project files should be generated.
@echo BCI2000 core modules will always be built.

@set OPT=BUILD_TOOLS
@set /p ANS=Build BCI2000 tools (y/n)?
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=BUILD_CONTRIB
@set /p ANS=Build contrib modules (y/n)?
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set OPT=BUILD_BCPY2000
@set /p ANS=Build BCPy2000 modules (y/n)?
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )

@set /p ANS=Do you want to use any contributed framework extensions (y/n)?
@if /i %ANS%==y ( call ..\src\contrib\Extensions\GetConfigOpts )


@if not "%1" == "IncludingMFC" goto SkipMFC
@set OPT=BUILD_MFC
@set /p ANS=Build modules that use MFC (y/n)? (Choose "n" if you are using Visual Studio Express)
@if /i %ANS%==y ( set CMAKEOPTS=%CMAKEOPTS% -D%OPT%:BOOL=TRUE )
:SkipMFC


:End