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
  if( Parameter( "SourceBufferSize" ).InSampleBlocks() < 2 )
    bcierr << "The SourceBufferSize parameter must be greater or"
           << " equal 2 sample blocks."
           << endl;
  State( "SourceTime" );
  this->OnPreflight( output );
}

void
BufferedADC::Initialize( const SignalProperties&,
                         const SignalProperties& output )
{
  mBuffer.clear();
  mTimeStamps.clear();
  size_t SourceBufferSize = static_cast<size_t>( Parameter( "SourceBufferSize" ).InSampleBlocks() );
  mBuffer.resize( SourceBufferSize, GenericSignal( output ) );
  mTimeStamps.resize( SourceBufferSize );
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
  this->OnProcess();

  bool waitForData = false;
  {
    OSMutex::Lock lock( mMutex );
    waitForData = ( mReadCursor == mWriteCursor );
    mAcquisitionDone.Reset();
  }
  if( waitForData )
    mAcquisitionDone.Wait();

  output = mBuffer[mReadCursor];
  State( "SourceTime" ) = mTimeStamps[mReadCursor];
  {
    OSMutex::Lock lock( mMutex );
    ++mReadCursor %= mBuffer.size();
  }
}

void
BufferedADC::Halt()
{
  OSThread::TerminateWait();
  OnHalt();
}

// The Execute() function runs in its own writer thread, concurrently with repeated calls to
// Process() from the main thread, which is the reader thread.
int
BufferedADC::Execute()
{
  this->OnStartAcquisition();
  while( !OSThread::IsTerminating() )
  {
    this->DoAcquire( mBuffer[mWriteCursor] );
    mTimeStamps[mWriteCursor] = PrecisionTime::Now();
    {
      OSMutex::Lock lock( mMutex );
      ++mWriteCursor %= mBuffer.size();
      if( mWriteCursor == mReadCursor )
        bciout << "Data acquisition buffer overflow" << endl;
      mAcquisitionDone.Set();
    }
  }
  this->OnStopAcquisition();
  return 0;
}

