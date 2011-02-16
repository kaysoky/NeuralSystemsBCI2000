:: $Id$
:: Script to create a self-extracting archive from a 7z archive.
:: Call this script with a an archive as its argument.

@if [%1]==[] goto error

@goto doit

:error
@echo This script takes a 7z archive as an argument.
@pause
@goto end

:doit
pushd %~dp0
for %%H in (%*) do copy ..\..\src\BCI2000.sfx /b + "%%H" /b "%%~dH%%~pH%%~nH.exe"
popd
pause
:end

