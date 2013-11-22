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
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <vector>
#include <typeinfo>

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
#include "MessageChannel.h"
#include "Version.h"

using namespace std;
using namespace bci;

string ToolInfo[] =
{
  "bci_decimate",
  PROJECT_VERSION,
  "Decimate data in a binary BCI2000 stream.",
  "Reads a BCI2000 compliant binary stream from standard input, "
    "and writes it to standard output, retaining only every "
    "Nth statevector and signal message.",
  "binary",
  "-d<N>,    --decimate=<N>        Decimation factor, defaults to 1",
  ""
};

class Decimate : public MessageChannel
{
 public:
  Decimate( int factor, istream& arIn, ostream& arOut )
  : MessageChannel( arIn, arOut ), mFactor( factor ), mCount( 0 ), mpStatevector( NULL ) {}
  ~Decimate() { delete mpStatevector; }

 private:
  int mFactor,
      mCount;
  StateList mStatelist;
  StateVector* mpStatevector;

  virtual bool OnProtocolVersion(     istream& );
  virtual bool OnStatus(              istream& );
  virtual bool OnParam(               istream& );
  virtual bool OnState(               istream& );
  virtual bool OnVisSignalProperties( istream& );
  virtual bool OnVisSignal(           istream& );
  virtual bool OnStateVector(         istream& );
  virtual bool OnSysCommand(          istream& );
  template<typename T> void Forward( istream& );
  template<typename T> void Absorb( istream& );
};


ToolResult
ToolInit()
{
  return noError;
}

ToolResult
ToolMain( OptionSet& arOptions, istream& arIn, ostream& arOut )
{
  int decimation = ::atoi( arOptions.getopt( "-d|-D|--decimation", "1" ).c_str() );
  Decimate decimator( decimation, arIn, arOut );
  while( arIn && arIn.peek() != EOF )
    decimator.HandleMessage();
  if( !arIn )
    return illegalInput;
  return noError;
}

template<typename T>
void
Decimate::Forward( istream& arIn )
{
  T t;
  t.ReadBinary( arIn );
  Send( t );
}

template<typename T>
void
Decimate::Absorb( istream& arIn )
{
  T t;
  t.ReadBinary( arIn );
}

bool
Decimate::OnProtocolVersion( istream& arIn )
{
  Forward<ProtocolVersion>( arIn );
  return true;
}

bool
Decimate::OnStatus( istream& arIn )
{
  Forward<Status>( arIn );
  return true;
}

bool
Decimate::OnParam( istream& arIn )
{
  Forward<Param>( arIn );
  return true;
}

bool
Decimate::OnState( istream& arIn )
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
    Send( s );
  }
  return true;
}

bool
Decimate::OnVisSignalProperties( istream& arIn )
{
  Forward<VisSignalProperties>( arIn );
  return true;
}

bool
Decimate::OnVisSignal( istream& arIn )
{
  if( mCount % mFactor == 0 )
    Forward<VisSignal>( arIn );
  else
    Absorb<VisSignal>( arIn );
  return true;
}

bool
Decimate::OnStateVector( istream& arIn )
{
  if( mpStatevector == NULL )
    mpStatevector = new StateVector( mStatelist );
  mpStatevector->ReadBinary( arIn );
  // state vectors are sent first, so we increase
  // the count here rather than in the signal handler
  if( ( ++mCount %= mFactor ) == 0 )
    Send( *mpStatevector );
  return true;
}

bool
Decimate::OnSysCommand( istream& arIn )
{
  Forward<SysCommand>( arIn );
  return true;
}
