////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Parameter-related object types for the script interpreter.
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

#include "ParameterTypes.h"

#include "SystemTypes.h"
#include "CommandInterpreter.h"
#include "StateMachine.h"
#include "BCI_OperatorLib.h"
#include "Lockable.h"
#include "BCIException.h"
#include "FileUtils.h"
#include "WildcardMatch.h"
#include <cstdlib>

using namespace std;
using namespace bci;
using namespace Interpreter;

//// ParameterType
ParameterType ParameterType::sInstance;
const ObjectType::MethodEntry ParameterType::sMethodTable[] =
{
  METHOD( Set ), METHOD( Get ),
  METHOD( Insert ), { "Add", &Insert },
  METHOD( List ),
  METHOD( Exists ), { "Is", &Exists },
  END
};

bool
ParameterType::Set( CommandInterpreter& inInterpreter )
{
  string line = inInterpreter.GetRemainder();
  istringstream iss( line );
  Param param;
  if( iss >> param )
  {
    Lock<StateMachine> lock( inInterpreter.StateMachine() );
    ParamList& list = inInterpreter.StateMachine().Parameters();
    if( !list.Exists( param.Name() ) )
      throw bciexception_( "Parameter " << param.Name() << " does not exist" );
    if( list[param.Name()].Section() == "System" )
      throw bciexception_( "Cannot set parameter from the \"System\" section" );
    list[param.Name()].AssignValues( param );
    inInterpreter.StateMachine().ExecuteCallback( BCI_OnParameter, line.c_str() );
    inInterpreter.StateMachine().ParameterChange();
    inInterpreter.Log() << "Set parameter " << param.Name();
  }
  else
  {
    inInterpreter.Unget();
    string name, value;
    ostringstream oss;
    {
      Lock<StateMachine> lock( inInterpreter.StateMachine() );
      ParamRef param = GetParamRef( inInterpreter );
      if( param->Section() == "System" )
        throw bciexception_( "Cannot set system parameter" );
      name = param->Name();
      value = inInterpreter.GetToken();
      param = value;
      if( !name.empty() )
        param->WriteToStream( oss );
    }
    if( !oss.str().empty() )
    {
      inInterpreter.StateMachine().ExecuteCallback( BCI_OnParameter, oss.str().c_str() );
      inInterpreter.StateMachine().ParameterChange();
      inInterpreter.Log() << "Set parameter " << name << " to " << value;
    }
  }
  return true;
}

bool
ParameterType::Get( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  inInterpreter.Out() << GetParamRef( inInterpreter );
  return true;
}

bool
ParameterType::Insert( CommandInterpreter& inInterpreter )
{
  string line = inInterpreter.GetRemainder();
  istringstream iss( line );
  Param param;
  if( !( iss >> param ) )
    throw bciexception_( "Invalid parameter line: " << line );
  {
    Lock<StateMachine> lock( inInterpreter.StateMachine() );
    if( inInterpreter.StateMachine().SystemState() != StateMachine::Idle
        && inInterpreter.StateMachine().SystemState() != StateMachine::Publishing )
      throw bciexception_( "Can not add parameter to list after publishing phase" );
    inInterpreter.StateMachine().Parameters().Add( param );
  }
  inInterpreter.StateMachine().ExecuteCallback( BCI_OnParameter, line.c_str() );
  inInterpreter.Log() << "Added parameter to list: " << param.Name();
  return true;
}

bool
ParameterType::List( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  ParamRef param = GetParamRef( inInterpreter );
  inInterpreter.Out() << *param.operator->();
  return true;
}

bool
ParameterType::Exists( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  bool exists = inInterpreter.StateMachine().Parameters().Exists( inInterpreter.GetToken() );
  inInterpreter.Out() << exists ? "true" : "false";
  return true;
}

ParamRef
ParameterType::GetParamRef( CommandInterpreter& inInterpreter )
{
  string name = inInterpreter.GetToken();
  if( name.empty() )
    throw bciexception_( "Expected a parameter name, with optional indices in parentheses" );
  CommandInterpreter::ArgumentList args;
  inInterpreter.ParseArguments( name, args );
  if( !inInterpreter.StateMachine().Parameters().Exists( name ) )
    throw bciexception_( "Parameter " << name << " does not exist" );

  Param* pParam = &inInterpreter.StateMachine().Parameters()[name];
  if( pParam->NumValues() < 1 )
    throw bciexception_( "Parameter " << name << " is empty" );
  try
  {
    size_t maxIdx = 0,
           nIdx = 0,
           i = 0;
    while( args.size() > 0 && i < args.size() - 1 )
    {
      nIdx = args[i].size();
      maxIdx = max( nIdx, maxIdx );
      size_t idx1 = 0,
             idx2 = 0;
      if( nIdx > 0 )
        idx1 = GetIndex( args[i][0], pParam->RowLabels() );
      if( nIdx > 1 )
        idx2 = GetIndex( args[i][1], pParam->ColumnLabels() );
      switch( nIdx )
      {
        case 0:
          pParam = pParam->Value().ToParam();
          break;
        case 1:
          pParam = pParam->Value( idx1 ).ToParam();
          break;
        case 2:
          pParam = pParam->Value( idx1, idx2 ).ToParam();
          break;
      }
      ++i;
    }
    size_t idx1 = ParamRef::none,
           idx2 = ParamRef::none;
    if( i < args.size() )
    {
      nIdx = args[i].size();
      maxIdx = max( nIdx, maxIdx );
      if( maxIdx > 2 )
        throw bciexception_( "Too many (" << maxIdx << ") indices" );
      if( nIdx > 0 )
        idx1 = GetIndex( args[i][0], pParam->RowLabels() );
      if( nIdx > 1 )
        idx2 = GetIndex( args[i][1], pParam->ColumnLabels() );
    }
    return ParamRef( pParam, idx1, idx2 );
  }
  catch( const BCIException& e )
  {
    throw bciexception_( name << ": " << e.what() );
  }
}

size_t
ParameterType::GetIndex( const string& inAddress, const LabelIndex& inLabels )
{
  int result = 0;
  if( inAddress.find_first_not_of( "0123456789" ) == string::npos )
  {
    result = ::atoi( inAddress.c_str() ) - 1;
    if( result < 0 || result >= inLabels.Size() )
      throw bciexception_( "Index " << inAddress << " out of range" );
  }
  else if( inLabels.Exists( inAddress ) )
  {
    result = inLabels[inAddress];
  }
  else
  {
    throw bciexception_( "Label " << inAddress << " does not exist" );
  }
  return result;
}

//// ParametersType
ParametersType ParametersType::sInstance;
const ObjectType::MethodEntry ParametersType::sMethodTable[] =
{
  METHOD( Load ), METHOD( List ),
  METHOD( Apply ), METHOD( Clear ),
  END
};

bool
ParametersType::Load( CommandInterpreter& inInterpreter )
{
  return ParameterfileType::Load( inInterpreter );
}

bool
ParametersType::List( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  string pattern = inInterpreter.GetOptionalRemainder();
  if( pattern.empty() )
    pattern = "*";
  const ParamList& parameters = inInterpreter.StateMachine().Parameters();
  for( int i = 0; i < parameters.Size(); ++i )
    if( WildcardMatch( pattern, parameters[i].Name(), false ) )
      inInterpreter.Out() << parameters[i] << '\n';
  return true;
}

bool
ParametersType::Apply( CommandInterpreter& inInterpreter )
{
  return SystemType::SetConfig( inInterpreter );
}

bool
ParametersType::Clear( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  if( inInterpreter.StateMachine().SystemState() != StateMachine::Idle )
    throw bciexception_( "Must be in idle state to clear parameters" );
  inInterpreter.StateMachine().Parameters().Clear();
  return true;
}

//// ParameterfileType
ParameterfileType ParameterfileType::sInstance;
const ObjectType::MethodEntry ParameterfileType::sMethodTable[] =
{
  METHOD( Load ),
  END
};

bool
ParameterfileType::Load( CommandInterpreter& inInterpreter )
{
  string fileName = inInterpreter.GetToken();
  if( fileName.empty() )
    throw bciexception_( "Must specify a file name" );
  fileName = FileUtils::AbsolutePath( fileName );
  if( !inInterpreter.StateMachine().Parameters().Load( fileName.c_str(), false ) )
    throw bciexception_( "Could not load \"" << fileName << "\" as a parameter file" );
  inInterpreter.StateMachine().ParameterChange();
  inInterpreter.Log() << "Loaded parameter file \"" << fileName << "\"";
  return true;
}

