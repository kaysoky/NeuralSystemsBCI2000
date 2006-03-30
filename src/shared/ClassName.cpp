////////////////////////////////////////////////////////////////////
// File:        ClassName.cpp
// Date:        Jan 31, 2006
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A function that hides compiler specific details
//              involved in translating the result of a
//              typeinfo::name() call into a human readable class
//              name.
////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ClassName.h"

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
