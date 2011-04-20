////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: BufferedADC is a base class for signal source filters that
//   provides buffering for the data packets read from the ADC, to avoid data
//   loss when data isn't read timely enough.
//   See the accompagnying header file for more information.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#include "BufferedADC.h"

BufferedADC::BufferedADC()
: mStartupLock( false )
{
}

BufferedADC::~BufferedADC()
{
  BufferedADC::Halt();
}

void
BufferedADC::Preflight( const SignalProperties&,
                              SignalProperties& output ) const
{
  Preflight( output );
}

void
BufferedADC::Initialize( const SignalProperties&,
                         const SignalProperties& output )
{
  mBuffer.SetSignalProperties( output );
  mQueue.clear();
  Initialize( output );

  mStartupLock = false;
  OSThread::Start();
  while( !mStartupLock ) // Wait until the acquisition thread has acquired the queue mutex.
    OSThread::Sleep( 0 );
}

// The Process() function is called from the main thread in regular intervals.
void
BufferedADC::Process( const GenericSignal&,
                            GenericSignal& output )
{
  mQueueMutex.Acquire();
  if( mQueue.empty() )
  { // This should never happen.
    bcierr << "Empty data queue in Process()" << endl;
  }
  else
  {
    output = mQueue.front();
    mQueue.pop();
  }
  mQueueMutex.Release();
}

void
BufferedADC::Halt()
{
  OSThread::Terminate();
  while( !OSThread::IsTerminated() )
    OSThread::Sleep( 0 );
}

// The Execute() function runs in its own (producer) thread, concurrently with repeated calls to
// Process() from the main thread (which acts as a consumer).
int
BufferedADC::Execute()
{
  mQueueMutex.Acquire(); // Acquire the mutex to make sure Process blocks until data is available.
  mStartupLock = true;   // Now that the queue mutex is acquired, unblock the main thread.
  StartDataAcquisition();
  AcquireData( mBuffer );
  while( !OSThread::Terminating() )
  {
    mQueue.push( mBuffer );
    if( !OSThread::Terminating() )
    {
      mQueueMutex.Release(); // Allow Process() to read from queue while AcquireData() is waiting for new data.
      AcquireData( mBuffer );
      mQueueMutex.Acquire();
    }
  }
  StopDataAcquisition();
  mQueueMutex.Release(); // Process() is no longer called once we reach this point.
}

