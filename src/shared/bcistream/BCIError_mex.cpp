////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Implementation of bcierr and bciout message handlers for a
//              Matlab MEX file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIError.h"
#include "mex.h"

using namespace std;

void
BCIError::DebugMessage( const string& message )
{
  mexWarnMsgTxt( message.c_str() );
}

void
BCIError::Warning( const string& message )
{
  mexWarnMsgTxt( message.c_str() );
}

void
BCIError::ConfigurationError( const string& message )
{
  mexErrMsgTxt( message.c_str() );
}

void
BCIError::RuntimeError( const string& message )
{
  mexErrMsgTxt( message.c_str() );
}

void
BCIError::LogicError( const string& message )
{
  mexErrMsgTxt( message.c_str() );
}
