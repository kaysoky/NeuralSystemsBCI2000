@call buildutils/GetConfigOpts
cmake %OPT1% %OPT2% %OPT4% -G "MinGW Makefiles"
@echo to build, run mingw32-make
@pause
