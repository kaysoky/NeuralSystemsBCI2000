# TODO:
# 	Write this such that dependencies are established on the following other targets:
# 		BCI2000RemoteLib
# 		BCI2000PythonBindings
# 		BCI2000Shell
# 		Operator
# 		OperatorLib
# 		PythonApplication
# 		PythonSignalProcessing
# 		PythonSource
#
# 	...and all of the above are copied from prog to src/contrib/BCPy2000/demo/prog directory after building

#	Switch to building Python*.exe modules in the main global prog directory by default
#	Link this in to the cmake system
#	Convert demo/tutorial batch files to BCI2000Shell (problem: can't export python-specific environment variables from portable.bat...)
