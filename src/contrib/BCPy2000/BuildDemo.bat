@echo off & setlocal

echo .
echo This script builds the three BCPy2000 foundation binaries in
echo the demo\prog directory.
echo .
echo The demo needs an operator as well, so at the end, this script  
echo builds operat.exe from the BCI2000 core section, and copies it  
echo down to the demo\prog directory.                                
echo .
echo Note that we are compiling and linking against official BCI2000
echo code from outside this tree, so nothing will work unless the
echo BCPy2000 source distribution is in its correct location within
echo the main BCI2000 source tree. Apologies to those who have only
echo downloaded BCPy200, but we're not allowed to distribute the main
echo BCI2000 source code, which is available for free to researchers,
echo but subject to registration and completion of a material transfer
echo agreement --- see http://bci2000.org/BCI2000/Download.html
echo .
echo This script uses the free Borland C++ command-line tools        
echo bpr2mak.exe and make.exe .  Make sure that these are on your    
echo PATH, and not overshadowed by anything of the same name (like   
echo GNU make under cygwin, for example).                            
echo .


set base=%cd%
set dest=%base%\demo\prog

cd foundation\SignalSource\
bpr2mak PythonSource.bpr
make  -fPythonSource.mak
erase   PythonSource.mak
cd %base%

cd foundation\SignalProcessing\
bpr2mak PythonSignalProcessing.bpr
make  -fPythonSignalProcessing.mak
erase   PythonSignalProcessing.mak
cd %base%

cd foundation\Application\
bpr2mak PythonApplication.bpr
make  -fPythonApplication.mak
erase   PythonApplication.mak
cd %base%

cd ..\..\core\Operator
bpr2mak operat.bpr
make  -foperat.mak
erase   operat.mak
cd %base%

copy /Y ..\..\..\prog\operat.exe %dest%\
erase %dest%\*.tds
 
