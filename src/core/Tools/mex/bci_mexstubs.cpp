////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A file to hold dummy implementations for functions that are
//         unneeded/unwanted for mex files
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "Environment.h"
#include <string>

using namespace std;

// Environment
int EnvironmentBase::sNumInstances = 0;

void
Environment::OnParamAccess( const string& ) const
{
}

void
Environment::OnStateAccess( const string& ) const
{
}
