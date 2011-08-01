////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: BufferedADC is a base class for signal source filters that
//   provides buffering for data packets read from the ADC, to avoid data
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

using namespace std;

BufferedADC::BufferedADC()
: mReadCursor( 0 ),
  mWriteCursor( 0 )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Source:Buffering int SourceBufferSize= 2s "
      "2s 1 % // size of data acquisition ring buffer (in blocks or seconds)",
  END_PARAMETER_DEFINITIONS
}

BufferedADC::~BufferedADC()
{
  BufferedADC::Halt();
}

void
BufferedADC::Preflight( const SignalProperties&,
                              SignalProperties& output ) const
{
  if( Parameter( "SourceBufferSize" ).InSampleBlocks() < 1 )
    bcierr << "The SourceBufferSize parameter must be greater or"
           << " equal 1 in terms of sample blocks."
           << endl;
  this->OnPreflight( output );
}

void
BufferedADC::Initialize( const SignalProperties&,
                         const SignalProperties& output )
{
  GenericSignal outputSignal( output );
  mBuffer.clear();
  mBuffer.resize( ( int )Parameter( "SourceBufferSize" ).InSampleBlocks(), outputSignal );
  mReadCursor = 0;
  mWriteCursor = 0;
  this->OnInitialize( output );
  OSThread::Start();
}

// The Process() function is called from the main thread in regular intervals.
void
BufferedADC::Process( const GenericSignal&,
                            GenericSignal& output )
{
  mMutex.Acquire();
  bool waitForData = ( mReadCursor == mWriteCursor );
  mAcquisitionDone.Reset();
  mMutex.Release();
  if( waitForData )
    mAcquisitionDone.Wait();

  output = mBuffer[mReadCursor];
  ++mReadCursor %= mBuffer.size();
}

void
BufferedADC::Halt()
{
  OSEvent terminationEvent;
  OSThread::Terminate( &terminationEvent );
  terminationEvent.Wait();
  OnHalt();
}

// The Execute() function runs in its own (producer) thread, concurrently with repeated calls to
// Process() from the main thread (which acts as a consumer).
int
BufferedADC::Execute()
{
  this->OnStartAcquisition();
  while( !OSThread::IsTerminating() )
  {
    this->DoAcquire( mBuffer[mWriteCursor] );
    mMutex.Acquire();
    ++mWriteCursor %= mBuffer.size();
    mAcquisitionDone.Set();
    mMutex.Release();
  }
  this->OnStopAcquisition();
  return 0;
}

