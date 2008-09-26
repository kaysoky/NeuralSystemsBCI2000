//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that holds references to state values, and
//         allows for convenient automatic type
//         conversions when accessing state values.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "StateRef.h"

const StateRef StateRef::Null;
State sNullState;
