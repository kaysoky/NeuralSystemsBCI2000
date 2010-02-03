////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Implementation of bcierr and bciout message handlers for a
//              console-based BCI2000 command line tool.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIError.h"
#include <iostream>

using namespace std;

#ifdef BCI_DLL
extern ostream sErr;
ostream& err_ = sErr;
#else
ostream& err_ = cerr;
#endif // BCI_DLL

void
BCIError::DebugMessage( const string& message )
{
  Warning( message );
}

void
BCIError::Warning( const string& message )
{
  if( message.length() > 1 )
    err_ << message << endl;
}

void
BCIError::ConfigurationError( const string& message )
{
  if( message.length() > 1 )
    err_ << "Configuration Error: " << message << endl;
}

void
BCIError::RuntimeError( const string& message )
{
  if( message.length() > 1 )
    err_ << "Runtime Error: " << message << endl;
}

void
BCIError::LogicError( const string& message )
{
  if( message.length() > 1 )
    err_ << "Logic Error: " << message << endl;
}
