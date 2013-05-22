////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Implementation of bcierr and bciout message handlers for a
//              GUI application.
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

#include "BCIStream.h"

#if _WIN32
# ifdef __CONSOLE__
#  define USE_CERR 1
# endif // __CONSOLE__
# include <Windows.h>
#elif USE_QT
# include <QMessageBox>
#else
# define USE_CERR 1
#endif

using namespace std;

enum
{
  plain,
  warning, 
  error
};

static void Display( const string& inText, int inType = plain )
{
  string text = inText;
  if( text.find_last_of( ".!?" ) != text.length() - 1 )
    text += '.';
  string kind;
  switch( inType )
  {
    case warning:
      kind = "Warning";
      break;
    case error:
      kind = "Error";
      break;
  }
  string title = "BCI2000" + kind.empty() ? " " : "" + kind;

#if USE_CERR
  ostream& os = ( inType == error ? cerr : cout );
  os << kind.empty() ? "" : ": " << text << endl;
#elif _WIN32
  ::MessageBoxA( NULL, text.c_str(), title.c_str(), MB_OK | MB_ICONHAND | MB_SYSTEMMODAL | MB_SETFOREGROUND );
#else
  QMessageBox::critical( NULL, title.c_str(), text.c_str() );
#endif
}

void
BCIStream::PlainMessage( const string& message )
{
  Display( message );
}

void
BCIStream::DebugMessage( const string& message )
{
  Display( message );
}

void
BCIStream::Warning( const string& message )
{
  Display( message, warning );
}

void
BCIStream::ConfigurationError( const string& message )
{
  Display( message, error );
}

void
BCIStream::RuntimeError( const string& message )
{
  Display( message, error );
}

void
BCIStream::LogicError( const string& message )
{
  Display( message, error );
}

