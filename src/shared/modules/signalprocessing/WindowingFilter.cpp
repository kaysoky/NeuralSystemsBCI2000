////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: The WindowingFilter provides
//  * Buffering of the signal into time windows that may be larger than
//    SampleBlockSize,
//  * Detrending options (mean or linear),
//  * Window functions typically used with FFT (Hann, Hamming, Blackman).
//  Typically, the Windowing filter provides its output to a spectral
//  estimator (AR, FFT).
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

#include "WindowingFilter.h"
#include "Detrend.h"
#include "BCIException.h"

using namespace std;

WindowingThread::WindowingThread()
: mInputElements( 0 ),
  mDetrend( None ),
  mWindowFunction( Rectangular )
{
}

void
WindowingThread::OnPublish() const
{
 BEGIN_PARAMETER_DEFINITIONS
  "Filtering:Windowing int WindowLength= 0.5s 0.5s % % "
      "// Length of window",
  "Filtering:Windowing int Detrend= 0 0 0 2 "
      "// Detrend data? "
         "0: no, "
         "1: mean, "
         "2: linear "
         "(enumeration)",
  "Filtering:Windowing int WindowFunction= 0 0 0 3 "
      "// Window function "
         "0: Rectangular, "
         "1: Hamming, "
         "2: Hann, "
         "3: Blackman "
         "(enumeration)",
 END_PARAMETER_DEFINITIONS
}


void
WindowingThread::OnPreflight( const SignalProperties& Input,
                                    SignalProperties& Output ) const
{
  Output = Input;
  double windowLength = Parameter( "WindowLength" ).InSampleBlocks();
  int numSamples = static_cast<int>( windowLength * Input.Elements() );
  if( numSamples < 0 )
  {
    bcierr << "WindowLength parameter must be >= 0" << endl;
    numSamples = 0;
  }
  Output.SetElements( numSamples )
        .SetIsStream( false )
        .ElementUnit().SetRawMin( 0 )
                      .SetRawMax( Output.Elements() - 1 );
}


void
WindowingThread::OnInitialize( const SignalProperties& Input,
                               const SignalProperties& Output )
{
  size_t numSamples = Output.Elements();
  mBuffers.clear();
  mBuffers.resize( Channels().size(), DataVector( numSamples ) );
  mInputElements = Input.Elements();

  mDetrend = Parameter( "Detrend" );
  if( mDetrend == None )
    mDetrendBuffer.resize( 0 );
  else
    mDetrendBuffer.resize( numSamples );

  mWindowFunction = Parameter( "WindowFunction" );
  mWindow.resize( numSamples );
  Real phasePerSample = M_PI / numSamples;
  // Window coefficients: Rect Hamming Hann Blackman
  const Real a1[] = {    0,   0.46,  0.5, 0.5, },
             a2[] = {    0,   0,     0,   0.08 };
  for( size_t i = 0; i < numSamples; ++i )
    mWindow[i] = 1 - a1[mWindowFunction] - a2[mWindowFunction]
                   + a1[mWindowFunction] * cos( i * phasePerSample )
                   + a2[mWindowFunction] * cos( i * 2 * phasePerSample );
}

void
WindowingThread::OnProcess( const GenericSignal& Input, GenericSignal& Output )
{
  for( size_t ch = 0; ch < Channels().size(); ++ch )
  {
    // Fill the rightmost part of the buffer with new input:
    size_t i = max<ptrdiff_t>( 0, mBuffers[ch].size() - mInputElements ),
           j = 0;
    while( i < mBuffers[ch].size() )
      mBuffers[ch][i++] = Input( Channels()[ch], j++ );

    DataVector* pDetrendedData = NULL;
    switch( mDetrend )
    {
      case None:
        pDetrendedData = &mBuffers[ch];
        break;

      case Mean:
        Detrend::MeanDetrend( mBuffers[ch], mDetrendBuffer );
        pDetrendedData = &mDetrendBuffer;
        break;

      case Linear:
        Detrend::LinearDetrend( mBuffers[ch], mDetrendBuffer );
        pDetrendedData = &mDetrendBuffer;
        break;

      default:
        throw bciexception( "Unknown detrend option" );
    }

    if( mWindowFunction == Rectangular )
      for( size_t i = 0; i < pDetrendedData->size(); ++i )
        Output( Channels()[ch], i ) = ( *pDetrendedData )[i];
    else
      for( size_t i = 0; i < pDetrendedData->size(); ++i )
        Output( Channels()[ch], i ) = mWindow[i] * ( *pDetrendedData )[i];
  }
}

void
WindowingThread::OnPostProcess()
{
  for( size_t ch = 0; ch < Channels().size(); ++ch )
  {
    // Move existing data towards the beginning of the buffer.
    size_t i = 0,
           j = mInputElements;
    while( j < mBuffers[ch].size() )
      mBuffers[ch][i++] = mBuffers[ch][j++];
  }
}

void
WindowingThread::OnStartRun()
{
  for( size_t ch = 0; ch < mBuffers.size(); ++ch )
    mBuffers[ch] = 0;
}
