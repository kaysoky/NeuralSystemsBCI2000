//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A commandline version of the Operator module.
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
///////////////////////////////////////////////////////////////////////
#include "../OperatorLib/BCI_OperatorLib.h"

#include <string>
#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;

void ExecuteScript( const char* );
// Operator Library Callbacks
void STDCALL OnConnect( void* );
void STDCALL OnSetConfig( void* );
void STDCALL OnStart( void* );
void STDCALL OnResume( void* );
void STDCALL OnSuspend( void* );
void STDCALL OnQuitRequest( void*, const char** );
void STDCALL OnDebugMessage( void*, const char* );
void STDCALL OnLogMessage( void*, const char* );
void STDCALL OnWarningMessage( void*, const char* );
void STDCALL OnErrorMessage( void*, const char* );
int  STDCALL OnUnknownCommand( void*, const char* inCommand );
void STDCALL OnScriptHelp( void*, const char** outHelp );

volatile bool gTerminated = false;
const char* gScript_OnConnect = NULL;
const char* gScript_OnSetConfig = NULL;
const char* gScript_OnStart = NULL;
const char* gScript_OnResume = NULL;
const char* gScript_OnSuspend = NULL;
const char* gScript_OnQuit = NULL;

int
main( int argc, char* argv[] )
{
  int i = 1;
  while( i < argc )
  {
    if( stricmp( "--OnConnect", argv[i] ) == 0 )
      gScript_OnConnect = argv[++i];
    else if( stricmp( "--OnExit", argv[i] ) == 0 )
      gScript_OnQuit = argv[++i];
    else if( stricmp( "--OnSetConfig", argv[i] ) == 0 )
      gScript_OnSetConfig = argv[++i];
    else if( stricmp( "--OnSuspend", argv[i] ) == 0 )
      gScript_OnSuspend = argv[++i];
    else if( stricmp( "--OnResume", argv[i] ) == 0 )
      gScript_OnResume = argv[++i];
    else if( stricmp( "--OnStart", argv[i] ) == 0 )
      gScript_OnStart = argv[++i];
    ++i;
  }

  BCI_Initialize();

  BCI_SetCallback( BCI_OnConnect, BCI_Function( OnConnect ), NULL );
  BCI_SetCallback( BCI_OnSetConfig, BCI_Function( OnSetConfig ), NULL );
  BCI_SetCallback( BCI_OnStart, BCI_Function( OnStart ), NULL );
  BCI_SetCallback( BCI_OnResume, BCI_Function( OnResume ), NULL );
  BCI_SetCallback( BCI_OnSuspend, BCI_Function( OnSuspend ), NULL );
  BCI_SetCallback( BCI_OnQuitRequest, BCI_Function( OnQuitRequest ), NULL );

  BCI_SetCallback( BCI_OnDebugMessage, BCI_Function( OnDebugMessage ), NULL );
  BCI_SetCallback( BCI_OnLogMessage, BCI_Function( OnLogMessage ), NULL );
  BCI_SetCallback( BCI_OnWarningMessage, BCI_Function( OnWarningMessage ), NULL );
  BCI_SetCallback( BCI_OnErrorMessage, BCI_Function( OnErrorMessage ), NULL );

  BCI_SetCallback( BCI_OnScriptError, BCI_Function( OnErrorMessage ), NULL );
  BCI_SetCallback( BCI_OnScriptHelp, BCI_Function( OnScriptHelp ), NULL );
  BCI_SetCallback( BCI_OnUnknownCommand, BCI_Function( OnUnknownCommand ), NULL );

  BCI_Startup( "SignalSource:4000 SignalProcessing:4001 Application:4002" );

  cout << "> " << flush;
  string line;
  while( !gTerminated && getline( cin, line ) )
  {
    const char* result = BCI_ExecuteScriptWithResult( line.c_str() );
    if( result )
    {
      switch( *result )
      {
        case '\0':
          break;
        case '\\':
          cout << result + 1 << '\n';
          break;
        default:
          cout << result << '\n';
      }
      BCI_ReleaseObject( result );
    }
    if( !gTerminated )
      cout << "> " << flush;
  }
  BCI_Shutdown();
  BCI_Dispose();
}

void
ExecuteScript( const char* inScript )
{
  string s = inScript;
  if( !s.empty() )
  {
    if( s[ 0 ] == '-' )
    {
      s = s.substr( 1 );
      BCI_ExecuteScript( s.c_str() );
    }
    else
    {
      ifstream file( s.c_str() );
      if( !file.is_open() )
      {
        string err = "Could not open script file ";
        err += s;
        OnErrorMessage( NULL, err.c_str() );
      }
      else
      {
        getline( file, s, '\0' );
        BCI_ExecuteScript( s.c_str() );
      }
    }
  }
}

void STDCALL
OnConnect( void* )
{
  if( gScript_OnConnect )
  {
    cout << "Executing script after all modules connected ..." << endl;
    ExecuteScript( gScript_OnConnect );
  }
  cout << "> " << flush;
}

void STDCALL
OnSetConfig( void* )
{
  if( gScript_OnSetConfig )
  {
    cout << "Executing OnSetConfig script ..." << endl;
    ExecuteScript( gScript_OnSetConfig );
  }
}

void STDCALL
OnStart( void* )
{
  if( gScript_OnStart )
  {
    cout << "Executing OnStart script ..." << endl;
    ExecuteScript( gScript_OnStart );
  }
}

void STDCALL
OnResume( void* )
{
  if( gScript_OnResume )
  {
    cout << "Executing OnResume script ..." << endl;
    ExecuteScript( gScript_OnResume );
  }
}

void STDCALL
OnSuspend( void* )
{
  if( gScript_OnSuspend )
  {
    cout << "Executing OnSuspend script ..." << endl;
    ExecuteScript( gScript_OnSuspend );
  }
}

void STDCALL
OnQuitRequest( void* inData, const char** )
{
  if( gScript_OnQuit )
  {
    cout << "Executing OnExit script ..." << endl;
    ExecuteScript( gScript_OnQuit );
  }
  gTerminated = true;
}

void STDCALL
OnDebugMessage( void*, const char* s )
{
  cout << s << endl;
}

void STDCALL
OnLogMessage( void*, const char* s )
{
  cout << s << endl;
}

void STDCALL
OnWarningMessage( void*, const char* s )
{
  cout << "Warning: " << s << endl;
}

void STDCALL
OnErrorMessage( void*, const char* s )
{
  cout << "Error: " << s << endl;
}

int STDCALL
OnUnknownCommand( void* inData, const char* inCommand )
{
  int result = BCI_NotHandled;
  const char* runCommand = "Run Script";
  if( ::strnicmp( inCommand, runCommand, ::strlen( runCommand ) ) == 0 )
  {
    result = BCI_Handled;
    const char* p = inCommand + ::strlen( runCommand );
    while( *p && ::isspace( *p ) )
      ++p;
    ExecuteScript( p );
  }
  return result;
}

void STDCALL
OnScriptHelp( void* inData, const char** outHelp )
{
  *outHelp = "Run Script <script>";
}
