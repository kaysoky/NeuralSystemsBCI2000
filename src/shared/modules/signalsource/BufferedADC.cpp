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
#include "BCIStream.h"

using namespace std;

BufferedADC::BufferedADC()
: mReadCursor( 0 ),
  mWriteCursor( 0 ),
  mAcquiring( false )
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
BufferedADC::AutoConfig( const SignalProperties& )
{
  this->OnAutoConfig();
}

void
BufferedADC::Preflight( const SignalProperties&,
                              SignalProperties& Output ) const
{
  if( Parameter( "SourceBufferSize" ).InSampleBlocks() < 2 )
    bcierr << "The SourceBufferSize parameter must be greater or"
           << " equal 2 sample blocks."
           << endl;
  State( "SourceTime" );
  mAcquisitionProperties = Output;
  this->OnPreflight( mAcquisitionProperties );
  Output = mAcquisitionProperties;
  int numStateChannels = 0;
  for( int ch = 0; ch < Output.Channels(); ++ch )
  {
    bool isStateChannel = ( *Output.ChannelLabels()[ch].c_str() == StateMark );
    if( numStateChannels && !isStateChannel )
      bcierr_ << "State channels must be located at the end of the channel list";
    else if( isStateChannel )
      ++numStateChannels;
  }
  Output.SetChannels( Output.Channels() - numStateChannels );
}

void
BufferedADC::Initialize( const SignalProperties&,
                         const SignalProperties& Output )
{
  mBuffer.clear();
  mTimeStamps.clear();
  size_t SourceBufferSize = static_cast<size_t>( Parameter( "SourceBufferSize" ).InSampleBlocks() );
  mBuffer.resize( SourceBufferSize, GenericSignal( mAcquisitionProperties ) );
  mTimeStamps.resize( SourceBufferSize );
  mReadCursor = 0;
  mWriteCursor = 0;
  this->OnInitialize( mAcquisitionProperties );
  if( bcierr.Empty() )
    StartAcquisition();
}

// The Process() function is called from the main thread in regular intervals.
void
BufferedADC::Process( const GenericSignal&,
                            GenericSignal& Output )
{
  this->OnProcess();
  
  bool abort = false,
       waitForData = false;
  {
    OSMutex::Lock lock( mMutex );
    abort = !IsAcquiring();
    waitForData = ( mReadCursor == mWriteCursor ) && !abort;
    mAcquisitionDone.Reset();
  }
  if( waitForData )
    mAcquisitionDone.Wait();

  const GenericSignal& acquired = mBuffer[mReadCursor];
  if( acquired.Channels() == Output.Channels() )
    Output = acquired;
  else
  {
    const LabelIndex& labels = mAcquisitionProperties.ChannelLabels();
    for( int el = 0; el < Output.Elements(); ++el )
    {
      for( int ch = 0; ch < Output.Channels(); ++ch )
        Output( ch, el ) = acquired( ch, el );
      for( int ch = Output.Channels(); ch < acquired.Channels(); ++ch )
        State( labels[ch].c_str() + 1 )( el ) = static_cast<State::ValueType>( acquired( ch, el ) );
    }
  }
  
  State( "SourceTime" ) = mTimeStamps[mReadCursor];
  {
    OSMutex::Lock lock( mMutex );
    ++mReadCursor %= mBuffer.size();
    abort = !IsAcquiring();
  }

  if( abort && State( "Running" ) )
    State( "Running" ) = 0;
  else if( abort )
    bcierr_ << ( mError.empty() ? "Acquisition Error" : mError );
}

void
BufferedADC::Halt()
{
  StopAcquisition();
  OSThread::TerminateWait();
  OnHalt();
}

void
BufferedADC::Error( const string& inError )
{
  mError = inError.empty() ? "Acquisition Error" : inError;
}

// When UseAcquisitionThread() returns true, the Execute() function runs in its own writer thread,
// concurrently with repeated calls to Process() from the main thread, which is the reader thread.
int
BufferedADC::OnExecute()
{
  this->OnStartAcquisition();
  while( !OSThread::IsTerminating() )
  {
    GenericSignal* p = GetBuffer();
    this->DoAcquire( *p );
    ReleaseBuffer( p );
  }
  this->OnStopAcquisition();
  return 0;
}

GenericSignal*
BufferedADC::GetBuffer()
{
  return &mBuffer[mWriteCursor];
}

void
BufferedADC::ReleaseBuffer( GenericSignal* inpBuffer )
{
  bciassert( inpBuffer == GetBuffer() );
  mTimeStamps[mWriteCursor] = PrecisionTime::Now();

  OSMutex::Lock lock( mMutex );
  if( !mError.empty() )
    StopAcquisition();
  
  ++mWriteCursor %= mBuffer.size();
  if( mWriteCursor == mReadCursor )
    bciwarn << "Data acquisition buffer overflow";
  mAcquisitionDone.Set();
}

void
BufferedADC::StartAcquisition()
{
  mError.clear();
  if( UseAcquisitionThread() )
    OSThread::Start();
  else
  {
    this->OnStartAcquisition();
    mAcquiring = mError.empty();
  }
}

void
BufferedADC::StopAcquisition()
{
  if( UseAcquisitionThread() )
    OSThread::Terminate();
  else
  {
    this->OnStopAcquisition();
    mAcquiring = false;
  }
}

bool
BufferedADC::IsAcquiring() const
{
  if( UseAcquisitionThread() )
    return !OSThread::IsTerminated();
  return mAcquiring;
}
