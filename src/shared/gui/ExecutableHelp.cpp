////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that displays an executable's associated
//   help file.
//   The help file is a html file that has the same name as the executable,
//   except that it bears a .html extension.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ExecutableHelp.h"
#include <fstream>
#include <sstream>

using namespace std;

const class ExecutableHelp& ExecutableHelp()
{
  static class ExecutableHelp instance( 0 );
  return instance;
}

static const string& HelpExtension()
{
  static string helpExtension = ".html";
  return helpExtension;
}

static const string& TocFileName()
{
  static string tocFileName = "htmlhelp.toc";
  return tocFileName;
}

ExecutableHelp::ExecutableHelp( int )
{
  Initialize();
  InitializeContextHelp();
}

bool
ExecutableHelp::Display() const
{
  bool result = false;
#ifdef _WIN32
  HINSTANCE err = ::ShellExecute( NULL, "open", mHelpFile.c_str(), NULL, mHelpFileDir.c_str(), SW_SHOWNORMAL );
  result = ( reinterpret_cast<int>( err ) > 32 );
  if( !result )
  {
    ::MessageBox(
      NULL,
      "The help file could not be found.\n\n"
      "Help files should be located in the\n"
      "executable's directory.\n\n"
      "Help files bear the executable's name,\n"
      "and a .html extension.",
      "Error Opening Help File",
      MB_OK | MB_ICONERROR
    );
  }
#endif // _WIN32
  return result;
}

void
ExecutableHelp::Initialize()
{
  const int increment = 512;
  char* buf = NULL;
  int bufSize = 0,
      bytesCopied = bufSize;
  while( bytesCopied == bufSize )
  {
    bufSize += increment;
    delete[] buf;
    buf = new char[ bufSize ];
    bytesCopied = ::GetModuleFileName( NULL, buf, bufSize );
  }
  string helpFile = buf;
  delete[] buf;

  size_t pos = helpFile.find_last_of( ".\\/" );
  if( pos != string::npos && helpFile[ pos ] != '.' )
    pos = string::npos;
  mHelpFile = helpFile.substr( 0, pos ) + HelpExtension();
  pos = mHelpFile.find_last_of( "\\/" );
  if( pos != string::npos )
    ++pos;
  mHelpFileDir = mHelpFile.substr( 0, pos );
}

void
ExecutableHelp::InitializeContextHelp()
{
  // When the help file contains a redirection, use it to infer the html
  // document path.
  string htmlPath;
  {
    fstream helpFileStream( mHelpFile.c_str() );
    while( helpFileStream && htmlPath.empty() )
    {
      string line;
      getline( helpFileStream, line );
      if( line.find( "\"refresh\"" ) != string::npos )
      {
        const string tag = "url=";
        size_t pos = line.find( tag );
        if( pos != string::npos )
        {
          pos += tag.length();
          line = line.substr( pos, line.find( "\"", pos ) - pos );
          pos = line.find_last_of( "\\/" );
          if( pos != string::npos )
            ++pos;
          htmlPath = line.substr( 0, pos );
        }
      }
    }
  }
  htmlPath = mHelpFileDir + htmlPath;
#ifdef _WIN32
  for( string::iterator i = htmlPath.begin(); i != htmlPath.end(); ++i )
    if( *i == '/' )
      *i = '\\';
  int bufLen = htmlPath.length() + 1;
  char* pathBuffer = new char[ bufLen ];
  if( ::GetFullPathName( htmlPath.c_str(), bufLen, pathBuffer, NULL ) )
    htmlPath = string( pathBuffer );
  delete[] pathBuffer;
  for( string::iterator i = htmlPath.begin(); i != htmlPath.end(); ++i )
    if( *i == '\\' )
      *i = '/';
#endif // _WIN32

  // Build help maps from the TOC file.
  mParamHelp.Clear();
  mParamHelp.SetPath( htmlPath );
  mStateHelp.Clear();
  mStateHelp.SetPath( htmlPath );
  {
    fstream tocFileStream( ( htmlPath + TocFileName() ).c_str() );
    string line;
    while( tocFileStream && ( line.empty() || *line.begin() == '#' ) )
      getline( tocFileStream, line );

    enum
    {
      outsideTOC,
      insideTOC,
      insideIgnoredTOC,
      insideParams,
      insideStates,
      finished,
      error
    } parserState = tocFileStream ? outsideTOC : error;

    string fileName;
    int tocLevel = 0,
        sectionLevel = 0;
    while( ( parserState != error ) && ( parserState != finished ) )
    {
      getline( tocFileStream, line );
      switch( parserState )
      {
        case outsideTOC:
          fileName = line;
          tocLevel = 1;
          sectionLevel = 1;
          if( line.empty() )
            parserState = finished;
          else if( fileName.find( "%253A" ) == string::npos )
            // Valid documentation page names contain a ":" character.
            parserState = insideIgnoredTOC;
          else
            parserState = insideTOC;
          break;

        case insideIgnoredTOC:
          if( line.empty() )
            parserState = outsideTOC;
          break;

        case insideTOC:
        case insideParams:
        case insideStates:
          if( line.empty() )
            parserState = outsideTOC;
          else
          {
            istringstream iss( line );
            int level;
            iss >> level >> ws;
            if( !iss )
              parserState = error;
            else
            {
              switch( parserState )
              {
                case insideTOC:
                  break;
                case insideParams:
                case insideStates:
                  if( level <= sectionLevel )
                    parserState = insideTOC;
                  break;
              }
              string anchor,
                     heading;
              iss >> anchor >> ws;
              getline( iss, heading );
              switch( parserState )
              {
                case insideTOC:
                  if( heading == "Parameters" )
                  {
                    parserState = insideParams;
                    sectionLevel = level;
                  }
                  else if( heading.find( "States" ) == 0 )
                  {
                    parserState = insideStates;
                    sectionLevel = level;
                  }
                  break;

                case insideParams:
                case insideStates:
                {
                  string::const_iterator i = heading.begin();
                  while( i != heading.end() )
                  {
                    string word;
                    while( ::isalpha( *i ) || *i == '_' )
                      word += *i++;
                    if( !word.empty() )
                    {
                      switch( parserState )
                      {
                        case insideParams:
                          mParamHelp.Add( word, fileName + "#" + anchor );
                          break;

                        case insideStates:
                          mStateHelp.Add( word, fileName + "#" + anchor );
                          break;
                      }
                    }
                    while( !::isalpha( *i ) && ( i != heading.end() ) )
                      ++i;
                  }
                } break;
              }
              tocLevel = level;
            }
          }
      }
    }
    if( parserState == error )
    {
      mParamHelp.Clear();
      mStateHelp.Clear();
    }
  }
}

bool
ExecutableHelp::HelpMap::Open( const string& inKey, const string& inContext ) const
{
  bool result = false;
  pair<const_iterator, const_iterator> r = this->equal_range( inKey );
  const_iterator match = r.first;
  if( !inContext.empty() )
    for( const_iterator i = r.first; i != r.second; ++i )
      if( i->second.find( inContext ) != string::npos )
        match = i;
  if( match != this->end() )
  {
#ifdef _WIN32
    string helpFileURL = string( "file:///" ) + mPath + match->second;
    // ShellExecute doesn't treat anchors properly, so we create a
    // temporary file containing a redirect.
    int bufLen = ::GetTempPath( 0, NULL );
    char* pathBuf = new char[ bufLen ];
    ::GetTempPath( bufLen, pathBuf );
    string tempFileName = string( pathBuf ) + "BCI2000Help" + HelpExtension();
    delete[] pathBuf;
    {
      ofstream tempFile( tempFileName.c_str() );
      tempFile << "<meta http-equiv=\"refresh\" content=\"0;url="
               << helpFileURL
               << "\" />"
               << endl;
    }
    HINSTANCE err = ::ShellExecute( NULL, "open", tempFileName.c_str(), NULL, NULL, SW_SHOWNORMAL );
    result = ( reinterpret_cast<int>( err ) > 32 );
#endif // _WIN32
  }
  return result;
}

