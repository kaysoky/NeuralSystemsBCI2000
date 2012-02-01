////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that displays an executable's associated
//   help file.
//   The help file is a html file that has the same name as the executable,
//   except that it bears a .html extension.
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

#include "ExecutableHelp.h"
#if _WIN32
# include <windows.h>
#else
# include <QtGui>
#endif
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
  const char* errorText = 
      "The help file could not be found.\n\n"
      "Help files should be located in the\n"
      "executable's directory.\n\n"
      "Help files bear the executable's name,\n"
      "and a .html extension.",
  * errorCaption =
      "Error Opening Help File";
#if _WIN32
  HINSTANCE err = ::ShellExecuteA( NULL, "open", mHelpFile.c_str(), NULL, mHelpFileDir.c_str(), SW_SHOWNORMAL );
  result = ( reinterpret_cast<int>( err ) > 32 );
  if( !result )
  {
    ::MessageBoxA(
      NULL,
      errorText,
      errorCaption,
      MB_OK | MB_ICONERROR
    );
  }
#else // _WIN32
  string url = string( "file://" ) + mHelpFile;
  result = QDesktopServices::openUrl( QUrl( url.c_str(), QUrl::TolerantMode ) );
  if( !result )
  {
    QMessageBox::critical( 
      NULL, 
      errorCaption,
      errorText
      );
  }
#endif // _WIN32
  return result;
}

void
ExecutableHelp::Initialize()
{
  string helpFile;
#if _WIN32
  const int increment = 512;
  char* buf = NULL;
  int bufSize = 0,
      bytesCopied = bufSize;
  while( bytesCopied == bufSize )
  {
    bufSize += increment;
    delete[] buf;
    buf = new char[ bufSize ];
    bytesCopied = ::GetModuleFileNameA( NULL, buf, bufSize );
  }
  helpFile = buf;
  delete[] buf;
#else // _WIN32
  helpFile = QApplication::applicationFilePath().toLocal8Bit().constData();
#endif // _WIN32

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
#if _WIN32
  for( string::iterator i = htmlPath.begin(); i != htmlPath.end(); ++i )
    if( *i == '/' )
      *i = '\\';
  DWORD bufLen = static_cast<DWORD>( htmlPath.length() + 1 );
  char* pathBuffer = new char[bufLen];
  if( ::GetFullPathNameA( htmlPath.c_str(), bufLen, pathBuffer, NULL ) )
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
                default:
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
                    while( ( i != heading.end() ) && ( ::isalnum( *i ) || *i == '_' ) )
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
                          
                        default:
                          break;
                      }
                    }
                    while( ( i != heading.end() ) && !::isalnum( *i ) )
                      ++i;
                  }
                } break;
                
                default:
                  break;
              }
              tocLevel = level;
            }
          }
          
        default:
          break;
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
    string helpFileURL = string( "file:///" ) + mPath + match->second;
#if _WIN32
    // ShellExecute doesn't treat anchors properly, so we create a
    // temporary file containing a redirect.
    int bufLen = ::GetTempPath( 0, NULL );
    char* pathBuf = new char[ bufLen ];
    ::GetTempPathA( bufLen, pathBuf );
    string tempFileName = string( pathBuf ) + "BCI2000Help" + HelpExtension();
    delete[] pathBuf;
    {
      ofstream tempFile( tempFileName.c_str() );
      tempFile << "<meta http-equiv=\"refresh\" content=\"0;url="
               << helpFileURL
               << "\" />"
               << endl;
    }
    HINSTANCE err = ::ShellExecuteA( NULL, "open", tempFileName.c_str(), NULL, NULL, SW_SHOWNORMAL );
    result = ( reinterpret_cast<int>( err ) > 32 );
#else // _WIN32
    result = QDesktopServices::openUrl( QUrl( helpFileURL.c_str(), QUrl::TolerantMode ) );
#endif // _WIN32
  }
  return result;
}

