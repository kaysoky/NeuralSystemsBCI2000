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

const char*
ClassName( const std::type_info& inTypeid )
{
#ifdef __GNUC__
  static std::string result;
  int err = 0;
  char* name = abi::__cxa_demangle( inTypeid.name(), 0, 0, &err );
  if( name )
  {
    result = name;
    free( name );
  }
  else
    result = "<N/A>";
  return result.c_str();
#else
  return inTypeid.name();
#endif
}

#endif // CLASS_NAME_H
