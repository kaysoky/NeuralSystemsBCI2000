@call buildutils/GetConfigOpts
cmake -DCMAKE_BUILD_TYPE=RELEASE %CMAKEOPTS% -G "MinGW Makefiles"
@echo to build, run mingw32-make
@pause
