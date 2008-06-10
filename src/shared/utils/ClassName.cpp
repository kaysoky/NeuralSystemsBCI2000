////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A function that hides compiler specific details
//              involved in translating the result of a
//              typeinfo::name() call into a human readable class
//              name.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ClassName.h"
#include <ctype.h>

using namespace bci;

const char*
bci::ClassName( const std::type_info& inTypeid )
{
  const char* result = inTypeid.name();
#ifdef __GNUC__
  static std::string nameBuffer;
  int err = 0;
  char* name = abi::__cxa_demangle( result, 0, 0, &err );
  if( name )
  {
    nameBuffer = name;
    free( name );
  }
  else
  {
    nameBuffer = "<N/A>";
  }
  result = nameBuffer.c_str();
#endif
  const char* p = result;
  while( *p != '\0' )
    if( ::isspace( *p++ ) )
      result = p;
  return result;
}

