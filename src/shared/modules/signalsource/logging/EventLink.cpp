/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A component that establishes a UDP connection with the
//   operator module, and asynchronously receives events over this
//   connection.
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
/////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "EventLink.h"
#include "PrecisionTime.h"
#include "BCIEvent.h"
#include "BCIError.h"
#include "CoreModule.h"

using namespace std;

static const int cTimeoutMs = 500;

Extension( EventLink );

EventLink::EventLink()
: mInput( mInputSocket )
{
}

EventLink::~EventLink()
{
  Halt();
}

void
EventLink::Publish()
{
  BEGIN_PARAMETER_DEFINITIONS
    "System:Additional%20Connections int EventLink= 1 1 0 1 // Send events from Operator to Source module (boolean)",
  END_PARAMETER_DEFINITIONS

  bool enabled = ( Parameter( "EventLink" ) != 0 );
  if( enabled )
  {
    istringstream iss( THISOPPORT );
    unsigned short port;
    iss >> port;
    mInputSocket.open( "localhost", port + 1 );
    if( !mInputSocket.is_open() )
      bcierr << "Could not open UDP port " << port + 1 << " for listening" << endl;
    else
    {
      string operatorIP = OptionalParameter( "OperatorIP", "localhost" );
      sending_udpsocket clientSocket;
      clientSocket.open( operatorIP.c_str(), port );
      clientSocket.write( "\n", 1 );
      clientSocket.close();

      if( !mInputSocket.wait_for_read( cTimeoutMs ) )
      {
        bciout << "Operator does not seem to support EventLink: "
               << "no additional event definitions were received within " << 1e-3 * cTimeoutMs << "s of query. "
               << "Using core-module event definitions only."
               << endl;
      }
      else
      {
        string line;
        while( std::getline( mInput, line ) && !line.empty() )
        {
          BEGIN_EVENT_DEFINITIONS
            line.c_str(),
          END_EVENT_DEFINITIONS
        }
      }
    }
  }
}

void
EventLink::Preflight() const
{
}

void
EventLink::Initialize()
{
}

void
EventLink::StartRun()
{
  OSThread::Start();
}

void
EventLink::StopRun()
{
  OSThread::TerminateWait();
}

void
EventLink::Halt()
{
  StopRun();
}

int
EventLink::OnExecute()
{
  const int cReactionTimeMs = 100;
  while( !OSThread::IsTerminating() )
  {
    if( mInputSocket.wait_for_read( cReactionTimeMs ) )
    {
      string message;
      while( mInput && mInput.rdbuf()->in_avail() )
      {
        if( mInput.peek() == '\n' )
        {
          mInput.ignore();
          if( !message.empty() )
            bcievent << message;
          message = "";
        }
        else
        {
          message += mInput.get();
        }
      }
    }
  }
  return 0;
}
