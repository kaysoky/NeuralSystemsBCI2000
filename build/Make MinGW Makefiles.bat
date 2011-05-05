@call buildutils/GetConfigOpts
cmake %CMAKEOPTS% -G "MinGW Makefiles"
@echo to build, run mingw32-make
@pause
