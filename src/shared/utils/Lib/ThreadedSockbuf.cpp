//////////////////////////////////////////////////////////////////////
// $Id: EventQueue.h 4629 2013-10-30 19:38:11Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A sockbuf descendant with background buffering from
//   a separate thread.
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
#include "ThreadedSockbuf.h"

using namespace std;

ThreadedSockbuf::ThreadedSockbuf()
: mReceiver( this ),
  mSender( this ),
  mResponseTimeMs( 1000 )
{
  mReceiver.Notify.Reset();
  mSender.Notify.Reset();
}

ThreadedSockbuf::~ThreadedSockbuf()
{
  mReceiver.TerminateWait();
  mSender.TerminateWait();
}

ThreadedSockbuf&
ThreadedSockbuf::AsyncReceive( bool b )
{
  if( b == mReceiver.IsTerminated() )
  {
    if( b )
      mReceiver.Start();
    else
      mReceiver.TerminateWait();
  }
  return *this;
}

ThreadedSockbuf&
ThreadedSockbuf::AsyncSend( bool b )
{
  if( b == mSender.IsTerminated() )
  {
    if( b )
      mSender.Start();
    else
      mSender.TerminateWait();
  }
  return *this;
}

ThreadedSockbuf&
ThreadedSockbuf::ResponseTimeMs( int ms )
{
  mResponseTimeMs = ms;
  return *this;
}

int
ThreadedSockbuf::Receiver::OnExecute()
{
  Notify.Reset();
  Count = 0;
  while( !IsTerminating() )
  {
    int result = Parent.ReadFromSocket( Parent.ResponseTimeMs() );
    if( result > 0 )
    {
      Count += result;
      Notify.Set();
    }
    if( result < 0 )
      Terminate();
  }
  Notify.Reset();
  return 0;
}

int
ThreadedSockbuf::Sender::OnExecute()
{
  Notify.Reset();
  while( !IsTerminating() )
  {
    if( Notify.Wait( Parent.ResponseTimeMs() ) )
    {
      Notify.Reset();
      while( Parent.WriteToSocket( Parent.TimeoutMs() ) > 0 )
        ;
    }
  }
  Notify.Reset();
  return 0;
}

int
ThreadedSockbuf::ReadFromSocket( int t )
{
  return sockbuf::read_from_socket( t );
}

int
ThreadedSockbuf::WriteToSocket( int t )
{
  return sockbuf::write_to_socket( t );
}

int
ThreadedSockbuf::write_to_socket( int inTimeout )
{
  if( mSender.IsTerminated() )
    return WriteToSocket( inTimeout );
 
  mSender.Notify.Set();
  return is_open() ? 1 : -1;
}

int
ThreadedSockbuf::read_from_socket( int inTimeout )
{
  if( mReceiver.IsTerminated() )
    return ReadFromSocket( inTimeout );

  mReceiver.Notify.Wait( inTimeout );
  mReceiver.Notify.Reset();
  return mReceiver.Count.Atomic().Exchange( 0 );
}

