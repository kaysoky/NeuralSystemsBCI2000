:: $Id$
:: Additional script for installation of support files.
:: This script gets called with the binary target directory as its
:: argument.

@if [%1]==[] goto error

@goto doit

:error
@echo The install script takes a target directory as an argument.
@goto end

:doit
:: Put your installation actions here.
copy /Y SIGFRIED.dll %1\
copy /Y sigfried.ini %1\

:end
