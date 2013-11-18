//////////////////////////////////////////////////////////////////////
// $Id$
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
#ifndef THREADED_SOCKBUF_H
#define THREADED_SOCKBUF_H

#include "SockStream.h"
#include "Lockable.h"
#include "Synchronized.h"
#include "OSThread.h"
#include "OSEvent.h"
#include "OSMutex.h"

namespace bci
{

class ThreadedSockbuf : public sockbuf
{
 public:
  ThreadedSockbuf();
  ~ThreadedSockbuf();
  ThreadedSockbuf& AsyncReceive( bool = true );
  ThreadedSockbuf& AsyncSend( bool = true );
  ThreadedSockbuf& TimeoutMs( int ms )
    { set_timeout( ms ); return *this; }
  int TimeoutMs() const
    { return get_timeout(); }
  ThreadedSockbuf& ResponseTimeMs( int ms );
  int ResponseTimeMs() const
    { return mResponseTimeMs; }
  const OSEvent& NotifyReceived()
  { return mReceiver.Notify; }

 protected:
  int write_to_socket( int );
  int read_from_socket( int );

  void socket_sync( bool ) { MemoryFence(); }
  void pbuf_sync( bool ) { MemoryFence(); }
  void gbuf_sync( bool ) { MemoryFence(); }
  void pbuf_lock( bool b ) { b ? mPutLock.Lock() : mPutLock.Unlock(); }
  void gbuf_lock( bool b ) { b ? mGetLock.Lock() : mGetLock.Unlock(); }

 private:
  int WriteToSocket( int );
  int ReadFromSocket( int );

  struct Receiver : OSThread
  {
    Receiver( ThreadedSockbuf* p ) : Parent( *p ) {}
    int OnExecute();
    OSEvent Notify;
    ThreadedSockbuf& Parent;
    Synchronized<int> Count;
  } mReceiver;
  friend struct Receiver;
  struct Sender : OSThread
  {
    Sender( ThreadedSockbuf* p ) : Parent( *p ) {}
    int OnExecute();
    OSEvent Notify;
    ThreadedSockbuf& Parent;
  } mSender;
  friend struct Sender;

  Lockable<OSMutex> mPutLock, mGetLock;
  int mResponseTimeMs;
};

} // namespace

using bci::ThreadedSockbuf;

#endif // THREADED_SOCKBUF_H
