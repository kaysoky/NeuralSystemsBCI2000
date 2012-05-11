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
#include "EnvVariable.h"
#include "CommandInterpreter.h"
#include "BCIException.h"

using namespace std;
using namespace Interpreter;

VariableType VariableType::sInstance;
const ObjectType::MethodEntry VariableType::sMethodTable[] =
{
  METHOD( Set ), METHOD( Get ),
  METHOD( Clear ),
  END
};

bool
VariableType::Set( CommandInterpreter& inInterpreter )
{
  string name = inInterpreter.GetToken(),
         value = inInterpreter.GetToken();
  if( inInterpreter.LocalVariables().Exists( name ) )
    inInterpreter.LocalVariables()[name] = value;
  else if( !EnvVariable::Set( name, value ) )
    throw bciexception_( "Could not set variable \"" << name << "\"" );
  return true;
}

bool
VariableType::Get( CommandInterpreter& inInterpreter )
{
  string name = inInterpreter.GetToken(),
         value;
  if( inInterpreter.LocalVariables().Exists( name ) )
    inInterpreter.Out() << inInterpreter.LocalVariables()[name];
  else if( EnvVariable::Get( name, value ) )
    inInterpreter.Out() << value;
  return true;
}

bool
VariableType::Clear( CommandInterpreter& inInterpreter )
{
  string name = inInterpreter.GetToken();
  if( inInterpreter.LocalVariables().Exists( name ) )
    inInterpreter.LocalVariables().erase( name );
  else if( !EnvVariable::Clear( name ) )
    throw bciexception_( "Could not clear variable \"" << name << "\"" );
  return true;
}
