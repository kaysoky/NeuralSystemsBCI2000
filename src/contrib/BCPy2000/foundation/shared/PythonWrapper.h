//   $Id$
//  
//   This file is part of the BCPy2000 foundation, a set of modules for
//   the BCI2000 <http://bci2000.org/> that allow communication with a
//   Python framework built on top. It is distributed together with the
//   BCPy2000 framework.
// 
//   Copyright (C) 2007-11  Jeremy Hill, Thomas Schreiner, 
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
// Description: Allows Python functions to be loaded dynamically from a DLL at
// run-time (no linking against import libraries). For the subset of the
// Python API that we need, only very minimal changes were needed to
// the client code, and the client code can still be compiled without the wrapper.
#ifndef DYNAMIC_PYTHON
#define DYNAMIC_PYTHON 1
#endif // DYNAMIC_PYTHON

#if DYNAMIC_PYTHON

#ifndef PYTHON_LINK_HEADER_MODE
#define PYTHON_LINK_HEADER_MODE 1
#endif

#ifdef PYTHON_LINK
#undef PYTHON_LINK
#endif

#ifndef PyObject
#define PyObject void
#endif
#ifndef Py_ssize_t
#define Py_ssize_t size_t // TODO: really this should be a signed int with the same size as size_t
#endif
#ifndef PyThreadState
#define PyThreadState void
#endif

////////////////////////////////////////////////////////////
#if     PYTHON_LINK_HEADER_MODE == 0   /// macros only
////////////////////////////////////////////////////////////

#define PYTHON_LINK(type,name,args) // do nothing

////////////////////////////////////////////////////////////
#elif   PYTHON_LINK_HEADER_MODE == 1 /// function prototypes
////////////////////////////////////////////////////////////

#define PYTHON_LINK(type,name,args)  extern type (*name) args;

////////////////////////////////////////////////////////////
#elif   PYTHON_LINK_HEADER_MODE == 2   ////// initialization
////////////////////////////////////////////////////////////

#define PYTHON_LINK(type,name,args)  type (*name) args;

////////////////////////////////////////////////////////////
#elif   PYTHON_LINK_HEADER_MODE == 3   // load the functions
////////////////////////////////////////////////////////////

// In non-header mode, the PYTHON_LINK macro actually loads the functions
#define PYTHON_LINK(type,name,args) \
	if(dll==0) name = 0; \
	else if ((name=(type(*)args)::GetProcAddress(dll,#name)) == 0) {\
		all_loaded = false; \
		bcierr << "failed to import " << #name << " from " << dllname << " shared library" << std::endl; \
	}

////////////////////////////////////////////////////////////
#endif /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

// Heeeere's Monty:

PYTHON_LINK(    PyObject*       ,   PyDict_New                   , (void)                            )
PYTHON_LINK(    PyObject*       ,   PyDict_GetItemString         , (PyObject*,const char*)           )
PYTHON_LINK(    int             ,   PyDict_SetItemString         , (PyObject*,const char*,PyObject*) )
PYTHON_LINK(    int             ,   PyDict_SetItem               , (PyObject*,PyObject*,PyObject*)   )
PYTHON_LINK(    void            ,   PyErr_Clear                  , (void)                            )
PYTHON_LINK(    PyObject*       ,   PyErr_Occurred               , (void)                            )
PYTHON_LINK(    void            ,   PyErr_Print                  , (void)                            )
PYTHON_LINK(    void            ,   PyEval_InitThreads           , (void)                            )
PYTHON_LINK(    void            ,   PyEval_RestoreThread         , (PyThreadState*)                  )
PYTHON_LINK(    PyThreadState*  ,   PyEval_SaveThread            , (void)                            )
PYTHON_LINK(    double          ,   PyFloat_AsDouble             , (PyObject*)                       )
PYTHON_LINK(    PyObject*       ,   PyFloat_FromDouble           , (double)                          )
PYTHON_LINK(    PyObject*       ,   PyImport_AddModule           , (char* name)                      )
PYTHON_LINK(    PyObject*       ,   PyImport_ImportModule        , (char* name)                      )
PYTHON_LINK(    long            ,   PyInt_AsLong                 , (PyObject *)                      )
PYTHON_LINK(    PyObject*       ,   PyInt_FromLong               , (long)                            )
PYTHON_LINK(    PyObject*       ,   PyList_GetItem               , (PyObject*,Py_ssize_t)            )
PYTHON_LINK(    PyObject*       ,   PyList_New                   , (Py_ssize_t size)                 )
PYTHON_LINK(    int             ,   PyList_SetItem               , (PyObject*,Py_ssize_t,PyObject*)  )
PYTHON_LINK(    Py_ssize_t      ,   PyList_Size                  , (PyObject*)                       )
PYTHON_LINK(    PyObject*       ,   PyModule_GetDict             , (PyObject*)                       )
PYTHON_LINK(    PyObject*       ,   PyObject_CallFunctionObjArgs , (PyObject*,...)                   )
PYTHON_LINK(    PyObject*       ,   PyObject_CallMethodObjArgs   , (PyObject*,PyObject*,...)         )
PYTHON_LINK(    PyObject*       ,   PyObject_CallMethod          , (PyObject*,char*,char*,...)       )
PYTHON_LINK(    PyObject*       ,   PyObject_GetAttrString       , (PyObject*,const char*)           )
PYTHON_LINK(    int             ,   PyObject_IsInstance          , (PyObject*,PyObject*)             )
PYTHON_LINK(    int             ,   PyRun_SimpleFile             , (void *fp, const char*)           )
PYTHON_LINK(    int             ,   PyRun_SimpleString           , (const char*)                     )
PYTHON_LINK(    char*           ,   PyString_AsString            , (PyObject*)                       )
PYTHON_LINK(    PyObject*       ,   PyString_FromString          , (const char*)                     )
PYTHON_LINK(    PyObject*       ,   PyString_FromStringAndSize   , (const char*,Py_ssize_t)          )
PYTHON_LINK(    Py_ssize_t      ,   PyString_Size                , (PyObject*)                       )
PYTHON_LINK(    PyObject*       ,   PyTuple_GetItem              , (PyObject*,Py_ssize_t)            )
PYTHON_LINK(    void            ,   Py_Initialize                , (void)                            )
PYTHON_LINK(    void            ,   Py_DecRef                    , (PyObject*)                       )

#if   PYTHON_LINK_HEADER_MODE == 1
////////////////////////////////////////////////////////////
#ifndef PYTHON_LINK_HEADER_ALREADY_INCLUDED
#define PYTHON_LINK_HEADER_ALREADY_INCLUDED

int LoadPythonLinks(const char* dllname=0);

// some functions for which I've had to write slow
// re-implementations because both dynamic linking
// and macro-wrapping are impossible (grrr)
int PyList_Check(PyObject* x);
int PyString_Check(PyObject* x);

#endif // PYTHON_LINK_HEADER_ALREADY_INCLUDED
////////////////////////////////////////////////////////////
#endif // PYTHON_LINK_HEADER_MODE


#undef PYTHON_LINK
#undef PYTHON_LINK_HEADER_MODE

#else // static linking

#if   PYTHON_LINK_HEADER_MODE == 2
#include "Python.h"
#endif

#endif // DYNAMIC_PYTHON
