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
#ifndef DYNAMIC_PYTHON
#define DYNAMIC_PYTHON 1
#endif // DYNAMIC_PYTHON

#if DYNAMIC_PYTHON

#define NO_IMPORT_ARRAY
#ifdef _WIN32
#include "python26/include/Python.h"
#include "python26/include/numpy/arrayobject.h"
#else
#include "python2.6/Python.h"
#include "numpy/arrayobject.h"
#endif
#define PYTHON_API_NAMESPACE PyAPI26
#include "PythonWrapper.cpp"

#endif // DYNAMIC_PYTHON
