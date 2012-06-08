@call buildutils/GetConfigOpts
cmake -DCMAKE_BUILD_TYPE=RELEASE %CMAKEOPTS% -G "CodeBlocks - MinGW Makefiles"
@pause
