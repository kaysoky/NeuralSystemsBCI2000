////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A small framework for platform independent command
//   line tools.
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
////////////////////////////////////////////////////////////////////
#include "bci_tool.h"
#include "ExceptionCatcher.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <fcntl.h>

// Do we need to switch stdin/stdout into binary mode?
#if defined( _WIN32 ) || defined( CYGWIN )
# define HAVE_BIN_MODE 1
#endif

#ifdef HAVE_BIN_MODE
# include <io.h>
#endif

using namespace std;

int main( int argc, const char** argv )
{
  if( ToolInfo[ name ] == "" )
  {
    size_t nameBegin = string( argv[ 0 ] ).find_last_of( "/\\" );
    ToolInfo[ name ] = ( nameBegin == string::npos ? argv[ 0 ] : argv[ 0 ] + nameBegin + 1 );
    size_t extensionBegin = ToolInfo[ name ].rfind( "." );
    if( extensionBegin != string::npos )
      ToolInfo[ name ].erase( extensionBegin );
  }

  FunctionCall< ToolResult() > callInit( ToolInit );
  bool finished = ExceptionCatcher()
                 .SetMessage( "Aborting " + ToolInfo[ name ] )
                 .Run( callInit );
  if( !finished )
    return genericError;
  ToolResult result = callInit.Result();
  if( result != noError )
  {
    cerr << ToolInfo[ name ] << ": Initialization error" << endl;
    return result;
  }
  OptionSet toolOptions;

  struct options
  {
    bool        execute;
    bool        help;
    bool        version;
    const char* inputFile;
    int         bufferSize;
  } options =
  {
    true,
    false,
    false,
    NULL,
    4096,
  };
  int i = 0;
  while( ++i < argc && argv[ i ][ 0 ] == '-' )
  {
    if( ::strlen( argv[ i ] ) < 2 )
    {
      toolOptions.insert( argv[ i ] );
    }
    else switch( argv[ i ][ 1 ] )
    {
      case '-':
        {
          string longOption( &argv[ i ][ 2 ] );
          if( longOption == "help" )
          {
            options.help = true;
            options.execute = false;
          }
          else if( longOption == "version" )
          {
            options.version = true;
            options.execute = false;
          }
          else if( longOption == "input" )
          {
            if( i == argc - 1 )
            {
              options.execute = false;
              options.help = true;
            }
            else
            {
              options.inputFile = argv[ i + 1 ];
              ++i;
            }
          }
          else if( longOption == "buffer" )
          {
            if( i == argc - 1 )
            {
              options.execute = false;
              options.help = true;
            }
            else
            {
              options.bufferSize = ::atoi( argv[ i + 1 ] );
              ++i;
            }
          }
          else
            toolOptions.insert( argv[ i ] );
        }
        break;
      case 'v':
      case 'V':
        options.version = true;
        options.execute = false;
        break;
      case 'h':
      case 'H':
      case '?':
        options.help = true;
        options.execute = false;
        break;
      case 'i':
      case 'I':
        options.inputFile = argv[ i ] + 2;
        if( options.inputFile == "" )
        {
          options.execute = false;
          options.help = true;
        }
        break;
      case 'b':
      case 'B':
      {
        const char* size = argv[ i ] + 2;
        if( size == "" )
        {
          options.execute = false;
          options.help = true;
        }
        else
        {
          options.bufferSize = ::atoi( size );
        }
      } break;
      default:
        toolOptions.insert( argv[ i ] );
    }
  }
  if( i != argc )
    result = illegalOption;

  if( options.inputFile )
    if( !::freopen( options.inputFile, "rb", stdin ) )
    {
      cerr << "Could not open " << options.inputFile << " for input" << endl;
      result = fileIOError;
    }

#if 0
  const char outputFile[] = "out.~tmp";
  if( !::freopen( outputFile, "wb", stdout ) )
  {
    cerr << "Could not open " << outputFile << " for output" << endl;
    result = fileIOError;
  }
#endif

  int mode = _IOFBF;
  if( options.bufferSize < 1 )
  {
    options.bufferSize = 0;
    mode = _IONBF;
  }
  if( ::setvbuf( stdin, NULL, mode, options.bufferSize )
      ||::setvbuf( stdout, NULL, mode, options.bufferSize ) )
  {
    cerr << "Could not set buffer size to " << options.bufferSize << endl;
    result = fileIOError;
  }
#ifdef HAVE_BIN_MODE
  ::setmode( fileno( stdin ), O_BINARY );
  ::setmode( fileno( stdout ), O_BINARY );
#endif

  if( result == noError && options.execute )
  {
    FunctionCall< ToolResult( const OptionSet&, istream&, ostream& ) >
      callMain( ToolMain, toolOptions, cin, cout );
    bool finished = ExceptionCatcher()
                   .SetMessage( "Aborting " + ToolInfo[ name ] )
                   .Run( callMain );
    if( !finished )
    {
      result = genericError;
    }
    else
    {
      result = callMain.Result();
#if 0
      if( !( in->good() || !in->eof() || result == fileIOError ) )
      {
        cerr << "Illegal data format" << endl;
        result = illegalInput;
      }
#endif
    }
  }

  options.help |= ( result == illegalOption );
  if( options.help )
  {
    ostream& out = ( result == noError ? cout : cerr );
    out << "Usage: " << ToolInfo[ name ] << " [OPTION]\n"
        << "Options are:\n"
        << "\t-h,       --help        \tDisplay this help\n"
        << "\t-v,       --version     \tOutput version information\n"
        << "\t-i<file>, --input<file> \tGet input from <file>\n"
        << "\t-b<size>, --buffer<size>\tSet IO buffer to <size>\n";
    for( int i = firstOption; ToolInfo[ i ] != ""; ++i )
      out << '\t' << ToolInfo[ i ] << '\n';
    out << '\n' << ToolInfo[ description ] << '\n';
    out.flush();
  }
  if( options.version )
    cout << ToolInfo[ name ] << " " << ToolInfo[ version ] << endl;

  if( !cout )
  {
    cerr << "Error writing to standard output" << endl;
    result = genericError;
  }
  return result;
}

string OptionSet::getopt( const string& optionNames, const string& optionDefault ) const
{
  const char synonymSeparator = '|';
  istringstream is( optionNames );
  string token;
  while( getline( is, token, synonymSeparator ) )
    for( const_iterator i = begin(); i != end(); ++i )
      if( i->find( token ) == 0 )
        return i->substr( token.length() );
  return optionDefault;
}

