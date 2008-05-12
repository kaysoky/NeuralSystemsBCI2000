/////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class wrapper for the system's high precision timer.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "PrecisionTime.h"

#ifdef _WIN32
# include <windows.h>
#else
# include <sys/time.h>
#endif

// **************************************************************************
// Function:   Now()
// Purpose:    gets the current time from the system's high-performance timers
// Parameters:
// Returns:    an unsigned sixteen bit value for the current time in ms
//             (-> wrap-around occurs every 65536ms, or about 6.5 seconds)
// **************************************************************************
PrecisionTime
PrecisionTime::Now()
{
#ifdef _WIN32

  // Get the current time from the Windows precision timer.
  LARGE_INTEGER prectime, prectimebase;
  ::QueryPerformanceCounter( &prectime );
  ::QueryPerformanceFrequency( &prectimebase );
  return ( prectime.QuadPart * 1000 ) / prectimebase.QuadPart;

#else // _WIN32

  // Use clock_gettime() on non-Windows systems.
  struct timespec t;
  ::clock_gettime( CLOCK_REALTIME, &t );
  return ( t.tv_sec * 1000 ) + t.tv_nsec / 1000000;

#endif // _WIN32
}

// **************************************************************************
// Function:   TimeDiff
// Purpose:    calculates the difference between two times (i.e., time2-time1)
//             takes roll-over into account (in case time2 < time1)
// Parameters: time1, time2 - two 16 bit integers
// Returns:    time2-time1, if time2-time1 >= 0
//             or time2-time1+65536, if time2-time1 < 0
// **************************************************************************
PrecisionTime::NumType
PrecisionTime::TimeDiff( NumType time1, NumType time2 )
{
  const int maxdiffPlusOne = 1 << ( 8 * sizeof( time1 ) );
  return ( ( time2 + maxdiffPlusOne ) - time1 ) % maxdiffPlusOne;
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
  int wraparound = 1 << ( 8 * sizeof( time1 ) ),
      diff1 = time1 - time2,
      diff2 = ( time1 + wraparound ) - time2;
  return ( diff2 >= wraparound / 2 ) ? diff1 : diff2;
}

