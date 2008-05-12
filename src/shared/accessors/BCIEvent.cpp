//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A std::ostream based accessor interface to add events
//   to a globally maintained event queue.
//   Basing the accessor on std::ostream allows for convenient
//   conversion of numbers (e.g., state values) into event descriptor
//   strings.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIEvent.h"
#include "PrecisionTime.h"

using namespace std;

EventQueue* BCIEvent::spQueue = NULL;

int
BCIEvent::StringBuf::sync()
{
  int result = stringbuf::sync();
  if( BCIEvent::spQueue != NULL && !str().empty() )
    spQueue->PushBack( str().c_str(), PrecisionTime::Now() );
  str( "" );
  return result;
}

