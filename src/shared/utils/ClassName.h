////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A function that hides compiler specific details
//              involved in translating the result of a
//              typeinfo::name() call into a human readable class
//              name.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////
#ifndef CLASS_NAME_H
#define CLASS_NAME_H

#include <typeinfo>
#include <string>
#include <cstdlib>
#ifdef __GNUC__
# include <cxxabi.h>
#endif

namespace bci
{

const char* ClassName( const std::type_info& inTypeid );

} // namespace bci

#endif // CLASS_NAME_H

