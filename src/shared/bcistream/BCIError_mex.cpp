////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Implementation of bcierr and bciout message handlers for a
//              Matlab MEX file.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIError.h"
#include "BCIException.h"
#include "mex.h"

using namespace std;

void
BCIError::DebugMessage( const string& message )
{
  ::mexWarnMsgTxt( message.c_str() );
}

void
BCIError::Warning( const string& message )
{
  ::mexWarnMsgTxt( message.c_str() );
}

void
BCIError::ConfigurationError( const string& message )
{
  // mexErrMsgTxt() would abort execution without executing destructors,
  // thus we need to throw an exception and call mexErrMsgTxt() from the
  // catch() clause.
  throw bciexception_( message );
}

void
BCIError::RuntimeError( const string& message )
{
  ::mexWarnMsgTxt( ( "Runtime error: " + message + "\n" ).c_str () );
}

void
BCIError::LogicError( const string& message )
{
  ::mexWarnMsgTxt( ( "Logic error: " + message + "\n" ).c_str () );
}
