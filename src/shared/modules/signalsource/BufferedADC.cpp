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
: mpBuffers( 0 ),
  mSourceBufferSize( 0 ),
  mReadCursor( 0 ),
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
  delete[] mpBuffers;
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
  State( "Running" );
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
  delete[] mpBuffers;
  mSourceBufferSize = static_cast<size_t>( Parameter( "SourceBufferSize" ).InSampleBlocks() );
  mpBuffers = new AcquisitionBuffer[mSourceBufferSize];
  for( size_t i = 0; i < mSourceBufferSize; ++i )
  {
    mpBuffers[i].Signal.SetProperties( mAcquisitionProperties );
    mpBuffers[i].TimeStamp = -1;
  }
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
  AcquisitionBuffer& buffer = mpBuffers[mReadCursor];
  ++mReadCursor %= mSourceBufferSize;

  Lock lock( buffer );
  if( !IsAcquiring() )
  {
    bcierr_ << ( mError.empty() ? "Acquisition Error" : mError );
    if( State( "Running" ) )
      State( "Running" ) = 0;
    return;
  }

  if( buffer.Signal.Channels() == Output.Channels() )
    Output = buffer.Signal;
  else
  {
    const LabelIndex& labels = mAcquisitionProperties.ChannelLabels();
    for( int el = 0; el < Output.Elements(); ++el )
    {
      for( int ch = 0; ch < Output.Channels(); ++ch )
        Output( ch, el ) = buffer.Signal( ch, el );
      for( int ch = Output.Channels(); ch < buffer.Signal.Channels(); ++ch )
        State( labels[ch].c_str() + 1 )( el ) = static_cast<State::ValueType>( buffer.Signal( ch, el ) );
    }
  }
  State( "SourceTime" ) = buffer.TimeStamp;
}

void
BufferedADC::Halt()
{
  StopAcquisition();
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
  StartAcquisitionInternal();
  GenericSignal* p = NextWriteBuffer();
  while( !OSThread::IsTerminating() )
  {
    this->DoAcquire( *p );
    p = NextWriteBuffer( p );
  }
  ReleaseBuffer( p );
  StopAcquisitionInternal();
  return 0;
}

GenericSignal*
BufferedADC::NextWriteBuffer( GenericSignal* inpBuffer )
{
  if( !mError.empty() )
  {
    StopAcquisition();
    ReleaseBuffer( inpBuffer );
    return &mpBuffers[0].Signal;
  }
  AcquisitionBuffer& buffer = mpBuffers[mWriteCursor];
  ++mWriteCursor %= mSourceBufferSize;
  buffer.Lock();
  ReleaseBuffer( inpBuffer );
#if BCIDEBUG
  buffer.Signal = GenericSignal( mAcquisitionProperties, GenericSignal::NaN );
  buffer.TimeStamp = -1;
#endif
  return &buffer.Signal;
}

void
BufferedADC::ReleaseBuffer( const GenericSignal* inpBuffer )
{
  for( size_t i = 0; i < mSourceBufferSize; ++i )
    if( &mpBuffers[i].Signal == inpBuffer )
    {
      mpBuffers[i].TimeStamp = PrecisionTime::Now();
      mpBuffers[i].Unlock();
      return;
    }
  if( !inpBuffer )
    mStarted.Set();
  else
    Error( "Invalid buffer pointer" );
}

void
BufferedADC::StartAcquisition()
{
  mStarted.Reset();
  if( UseAcquisitionThread() )
    OSThread::Start();
  else
    StartAcquisitionInternal();
  mAcquiring = mError.empty();
  if( !mStarted.Wait( 2000 ) )
    bcierr << "Could not start acquisition";
}

void
BufferedADC::StartAcquisitionInternal()
{
  mWriteCursor = 0;
  mReadCursor = 0;
  mError.clear();
  this->OnStartAcquisition();
}

void
BufferedADC::StopAcquisition()
{
  if( UseAcquisitionThread() )
    OSThread::Terminate();
  else if( mAcquiring )
    StopAcquisitionInternal();
  mAcquiring = false;
  if( !OSThread::InOwnThread() )
    OSThread::TerminateWait();
}

void
BufferedADC::StopAcquisitionInternal()
{
  this->OnStopAcquisition();
}

bool
BufferedADC::IsAcquiring() const
{
  return mAcquiring;
}
