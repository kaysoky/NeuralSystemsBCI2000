////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: An abstract base class that implements an IIR filter.
//   Subclasses will provide individual implementations for the
//   DesignFilter() member, which is supposed to translate parameter settings
//   into a rational transfer function (complex poles and zeros).
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

#include "IIRFilterBase.h"
#include "ThreadUtils.h"
#include "BCIStream.h"

using namespace std;

IIRFilterBase::IIRFilterBase()
: mpThreads( 0 ),
  mAdditionalThreads( 0 )
{
}

IIRFilterBase::~IIRFilterBase()
{
  delete[] mpThreads;
}

void
IIRFilterBase::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  OptionalParameter( "NumberOfThreads" );

  Real          preflightGain;
  ComplexVector preflightZeros,
                preflightPoles;
  Output = Input;
  DesignFilter( Output, preflightGain, preflightZeros, preflightPoles );
  if( preflightZeros.size() != preflightPoles.size() )
    bcierr << "The numbers of zeros and poles must agree" << endl;
}

void
IIRFilterBase::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  if( Input.Elements() != Output.Elements() )
    bciout << "Downsampling from " << Input.SamplingRate() << "Hz to " << Output.SamplingRate() << "Hz";

  Real gain;
  ComplexVector zeros, poles;
  DesignFilter( SignalProperties( Input ), gain, zeros, poles );

  delete[] mpThreads;
  mpThreads = 0;
  mAdditionalThreads = 0;

  int NumberOfThreads = 1;
  if( !zeros.empty() || gain != 1 )
  {
    NumberOfThreads = OptionalParameter( "NumberOfThreads", ThreadUtils::NumberOfProcessors() );
    if( NumberOfThreads < 2 )
      NumberOfThreads = 1;
    int channelsPerThread = min( 1, Input.Channels() / NumberOfThreads );
    if( NumberOfThreads * channelsPerThread > Input.Channels() )
      NumberOfThreads = Input.Channels() / channelsPerThread;

    mAdditionalThreads = NumberOfThreads - 1;
    mpThreads = new FilterThread[mAdditionalThreads];
    for( int i = 0; i < mAdditionalThreads; ++i )
    {
      Filter& filter = mpThreads[i].filter;
      filter.channels.begin = i * channelsPerThread,
      filter.channels.end = ( i + 1 ) * channelsPerThread;
      filter.SetGain( gain )
            .SetZeros( zeros )
            .SetPoles( poles )
            .Initialize( channelsPerThread );
    }
  }
  mFilter.channels.begin = mAdditionalThreads ? mpThreads[mAdditionalThreads-1].filter.channels.end : 0;
  mFilter.channels.end = Input.Channels();
  mFilter.SetGain( gain )
         .SetZeros( zeros )
         .SetPoles( poles )
         .Initialize( mFilter.channels.end - mFilter.channels.begin );
}

void
IIRFilterBase::StartRun()
{
  for( int i = 0; i < mAdditionalThreads; ++i )
    mpThreads[i].filter.Initialize();
  mFilter.Initialize();
}

void
IIRFilterBase::Process( const GenericSignal& Input, GenericSignal& Output )
{
  for( int i = 0; i < mAdditionalThreads; ++i )
  {
    mpThreads[i].filter.channels.pInput = &Input;
    mpThreads[i].filter.channels.pOutput = &Output;
    mpThreads[i].Run( mpThreads[i].filter );
  }

  mFilter.channels.pInput = &Input;
  mFilter.channels.pOutput = &Output;
  mFilter.Run();

  for( int i = 0; i < mAdditionalThreads; ++i )
    mpThreads[i].Wait();
}

void
IIRFilterBase::ChannelSet::operator=( const ChannelSet& s )
{
  bciassert( &s == this );
  bciassert( begin == 0 && end == pInput->Channels() );
  bciassert( pInput->Elements() == pOutput->Elements() );
  *pOutput = *pInput;
}
