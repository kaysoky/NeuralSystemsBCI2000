////////////////////////////////////////////////////////////////////
// $Id$
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
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
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <typeinfo>
#include <cstdio>

#include "bci_tool.h"
#include "ClassName.h"
#include "ProtocolVersion.h"
#include "Status.h"
#include "Param.h"
#include "ParamList.h"
#include "State.h"
#include "StateList.h"
#include "StateVector.h"
#include "SysCommand.h"
#include "GenericVisualization.h"
#include "MessageHandler.h"
#include "Version.h"

using namespace std;
using namespace bci;

string ToolInfo[] =
{
  "bci_stream2asc",
  BCI2000_VERSION,
  "Convert a binary BCI2000 stream into a human readable form.",
  "Reads a BCI2000 compliant binary stream from standard input, "
    "and writes it to standard output as a sequence of "
    "BCI2000 messages in a human readable representation.",
  "text",
  "",
};

class StreamToAsc : public MessageHandler
{
 public:
  StreamToAsc( ostream& arOut )
  : mrOut( arOut ), mpStatevector( NULL ) {}
  ~StreamToAsc() { delete mpStatevector; }

 private:
  ostream& mrOut;
  StateList mStatelist;
  StateVector* mpStatevector;

  virtual bool HandleProtocolVersion(     istream& );
  virtual bool HandleStatus(              istream& );
  virtual bool HandleParam(               istream& );
  virtual bool HandleState(               istream& );
  virtual bool HandleVisSignalProperties( istream& );
  virtual bool HandleVisSignal(           istream& );
  virtual bool HandleStateVector(         istream& );
  virtual bool HandleSysCommand(          istream& );
};

template<typename T> void Convert( istream&, ostream& );
template<typename T> void Print( ostream&, const T& );

ToolResult
ToolInit()
{
  return noError;
}

ToolResult
ToolMain( OptionSet& arOptions, istream& arIn, ostream& arOut )
{
  if( arOptions.size() > 0 )
    return illegalOption;
  StreamToAsc converter( arOut );
  while( arIn && arIn.peek() != EOF )
    converter.HandleMessage( arIn );
  if( !arIn )
    return illegalInput;
  return noError;
}

template<typename T>
void
Convert( istream& arIn, ostream& arOut )
{
  T t;
  t.ReadBinary( arIn );
  Print( arOut, t );
}

template<typename T>
void
Print( ostream& arOut, const T& arObj )
{
  arOut << ClassName( typeid( T ) ) << " { "
        << setw( 2 ) << arObj << "\n}\n";
}

bool
StreamToAsc::HandleProtocolVersion( istream& arIn )
{
  Convert<ProtocolVersion>( arIn, mrOut );
  return true;
}

bool
StreamToAsc::HandleStatus( istream& arIn )
{
  Convert<Status>( arIn, mrOut );
  return true;
}

bool
StreamToAsc::HandleParam( istream& arIn )
{
  Convert<Param>( arIn, mrOut );
  return true;
}

bool
StreamToAsc::HandleState( istream& arIn )
{
  State s;
  s.ReadBinary( arIn );
  if( arIn )
  {
    mStatelist.Delete( s.Name() );
    mStatelist.Add( s );
    if( mpStatevector != NULL )
    {
      delete mpStatevector;
      mpStatevector = new StateVector( mStatelist );
    }
    Print( mrOut, s );
  }
  return true;
}

bool
StreamToAsc::HandleVisSignalProperties( istream& arIn )
{
  Convert<VisSignalProperties>( arIn, mrOut );
  return true;
}

bool
StreamToAsc::HandleVisSignal( istream& arIn )
{
  Convert<VisSignal>( arIn, mrOut );
  return true;
}

bool
StreamToAsc::HandleStateVector( istream& arIn )
{
  if( mpStatevector == NULL )
    mpStatevector = new StateVector( mStatelist );
  mpStatevector->ReadBinary( arIn );
  Print( mrOut, *mpStatevector );
  return true;
}

bool
StreamToAsc::HandleSysCommand( istream& arIn )
{
  Convert<SysCommand>( arIn, mrOut );
  return true;
}
