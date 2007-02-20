////////////////////////////////////////////////////////////////////
// $Id$
// File:        ClassName.h
// Date:        Jan 31, 2006
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A function that hides compiler specific details
//              involved in translating the result of a
//              typeinfo::name() call into a human readable class
//              name.
// $Log$
// Revision 1.3  2006/07/04 16:02:21  mellinger
// Introduced namespace "bci", put the ClassName() global function inside that namespace.
//
// (C) 2000-2007, BCI2000 Project
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

