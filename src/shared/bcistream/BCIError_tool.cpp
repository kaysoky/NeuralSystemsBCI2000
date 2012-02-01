////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Implementation of bcierr and bciout message handlers for a
//              console-based BCI2000 command line tool.
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
