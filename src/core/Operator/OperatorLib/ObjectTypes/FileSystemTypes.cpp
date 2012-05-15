////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: CommandInterpreter types for files and directories.
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

#include "FileSystemTypes.h"
#include "CommandInterpreter.h"
#include "FileUtils.h"
#include "ProcessUtils.h"
#include "BCIException.h"

using namespace std;
using namespace Interpreter;
using namespace FileUtils;

// DirectoryType
DirectoryType DirectoryType::sInstance;
const ObjectType::MethodEntry DirectoryType::sMethodTable[] =
{
  METHOD( Extract ), { "Get", &Extract },
  METHOD( Parent ),
  METHOD( Exists ), { "Is", &Exists },
  METHOD( Current ),
  METHOD( Change ),
  METHOD( Make ), { "Create", &Make },
  METHOD( List ),
  END
};

bool
DirectoryType::Extract( CommandInterpreter& inInterpreter )
{
  inInterpreter.Out() << ExtractDirectory( inInterpreter.GetToken() );
  return true;
}

bool
DirectoryType::Parent( CommandInterpreter& inInterpreter )
{
  inInterpreter.Out() << ParentDirectory( inInterpreter.GetToken() );
  return true;
}

bool
DirectoryType::Exists( CommandInterpreter& inInterpreter )
{
  bool result = IsDirectory( inInterpreter.GetToken() );
  inInterpreter.Out() << ( result ? "true" : "false" );
  return true;
}

bool
DirectoryType::Current( CommandInterpreter& inInterpreter )
{
  inInterpreter.Out() << GetCWD();
  return true;
}

bool
DirectoryType::Change( CommandInterpreter& inInterpreter )
{
  string dir = inInterpreter.GetToken();
  if( !ChDir( dir ) )
    inInterpreter.Out() << "Could not change to directory \"" << dir << "\"";
  return true;
}

bool
DirectoryType::Make( CommandInterpreter& inInterpreter )
{
  string dir = inInterpreter.GetToken();
  if( !MkDir( dir ) )
    inInterpreter.Out() << "Could not make directory \"" << dir << "\"";
  return true;
}

bool
DirectoryType::List( CommandInterpreter& inInterpreter )
{
  string args;
#if _WIN32
  args = "/n ";
#else
  args = "-l ";
#endif
  string remainder = inInterpreter.GetOptionalRemainder();
  args += remainder;
  inInterpreter.Out() << ListDirectory( args );
  return true;
}

string
DirectoryType::ListDirectory( const string& inArgs )
{
  string shell, command;
#if _WIN32
  shell = "cmd";
  command = "/c dir ";
#else
  shell = "/bin/sh";
  command = "-c ls ";
#endif
  command += inArgs;
  int exitCode = 0;
  stringstream oss;
  if( !ProcessUtils::ExecuteSynchronously( shell, command, oss, exitCode ) )
    throw bciexception_( "Could not get directory listing" );
#if _WIN32
  string listing;
  char c;
  while( oss.get( c ) )
    if( c != '\r' )
      listing += c;
  return listing;
#else // _WIN32
  return oss.str();
#endif // _WIN32
}

// FileType
FileType FileType::sInstance;
const ObjectType::MethodEntry FileType::sMethodTable[] =
{
  METHOD( Extract ), { "Get", &Extract },
  METHOD( Exists ), { "Is", &Exists },
  METHOD( List ),
  END
};

bool
FileType::Extract( CommandInterpreter& inInterpreter )
{
  string file = inInterpreter.GetToken();
  if( !::stricmp( file.c_str(), "Base" ) )
    inInterpreter.Out() << ExtractBase( inInterpreter.GetToken() );
  else
    inInterpreter.Out() << ExtractFile( file );
  return true;
}

bool
FileType::Exists( CommandInterpreter& inInterpreter )
{
  bool result = IsFile( inInterpreter.GetToken() );
  inInterpreter.Out() << ( result ? "true" : "false" );
  return true;
}

bool
FileType::List( CommandInterpreter& inInterpreter )
{
  return FilesType::ListFiles( inInterpreter, inInterpreter.GetRemainder() );
}

// FilesType
FilesType FilesType::sInstance;
const ObjectType::MethodEntry FilesType::sMethodTable[] =
{
  METHOD( List ),
  END
};

bool
FilesType::List( CommandInterpreter& inInterpreter )
{
  string args = inInterpreter.GetOptionalRemainder();
  return ListFiles( inInterpreter, args );
}

bool
FilesType::ListFiles( CommandInterpreter& inInterpreter, const string& inArgs )
{
  string args;
#if _WIN32
  args = "/b ";
#endif
  args += inArgs;
  string listing = DirectoryType::ListDirectory( args );
  istringstream iss( listing );
  string line;
  while( getline( iss, line ) )
    if( IsFile( line ) )
      inInterpreter.Out() << line << '\n';
  return true;
}

// PathType
PathType PathType::sInstance;
const ObjectType::MethodEntry PathType::sMethodTable[] =
{
  METHOD( Canonicalize ), { "Canonical", &Canonicalize },
  METHOD( Exists ), { "Is", &Exists },
  END
};

bool
PathType::Canonicalize( CommandInterpreter& inInterpreter )
{
  inInterpreter.Out() << CanonicalPath( inInterpreter.GetToken() );
  return true;
}

bool
PathType::Exists( CommandInterpreter& inInterpreter )
{
  string path = inInterpreter.GetToken();
  bool result = IsFile( path ) || IsDirectory( path );
  inInterpreter.Out() << ( result ? "true" : "false" );
  return true;
}
