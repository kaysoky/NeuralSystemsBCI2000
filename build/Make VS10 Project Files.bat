@setlocal
@call buildutils/GetConfigOpts IncludingMFC
cmake %OPT1% %OPT2% %OPT3% %OPT4% -G"Visual Studio 10"
@pause