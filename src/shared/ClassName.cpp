////////////////////////////////////////////////////////////////////
// $Id$
// File:        ClassName.cpp
// Date:        Jan 31, 2006
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A function that hides compiler specific details
//              involved in translating the result of a
//              typeinfo::name() call into a human readable class
//              name.
// $Log$
// Revision 1.2  2006/07/04 16:02:21  mellinger
// Introduced namespace "bci", put the ClassName() global function inside that namespace.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ClassName.h"

using namespace bci;

const char*
bci::ClassName( const std::type_info& inTypeid )
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

