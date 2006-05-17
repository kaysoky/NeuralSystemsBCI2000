////////////////////////////////////////////////////////////////////
// File:        ClassName.h
// Date:        Jan 31, 2006
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A function that hides compiler specific details
//              involved in translating the result of a
//              typeinfo::name() call into a human readable class
//              name.
////////////////////////////////////////////////////////////////////
#ifndef CLASS_NAME_H
#define CLASS_NAME_H

#include <typeinfo>
#include <string>
#include <cstdlib>
#ifdef __GNUC__
# include <cxxabi.h>
#endif

const char* ClassName( const std::type_info& inTypeid );

#endif // CLASS_NAME_H