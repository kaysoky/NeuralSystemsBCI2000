:: The "Full Monty" is a portable Python distribution, which means
:: it can be run from (almost) anywhere without installation.
::
:: However, in order for this to work:
:: (1)  BCI2000 will need to know where Python is.
:: (2)  Python will need to know where its own files are.
::
:: For this reason, BCI2000 batch files need to call this file,
:: portable.bat
:: 
:: First we identify the directory where portable.bat is (%0\..)
:: From there we go up (out of prog), up again (out of BCI2000), then
:: down into the FullMonty directory, then down into App: that is
:: where Python.exe lives.
::    .                                                       .
::   /|\                                                     /|\
::    |   IF YOU WANT TO SEPARATE THE BCI2000 AND FullMonty   |
::        DIRECTORIES, YOU WILL HAVE TO CHANGE THIS BEHAVIOUR.
::        The easiest way: in your BCI2000 batch file, call
::        portable.bat with an argument that specifies the
::        absolute path to the App directory:
::
::                 call portable.bat C:\Absolute\Path\To\App
:: 
:: We determine the full path to python's home, and set the "PYTHONHOME"
:: environment variable to that.  This solves problem (2). We also add
:: the PYTHONHOME directory, delimited by a semicolon, to
:: the list of paths in the system environment variable "Path":  that
:: solves problem (1).   Set these two environment variables permanently
:: ( Control Panel-> System -> Advanced -> Environment Variables )
:: if you wish to permanently "install" your Full Monty distro in a fixed
:: location and never worry about this again.
::
:: Finally, as a nice-to-have, we set the IPYTHONDIR variable, such that
:: IPython shells take their user config from a place we have pre-prepared
:: inside the Full Monty distro. This prevents BCI2000-driven IPython
:: shells from prompting/distracting the user the very first time they are
:: used, and also ensures that some useful shortcuts are loaded.

@set PYTHONHOME="%1"
@if "%1"=="" set PYTHONHOME=%0\..\..\..\..\FullMonty254-20110710\App

@set OLDDIR=%CD%
@cd  %PYTHONHOME%
@set PYTHONHOME=%CD%
@cd  %OLDDIR%

@set PATH=%PYTHONHOME%;%PATH%
@set IPYTHONDIR=%PYTHONHOME%\_ipython_userconfig_bcpy2000
