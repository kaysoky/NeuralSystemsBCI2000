/////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class wrapper for the system's high precision timer.
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
/////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "PrecisionTime.h"
#include "BCIException.h"
#include "OSError.h"

// **************************************************************************
// Function:   Now()
// Purpose:    gets the current time from the system's high-performance timers
// Parameters:
// Returns:    an unsigned sixteen bit value for the current time in ms
//             (-> wrap-around occurs every 65536ms, or about 65 seconds)
// **************************************************************************
#if defined ( _WIN32 )
// **************************************************************************

#include <windows.h>

static LARGE_INTEGER
GetPrecTimeBase()
{
  LARGE_INTEGER result;
  result.QuadPart = 0;
  ::QueryPerformanceFrequency( &result );
  return result;
}

static LARGE_INTEGER sPrecTimeBase = GetPrecTimeBase();

PrecisionTime
PrecisionTime::Now()
{
  if( sPrecTimeBase.QuadPart == 0 )
    throw bciexception( "Your system does not provide a high precision timer" );
  // Get the current time from the Windows precision timer.
  LARGE_INTEGER prectime;
  if( !::QueryPerformanceCounter( &prectime ) )
    throw bciexception( "Could not read high precision timer: " << OSError().Message() );
  return static_cast<PrecisionTime::NumType>( ( prectime.QuadPart * 1000 ) / sPrecTimeBase.QuadPart );
}

// **************************************************************************
#elif defined ( __APPLE__ )
// **************************************************************************

#include <mach/mach_time.h>
PrecisionTime
PrecisionTime::Now()
{
  static int64_t mt0;
  static double  multiplier = 0.0;  
  if(!multiplier) {
  	mach_timebase_info_data_t mtbinfo;
    mach_timebase_info( &mtbinfo );
    multiplier = 1.0e-6 * (double(mtbinfo.numer) / double(mtbinfo.denom));
    mt0 = mach_absolute_time();
  }
  return multiplier * double(mach_absolute_time() - mt0);
}

// **************************************************************************
#else // neither _WIN32 nor __APPLE__
// **************************************************************************

#include <time.h>
PrecisionTime
PrecisionTime::Now()
{
  // Use clock_gettime() on non-Windows systems with librt.
  struct timespec t;
  ::clock_gettime( CLOCK_REALTIME, &t );
  return ( t.tv_sec * 1000 ) + t.tv_nsec / 1000000;
}

// **************************************************************************
#endif // _WIN32, __APPLE__
// **************************************************************************



// **************************************************************************
// Function:   UnsignedDiff
// Purpose:    calculates the difference between two times (i.e., time1-time2)
//             takes roll-over into account (in case time1 < time2)
// Parameters: time1, time2 - two 16 bit integers
// Returns:    time1-time2, if time1-time2 >= 0
//             or time1-time2+65536, if time1-time2 < 0
// **************************************************************************
PrecisionTime::NumType
PrecisionTime::UnsignedDiff( NumType time1, NumType time2 )
{
  const int maxdiffPlusOne = 1 << ( 8 * sizeof( time1 ) );
  return ( ( time1 + maxdiffPlusOne ) - time2 ) % maxdiffPlusOne;
}

// **************************************************************************
// Function:   SignedDiff
// Purpose:    calculates the signed difference between two times,
//             taking roll-over into account.
// Parameters: time1, time2 - two 16 bit integers
// Returns:    signed difference
// **************************************************************************
int
PrecisionTime::SignedDiff( NumType time1, NumType time2 )
{
  const int wraparound = 1 << ( 8 * sizeof( time1 ) );
  int diff = time1;
  diff -= time2;
  if( diff >= wraparound / 2 )
    diff -= wraparound;
  else if( diff < -wraparound / 2 )
    diff += wraparound;
  return diff;
}

