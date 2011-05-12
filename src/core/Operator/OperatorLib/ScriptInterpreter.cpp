////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates interpretation of operator scripts.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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

#include "ScriptInterpreter.h"
#include "StateMachine.h"
#include "BCIDirectory.h"
#include "defines.h"
#include "BCI_OperatorLib.h"

#include <sstream>
#include <cstdio>

using namespace std;

#define PROPERTY_SETS_PARAM "VisPropertySets"

static const struct
{
  const char* name;
  uint8       value;
}
sCfgIDs[] =
{
  { "Top", CfgID::Top },
  { "Left", CfgID::Left },
  { "Width", CfgID::Width },
  { "Height", CfgID::Height },
  { "WindowTitle", CfgID::WindowTitle },
  // Graph options
  { "MinValue", CfgID::MinValue },
  { "MaxValue", CfgID::MaxValue },
  { "NumSamples", CfgID::NumSamples },
  { "ChannelGroupSize", CfgID::ChannelGroupSize },
  { "GraphType", CfgID::GraphType },
  // Graph types
  { "Polyline", CfgID::Polyline },
    // Polyline options
    { "ShowBaselines", CfgID::ShowBaselines },
    { "ChannelColors", CfgID::ChannelColors },
  { "Field2d", CfgID::Field2d },

  // Units
  { "SampleUnit", CfgID::SampleUnit },
  { "ChannelUnit", CfgID::ChannelUnit },
  { "ValueUnit", CfgID::ValueUnit },

  // Memo options
  { "NumLines", CfgID::NumLines },

  // Label lists
  { "ChannelLabels", CfgID::ChannelLabels },
  { "GroupLabels", CfgID::GroupLabels },
  { "XAxisLabels", CfgID::XAxisLabels },
  { "YAxisLabels", CfgID::YAxisLabels },
  // Marker lists
  { "XAxisMarkers", CfgID::XAxisMarkers },
  { "YAxisMarkers", CfgID::YAxisMarkers },
  // Miscellaneous
  { "ShowSampleUnit", CfgID::ShowSampleUnit },
  { "ShowChannelUnit", CfgID::ShowChannelUnit },
  { "ShowValueUnit", CfgID::ShowValueUnit },
  { "SampleOffset", CfgID::SampleOffset },

  { "Visible", CfgID::Visible },
  { "InvertedDisplay", CfgID::InvertedDisplay },
  // Filters: Set to "off" to disable a filter
  { "HPFilter", CfgID::HPFilter },
  { "LPFilter", CfgID::LPFilter },
  { "NotchFilter", CfgID::NotchFilter },

};


ScriptInterpreter::ScriptInterpreter( StateMachine& s )
: mLine( 0 ),
  mrStateMachine( s )
{
  Param p(
    "Visualize:Property%20Sets matrix " PROPERTY_SETS_PARAM "= 0 1 % % % "
    "// row titles are properties in the form \"SRCD.Left\", columns are property sets"
  );
  mrStateMachine.Parameters()[PROPERTY_SETS_PARAM] = p;
}

bool
ScriptInterpreter::Execute( const char* inScript )
{
  mLine = 0;
  bool syntaxOK = true;
  istringstream iss( inScript );
  while( !iss.eof() )
  {
    ++mLine;
    string line;
    getline( iss, line );
    syntaxOK = syntaxOK && ExecuteLine( line );
  }
  return syntaxOK;
}

bool
ScriptInterpreter::ExecuteLine( const string& inLine )
{
  if( inLine.empty() )
    return true;

  bool syntaxOK = true;
  istringstream iss( inLine );
  while( !iss.eof() )
  {
    string command;
    getline( iss >> ws, command, ';' );
    syntaxOK = syntaxOK && ExecuteCommand( command );
  }
  return syntaxOK;
}

bool
ScriptInterpreter::ExecuteCommand( const string& inCommand )
{
  bool unknownCommand = false,
       syntaxOK = true;
  istringstream iss( inCommand );
  string token;
  iss >> token;
  if( 0 == ::stricmp( token.c_str(), "LOAD" ) )
    syntaxOK = Execute_Load( iss );
  else if( 0 == ::stricmp( token.c_str(), "SET" ) )
    syntaxOK = Execute_Set( iss );
  else if( 0 == ::stricmp( token.c_str(), "INSERT" ) )
    syntaxOK = Execute_Insert( iss );
  else if( 0 == ::stricmp( token.c_str(), "SYSTEM" ) )
    syntaxOK = Execute_System( iss );
  else if( 0 == ::stricmp( token.c_str(), "SETCONFIG" ) )
    syntaxOK = Execute_SetConfig( iss );
  else if( 0 == ::stricmp( token.c_str(), "START" ) )
    syntaxOK = Execute_Start( iss );
  else if( 0 == ::stricmp( token.c_str(), "STOP" ) )
    syntaxOK = Execute_Stop( iss );
  else if( 0 == ::stricmp( token.c_str(), "QUIT" ) )
    syntaxOK = Execute_Quit( iss );
  else if( token.empty() )
    syntaxOK = true;
  else
    unknownCommand = true;
  if( unknownCommand )
    mrStateMachine.ExecuteCallback( BCI_OnUnknownCommand, inCommand.c_str() );
  if( !syntaxOK )
  {
    ostringstream oss;
    oss << "Syntax error in line " << mLine << ": " << inCommand;
    mrStateMachine.ExecuteCallback( BCI_OnScriptError, oss.str().c_str() );
  }
  return syntaxOK;
}

bool
ScriptInterpreter::Execute_Load( istream& is )
{
  bool syntaxOK = false;
  string token;
  is >> ws >> token;
  if( ::stricmp( token.c_str(), "PARAMETERFILE" ) == 0 )
  {
    syntaxOK = true;
    EncodedString fileName;
    is >> ws >> fileName;
    fileName = BCIDirectory::AbsolutePath( fileName );
    if( mrStateMachine.Parameters().Load( fileName.c_str(), false ) )
    {
      ostringstream oss;
      oss << "Successfully loaded parameter file " << fileName;
      mrStateMachine.ExecuteCallback( BCI_OnLogMessage, oss.str().c_str() );
    }
    else
    {
      ostringstream oss;
      oss << "Could not load parameter file " << fileName;
      mrStateMachine.ExecuteCallback( BCI_OnScriptError, oss.str().c_str() );
    }
  }
  return syntaxOK;
}

bool
ScriptInterpreter::Execute_Set( istream& is )
{
  bool syntaxOK = false;
  string token;
  is >> ws >> token;
  if( ::stricmp( token.c_str(), "STATE" ) == 0 )
  {
    syntaxOK = true;
    string name, value;
    is >> ws >> name >> ws >> value;
    State::ValueType intVal = atoi( value.c_str() );
    if( mrStateMachine.SetStateValue( name.c_str(), intVal ) )
    {
      State state = mrStateMachine.States()[name];
      state.SetValue( intVal );
      ostringstream oss;
      oss << state;
      mrStateMachine.ExecuteCallback( BCI_OnState, oss.str().c_str() );
      oss.str( "" );
      oss << "Set state " << name << " to " << value;
      mrStateMachine.ExecuteCallback( BCI_OnLogMessage, oss.str().c_str() );
    }
    else
    {
      ostringstream oss;
      oss << "Could not set state " << name << " to " << value;
      mrStateMachine.ExecuteCallback( BCI_OnScriptError, oss.str().c_str() );
    }
  }
  else if( ::stricmp( token.c_str(), "PARAMETER" ) == 0 )
  {
    syntaxOK = true;
    string name, value;
    is >> ws >> name >> ws >> value;
    mrStateMachine.LockData();
    if( mrStateMachine.Parameters().Exists( name ) )
    {
      mrStateMachine.Parameters()[name].Value() = value;
      mrStateMachine.UnlockData();
      ostringstream oss;
      oss << mrStateMachine.Parameters()[name];
      mrStateMachine.ExecuteCallback( BCI_OnParameter, oss.str().c_str() );
      oss.str( "" );
      oss << "Set parameter " << name << " to " << value;
      mrStateMachine.ExecuteCallback( BCI_OnLogMessage, oss.str().c_str() );
    }
    else
    {
      mrStateMachine.UnlockData();
      ostringstream oss;
      oss << "Parameter " << name << " does not exist";
      mrStateMachine.ExecuteCallback( BCI_OnScriptError, oss.str().c_str() );
    }
  }
  else if( ::stricmp( token.c_str(), "VISPROPERTY" ) == 0 )
  {
    string visID_enc, cfgID, value;
    getline( is >> ws, visID_enc, '.' );
    is >> cfgID >> ws >> value;
    istringstream iss( visID_enc );
    EncodedString visID;
    iss >> visID;
    IDType numCfgID = Resolve_VisCfg( cfgID.c_str() );
    mrStateMachine.LockData();
    if( numCfgID != static_cast<IDType>( CfgID::None ) )
    {
      syntaxOK = true;
      mrStateMachine.Visualizations()[visID].Put( numCfgID, value );
      mrStateMachine.UnlockData();
      mrStateMachine.ExecuteCallback( BCI_OnVisProperty, visID.c_str(), numCfgID, value.c_str() );
      ostringstream oss;
      oss << "Set vis property " << visID << "." << cfgID << " to " << value;
      mrStateMachine.ExecuteCallback( BCI_OnLogMessage, oss.str().c_str() );
    }
    else
    {
      syntaxOK = false;
      mrStateMachine.UnlockData();
      ostringstream oss;
      oss << "Invalid visconfig ID " << cfgID;
      mrStateMachine.ExecuteCallback( BCI_OnScriptError, oss.str().c_str() );
    }
  }
  else if( ::stricmp( token.c_str(), "VISPROPERTIES" ) == 0 )
  {
    syntaxOK = true;
    string setID;
    getline( is >> ws, setID );
    if( ApplyVisPropertySet( setID ) )
    {
      ostringstream oss;
      oss << "Applied vis property set " << setID;
      mrStateMachine.ExecuteCallback( BCI_OnLogMessage, oss.str().c_str() );
    }
    else
    {
      ostringstream oss;
      oss << "Invalid vis property set ID " << setID;
      mrStateMachine.ExecuteCallback( BCI_OnScriptError, oss.str().c_str() );
    }
  }
  return syntaxOK;
}

bool
ScriptInterpreter::Execute_Insert( istream& is )
{
  bool syntaxOK = false;
  string token;
  is >> ws >> token;
  if( ::stricmp( token.c_str(), "STATE" ) == 0 )
  {
    syntaxOK = true;
    string name, line;
    getline( is >> ws >> name, line );
    mrStateMachine.LockData();
    if( mrStateMachine.SystemState() == StateMachine::Idle
        || mrStateMachine.SystemState() == StateMachine::Publishing
        || mrStateMachine.SystemState() == StateMachine::Information )
    {
      string stateline = name + line + " 0 0";
      mrStateMachine.States().Add( stateline );
      mrStateMachine.UnlockData();
      mrStateMachine.ExecuteCallback( BCI_OnState, stateline.c_str() );
      ostringstream oss;
      oss << "Added state " << name << " to list";
      mrStateMachine.ExecuteCallback( BCI_OnLogMessage, oss.str().c_str() );
    }
    else
    {
      mrStateMachine.UnlockData();
      ostringstream oss;
      oss << "Could not add state " << name << " to list after information phase";
      mrStateMachine.ExecuteCallback( BCI_OnScriptError, oss.str().c_str() );
    }
  }
  else if( ::stricmp( token.c_str(), "PARAMETER" ) == 0 )
  {
    syntaxOK = true;
    string line;
    getline( is >> ws, line );
    mrStateMachine.LockData();
    if( mrStateMachine.SystemState() == StateMachine::Idle
        || mrStateMachine.SystemState() == StateMachine::Publishing )
    {
      mrStateMachine.Parameters().Add( line );
      mrStateMachine.UnlockData();
      mrStateMachine.ExecuteCallback( BCI_OnParameter, line.c_str() );
      ostringstream oss;
      oss << "Added parameter to list: " << line;
      mrStateMachine.ExecuteCallback( BCI_OnLogMessage, oss.str().c_str() );
    }
    else
    {
      mrStateMachine.UnlockData();
      ostringstream oss;
      oss << "Could not add parameter to list after publishing phase";
      mrStateMachine.ExecuteCallback( BCI_OnScriptError, oss.str().c_str() );
    }
  }
  return syntaxOK;
}

bool
ScriptInterpreter::Execute_System( istream& is )
{
  EncodedString command;
  is >> ws >> command;
  if( ::system( command.c_str() ) == 0 )
  {
    ostringstream oss;
    oss << "Successfully executed " << command;
    mrStateMachine.ExecuteCallback( BCI_OnLogMessage, oss.str().c_str() );
  }
  else
  {
    ostringstream oss;
    oss << "Could not start " << command;
    mrStateMachine.ExecuteCallback( BCI_OnScriptError, oss.str().c_str() );
  }
  return true;
}

bool
ScriptInterpreter::Execute_SetConfig( istream& )
{
  if( mrStateMachine.SetConfig() )
    mrStateMachine.ExecuteCallback( BCI_OnLogMessage, "Set configuration" );
  else
    mrStateMachine.ExecuteCallback( BCI_OnScriptError, "Could not set configuration" );
  return true;
}

bool
ScriptInterpreter::Execute_Start( istream& )
{
  if( mrStateMachine.StartRun() )
    mrStateMachine.ExecuteCallback( BCI_OnLogMessage, "Started operation" );
  else
    mrStateMachine.ExecuteCallback( BCI_OnScriptError, "Could not start operation" );
  return true;
}

bool
ScriptInterpreter::Execute_Stop( istream& )
{
  if( mrStateMachine.StopRun() )
    mrStateMachine.ExecuteCallback( BCI_OnLogMessage, "Stopped operation" );
  else
    mrStateMachine.ExecuteCallback( BCI_OnScriptError, "Could not stop operation" );
  return true;
}

bool
ScriptInterpreter::Execute_Quit( istream& )
{
  if( mrStateMachine.Shutdown() )
    mrStateMachine.ExecuteCallback( BCI_OnLogMessage, "Shut down system" );
  else
    mrStateMachine.ExecuteCallback( BCI_OnScriptError, "Could not shut down system" );
  return true;
}

bool
ScriptInterpreter::ApplyVisPropertySet( const std::string& inSetID )
{
  const string paramName = PROPERTY_SETS_PARAM;
  bool result = false;
  mrStateMachine.LockData();
  const ParamList& parameters = mrStateMachine.Parameters();
  if( parameters.Exists( paramName ) )
  {
    Param p = parameters[paramName];
    int col = -1;
    if( p.ColumnLabels().Exists( inSetID ) )
    {
      result = true;
      col = p.ColumnLabels()[inSetID];
    }
    else
    {
      istringstream iss( inSetID );
      if( iss >> col )
      {
        if( --col < p.NumColumns() )
          result = true;
      }
    }
    if( result )
    {
      for( int row = 0; row < p.NumRows(); ++row )
      {
        if( !p.Value( row, col ).ToString().empty() )
        {
          istringstream iss( p.RowLabels()[row] );
          string visID_enc, cfgID;
          getline( iss >> ws, visID_enc, '.' );
          iss >> cfgID;
          istringstream iss2( visID_enc );
          EncodedString visID;
          iss2 >> visID;
          IDType numCfgID = Resolve_VisCfg( cfgID.c_str() );
          if( numCfgID != static_cast<IDType>( CfgID::None ) )
          {
            string value = p.Value( row, col ).ToString();
            mrStateMachine.Visualizations()[visID].Put( numCfgID, value );
            mrStateMachine.UnlockData();
            mrStateMachine.ExecuteCallback( BCI_OnVisProperty, visID.c_str(), numCfgID, value.c_str() );
            mrStateMachine.LockData();
          }
        }
      }
    }
  }
  mrStateMachine.UnlockData();
  return result;
}

IDType
ScriptInterpreter::Resolve_VisCfg( const char* inName )
{
  IDType result = CfgID::None;
  for( size_t i = 0; result == static_cast<IDType>( CfgID::None ) 
                     && i < sizeof( sCfgIDs ) / sizeof( *sCfgIDs ); ++i )
    if( 0 == ::stricmp( inName, sCfgIDs[i].name ) )
      result = sCfgIDs[i].value;
  return result;
}

