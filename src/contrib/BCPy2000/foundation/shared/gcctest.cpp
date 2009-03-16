//   $Id$
//  
//   This file is part of the BCPy2000 foundation, a set of modules for
//   the BCI2000 <http://bci2000.org/> that allow communication with a
//   Python framework built on top. It is distributed together with the
//   BCPy2000 framework.
// 
//   Copyright (C) 2007-9  Jeremy Hill, Thomas Schreiner, 
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
#include <iostream>
#include <string>
#ifndef DYNAMIC_PYTHON
#define DYNAMIC_PYTHON 1
#endif
#if DYNAMIC_PYTHON
#include "PythonWrapper.h"
#else
#include "Python.h"
#endif
int main(int argc, const char * argv[])
{
#if DYNAMIC_PYTHON
	std::string dllname;
	if(argc < 2) dllname = "libpython2.5.so";
	else dllname = argv[1];
	std::cout << dllname << std::endl;
	if(LoadPythonLinks(dllname.c_str())!=0) return -1;
#endif
	Py_Initialize();
	std::cout << std::endl;
	PyRun_SimpleString("import sys; print sys.version");
	std::cout << std::endl;
	PyRun_SimpleString("import sys; sys.argv=[]; import IPython; IPython.Shell.start().mainloop()");
	return 0;
}
// g++ -DDYNAMIC_PYTHON=1 gcctest.cpp PythonWrapper.cpp PythonLink24.cpp PythonLink25.cpp -ldl
// g++ -DDYNAMIC_PYTHON=0 gcctest.cpp -I/usr/include/python2.5 -lpython2.5
