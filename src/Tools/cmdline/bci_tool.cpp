////////////////////////////////////////////////////////////////////
// File:        bci_tool.cpp
// Date:        Jul 21, 2003
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: Provides a basic structure for platform independent
//              command line tools.
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////
#include "bci_tool.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include <io.h>
#include <fcntl.h>

using namespace std;

int main( int argc, const char** argv )
{
  setmode( fileno( stdin ), O_BINARY );
  setmode( fileno( stdout ), O_BINARY );

  if( ToolInfo[ name ] == "" )
  {
    size_t nameBegin = string( argv[ 0 ] ).find_last_of( "/\\" );
    ToolInfo[ name ] = ( nameBegin == string::npos ? argv[ 0 ] : argv[ 0 ] + nameBegin + 1 );
    size_t extensionBegin = ToolInfo[ name ].rfind( "." );
    if( extensionBegin != string::npos )
      ToolInfo[ name ].erase( extensionBegin );
  }

  ToolResult result = ToolInit();
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
  } options =
  {
    true,
    false,
    false,
    NULL,
  };
  int i = 0;
  while( ++i < argc && argv[ i ][ 0 ] == '-' )
  {
    switch( argv[ i ][ 1 ] )
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
            options.inputFile = argv[ i ] + 2;
            if( options.inputFile == "" )
            {
              options.execute = false;
              options.help = true;
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

  if( result == noError && options.execute )
  {
    result = ToolMain( toolOptions, cin, cout );
#if 0
    if( !( in->good() || !in->eof() || result == fileIOError ) )
    {
      cerr << "Illegal data format" << endl;
      result = illegalInput;
    }
#endif
  }
  
  options.help |= ( result == illegalOption );
  if( options.help )
  {
    ostream& out = ( result == noError ? cout : cerr );
    out << "Usage: " << ToolInfo[ name ] << " [OPTION]\n"
        << "Options are:\n"
        << "\t-h,       --help        \tDisplay this help\n"
        << "\t-v,       --version     \tOutput version information\n"
        << "\t-i<file>, --input<file> \tGet input from <file>\n";
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

