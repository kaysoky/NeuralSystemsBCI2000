"""
	InstallationDir
	Log
	MODTYPE
	UseConsole
	Framework
	WD
	ClassFile
	Shell

	Called from C code:
		paramDefList, stateDefList = self._Construct()
						  outProps = self._Preflight(inProps)
									 self._Initialize(inProps, outProps)
									 self._StartRun()
							outSig = self._Process(inSig)
									 self._StopRun()
									 self._Resting()
									 self._Halt()
									 self._Destruct()
		
		
		paramDict = _get_parameters()
		stateDict = _get_states()
		
					_set_parameters(paramDict)
					_param_labels(paramName, rowLabelList, colLabelList)
		TODO: maybe collapse these together?
		
					_set_states(stateDict)
					_set_state_precisions(statePrecisionDict)
		TODO: maybe collapse these together?
					
				
		inSigArray,outSigArray,stateValArray,flagArray = _sharing_setup(inDimList, outDimList, stateList)
		TODO:  make this gracefully omittable	
"""

def go(BCI2000):
	import sys,os
	os.chdir(BCI2000['InstallationDir'])
	logfile = open(BCI2000['Log'], 'w', 0)
	sys.stderr = logfile
	sys.stdout = logfile
	
	MODTYPE = int(BCI2000['MODTYPE'])
	moduleNames = {
		'console':'EmbeddedPythonConsole',
		   'core':'BCI2000PythonCore',
				1:'BCI2000PythonSource',
				2:'BCI2000PythonSignalProcessing',
				3:'BCI2000PythonApplication',
	}
	moduleNamesInstalled = {
		'console':'BCPy2000.EmbeddedPythonConsole'
		   'core':'BCPy2000.Generic',
				1:'BCPy2000.GenericSource',
				2:'BCPy2000.GenericSignalProcessing',
				3:'BCPy2000.GenericApplication',
	}
	superClassNames = {
				1:'BciGenericSource',
				2:'BciGenericSignalProcessing',
				3:'BciGenericApplication',
	}
	subClassNames = {
				1:'BciSource',
				2:'BciSignalProcessing',
				3:'BciApplication',
	}
	
	if len(BCI2000['Framework']):
		os.chdir(BCI2000['Framework'])
		if not os.getcwd() in sys.path:
			sys.path.append(os.getcwd())
		moduleNamesInstalled = moduleNames
		
	if BCI2000['UseConsole']:
		exec('import ' + moduleNamesInstalled['console'] + ' as ' + moduleNames['console'])
	
	exec('import ' + moduleNamesInstalled['core']    + ' as ' + moduleNames['core'])
	exec('import ' + moduleNamesInstalled[MODTYPE]   + ' as ' + moduleNames[MODTYPE])
	exec('from '   + moduleNames['core']    + ' import *')
	exec('from '   + moduleNames[MODTYPE]   + ' import *')
	
	# add framework directory, and any extension subdirectories, verbosely to path
	BCPy2000PythonCore.register_framework_dir()
	
	# change to working dir (in two steps, to cope with relative paths in WD option)
	os.chdir(BCI2000['InstallationDir'])
	if len(BCI2000['WD']):
		os.chdir(BCI2000['WD'])
	
	# add working directory verbosely to path
	BCPy2000PythonCore.register_working_dir()
	
	# TODO:  could consolidate register_framework_dir, register_working_dir, and setting of installation_dir, into one call?
	# TODO:  alternatively, could bring all of register_framework_dir, register_working_dir and search_for_file code here
	
	if len(BCI2000['ClassFile']):
		devfile = BCPy2000PythonCore.search_for_file(BCI2000['ClassFile'])
		execfile(devfile)
		exec('instance = %s()' % subClassNames[MODTYPE] )
	else:
		exec('instance = %s()' % superClassNames[MODTYPE] )
	
	
	if int(BCI2000['Shell']):
		instance._enable_shell()
	
	instance.installation_dir = BCI2000['InstallationDir']
	instance._start()
	return instance
	