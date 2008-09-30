:: $Id$
:: Support file installation script.
:: This script gets called with the binary target directory as its
:: argument, and with its location as its working directory.

@if [%1]==[] goto error

@goto doit

:error
@echo The install script takes a target directory as an argument.
@goto end

:doit
:: Put your installation actions here.
copy /Y ..\..\demo\prog\PythonApplication.exe %1\

:end
