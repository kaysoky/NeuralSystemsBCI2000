////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Environment variable object type for the script interpreter.
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

#include "VariableType.h"
#include "CommandInterpreter.h"
#include "BCIException.h"

#if _WIN32
# include <Windows.h>
#else // _WIN32
# include <cstdlib>
#endif // _WIN32

using namespace std;
using namespace Interpreter;

VariableType VariableType::sInstance;
const ObjectType::MethodEntry VariableType::sMethodTable[] =
{
  METHOD( Set ), METHOD( Clear ),
  METHOD( Get ),
  END
};

bool
VariableType::Set( CommandInterpreter& inInterpreter )
{
  string name = inInterpreter.GetToken(),
         value = inInterpreter.GetToken();
  bool success = false;
#if _WIN32
  success = ::SetEnvironmentVariableA( name.c_str(), value.c_str() );
#else // _WIN32
  success = ( 0 == ::setenv( name.c_str(), value.c_str(), 1 ) );
#endif // _WIN32
  if( !success )
    throw bciexception_( "Could not set variable \"" << name << "\"" );
  return true;
}

bool
VariableType::Clear( CommandInterpreter& inInterpreter )
{
  string name = inInterpreter.GetToken();
  bool success = false;
#if _WIN32
  success = ::SetEnvironmentVariableA( name.c_str(), NULL );
#else // _WIN32
  success = ( 0 == ::unsetenv( name.c_str() );
#endif // _WIN32
  if( !success )
    throw bciexception_( "Could not clear variable \"" << name << "\"" );
  return true;
}

bool
VariableType::Get( CommandInterpreter& inInterpreter )
{
  string name = inInterpreter.GetToken(),
         value;
  if( GetVariable( name, value ) )
    inInterpreter.Out() << value;
  return true;
}

bool
VariableType::GetVariable( const string& inName, string& outValue )
{
#if _WIN32
  int length = ::GetEnvironmentVariableA( inName.c_str(), NULL, 0 );
  if( length > 0 )
  {
    char* pBuffer = new char[length];
    ::GetEnvironmentVariable( inName.c_str(), pBuffer, length );
    outValue = pBuffer;
    delete[] pBuffer;
  }
  return ( length > 0 );
#else // _WIN32
  const char* pValue = ::getenv( inName.c_str() );
  if( pValue )
    outValue = pValue;
  return pValue;
#endif // _WIN32
}
