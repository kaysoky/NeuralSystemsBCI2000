//   $Id$
//  
//   This file is part of the BCPy2000 foundation, a set of modules for
//   the BCI2000 <http://bci2000.org/> that allow communication with a
//   Python framework built on top. It is distributed together with the
//   BCPy2000 framework.
// 
//   Copyright (C) 2007-8  Thomas Schreiner, Jeremy Hill, 
//                         Christian Puzicha, Jason Farquhar
//   
//   bcpy2000@bci2000.org
//   
//   The BCPy2000 foundation is free software: you can redistribute it
//   and/or modify it under the terms of the GNU Lesser General Public
//   License as published by the Free Software Foundation, either
//   version 3 of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU Lesser General Public License for more details.
//
//   You should have received a copy of the GNU Lesser General Public License
//   along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef PythonFilterH
#define PythonFilterH

//#include "defines.h"
#include "PrecisionTime.h"

#ifndef DYNAMIC_PYTHON
#define DYNAMIC_PYTHON 1
#endif
#if DYNAMIC_PYTHON
#include "PythonWrapper.h"
#else
#include "Python.h"
#include "numpy/arrayobject.h"
#endif


#define PYTHON_CONSOLE              "EmbeddedPythonConsole"
#define PYTHON_CONSOLE_INSTALLED    "BCPy2000.EmbeddedPythonConsole"

#define PYTHON_COREMODULE           "BCI2000PythonCore"
#define PYTHON_COREMODULE_INSTALLED "BCPy2000.Generic"

#if   MODTYPE == 1
#include                            "GenericADC.h"
#define    FILTER_SUPERCLASS         GenericADC
#define    FILTER_NAME               PythonSrcADC
#define    FILTER_ABBREV            "PythonSrc"
#define	   PYTHON_MODULE            "BCI2000PythonSource"
#define	   PYTHON_MODULE_INSTALLED  "BCPy2000.GenericSource"
#define    PYTHON_SUPERCLASS        "BciGenericSource"
#define    PYTHON_SUBCLASS          "BciSource"
#define    DEFAULT_CLASS_FILE       "BciSource.py"
#elif MODTYPE == 2
#include                            "GenericFilter.h"
#define    FILTER_SUPERCLASS         GenericFilter
#define    FILTER_NAME               PythonSigFilter
#define    FILTER_ABBREV            "PythonSig"
#define	   PYTHON_MODULE            "BCI2000PythonSignalProcessing"
#define	   PYTHON_MODULE_INSTALLED  "BCPy2000.GenericSignalProcessing"
#define    PYTHON_SUPERCLASS        "BciGenericSignalProcessing"
#define    PYTHON_SUBCLASS          "BciSignalProcessing"
#define    DEFAULT_CLASS_FILE       "BciSignalProcessing.py"
#elif MODTYPE == 3
#include                            "GenericFilter.h"
#define    FILTER_SUPERCLASS         GenericFilter
#define    FILTER_NAME               PythonAppFilter
#define    FILTER_ABBREV            "PythonApp"
#define	   PYTHON_MODULE            "BCI2000PythonApplication"
#define	   PYTHON_MODULE_INSTALLED  "BCPy2000.GenericApplication"
#define    PYTHON_SUPERCLASS        "BciGenericApplication"
#define    PYTHON_SUBCLASS          "BciApplication"
#define    DEFAULT_CLASS_FILE       "BciApplication.py"
#endif


class EndUserError : public Exception {
	public:
		EndUserError(const char* s);
};

class FILTER_NAME : public FILTER_SUPERCLASS
{
	public:
		FILTER_NAME();
		virtual ~FILTER_NAME();
		virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
		virtual void Initialize( const SignalProperties&, const SignalProperties& );
		virtual void Process( const GenericSignal& Input, GenericSignal& Output );
		virtual void StartRun();
		virtual void StopRun();
		virtual void Resting();
		virtual void Halt();
	
	private:
		PrecisionTime* cur_time;
		PyObject*      bci2000_instance;
		PyThreadState* _save;
		bool           stay_open;
		bool           use_console;
		
		PyArrayObject* shared_insignal;
		PyArrayObject* shared_outsignal;
		double*        shared_statevals;
		double*        shared_flag;
		
	protected:
		void        SharingSetup(const SignalProperties &inProp, const SignalProperties &outProp);
        int         Share(const GenericSignal &inSignal, GenericSignal &outSignal);

		void        SendParametersToPython() const;
		void        ReceiveParametersFromPython();
		void        SendStatesToPython() const;
		void        ReceiveStatesFromPython() const;
		void        SendStatePrecisionsToPython() const;
	
		PyObject*   ConvertSignalToPyArrayObject(const GenericSignal& inSignal, PyArrayObject* array=NULL) const;
		void        ConvertPyArrayObjectToSignal(PyObject* pyOutSignal, GenericSignal& outSignal) const;
	
		PyObject*   ConvertPropertiesToPyObject(const SignalProperties& inSignalProperties) const;
		void        ConvertPyObjectToProperties(PyObject* pyOutSignalProperties, SignalProperties& outSignalProperties) const;
	
		PyObject*   ConvertLabelIndexToPyList(LabelIndex from) const;
		void        ConvertPyListToLabelIndex(PyObject*, LabelIndex& to) const;
		            
		PyObject*   ConvertPhysicalUnitToPyDict(PhysicalUnit from) const;
		void        ConvertPyDictToPhysicalUnit(PyObject* from, PhysicalUnit& to) const;

		void        DoubleErr(const char *msg, const char *qualifier=NULL, bool notify_restart=false) const;
		void        HandleEndUserError(EndUserError& e, std::string qualifier) const;
		void        HandleException(Exception& e, std::string qualifier) const;
		void        ChangeDir(std::string& d);
		void        OpenConsole(const char *title);
                    
		void        BlockThreads();
		void        UnblockThreads();

		std::string EscapePythonString(std::string in);
		void        EvalPythonString(std::string s);
		PyObject*   CallModuleMember(std::string module, std::string member, PyObject* arg=NULL);
		PyObject*   CallMethod(const char* name, PyObject* arg1=NULL, PyObject* arg2=NULL, PyObject* arg3=NULL) const;
		PyObject*   CallHook(const char* name, PyObject* arg1=NULL, PyObject* arg2=NULL) const;
		void        HandlePythonError(std::string msg, bool errorCodeReturned=false);
		
};

#endif // PythonFilterH
