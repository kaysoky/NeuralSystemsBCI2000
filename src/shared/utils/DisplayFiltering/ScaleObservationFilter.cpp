////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that observes the scale of the data it sees.
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

#include "ScaleObservationFilter.h"
#include <limits>
#include <cmath>
#include <cstring>

using namespace std;

namespace {
  template<typename T>
  bool IsNan( T t )
  {
    static const T nan = numeric_limits<T>::quiet_NaN();
    return ::memcmp( &t, &nan, sizeof( T ) ) == 0;
  }
} // namespace


ScaleObservationFilter::ScaleObservationFilter()
: mTimeConstant( 0 ),
  mDecayFactor( 1 )
{
  Reset();
}

ScaleObservationFilter&
ScaleObservationFilter::TimeConstant( double inTimeConstant )
{
  if( mTimeConstant != inTimeConstant )
  {
    mTimeConstant = inTimeConstant;
    if( mTimeConstant < numeric_limits<double>::epsilon() )
      mDecayFactor = 0;
    else
      mDecayFactor = ::exp( - 1 / inTimeConstant );
    Reset();
  }
  return *this;
}

void
ScaleObservationFilter::Process( const GenericSignal& Input )
{
  double rate = Input.Properties().SamplingRate(),
        duration = 1;
  if( rate > 0 )
    duration = Input.Elements() / rate;
  double decayFactor = ::pow( mDecayFactor, duration ),
         oldMean = Mean();
  if( IsNan( oldMean ) )
  {
    Reset();
    oldMean = 0;
  }
  double oldMax = ( mMax - oldMean ) * decayFactor,
         oldMin = ( mMin - oldMean ) * decayFactor;
  mCount *= decayFactor;
  mCount += Input.Channels() * Input.Elements();
  mSum *= decayFactor;
  for( int ch = 0; ch < Input.Channels(); ++ch )
    for( int el = 0; el < Input.Elements(); ++el )
      mSum += Input( ch, el );
  double mean = Mean();
  mMax = oldMax + mean;
  mMin = oldMin + mean;
  for( int ch = 0; ch < Input.Channels(); ++ch )
    for( int el = 0; el < Input.Elements(); ++el )
      if( Input( ch, el ) > mMax )
        mMax = Input( ch, el );
      else if( Input( ch, el ) < mMin )
        mMin = Input( ch, el );
}

void
ScaleObservationFilter::Reset()
{
  mCount = numeric_limits<double>::epsilon();
  mSum = 0;
  mMax = 0;
  mMin = 0;
}
