/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "UBCItime.h"

#ifdef _WIN32
# include <windows.h>
#else
# include <sys/time.h>
#endif

// **************************************************************************
// Function:   GetBCItime_ms
// Purpose:    gets the current time from Windows' high-performance timers
// Parameters:
// Returns:    an unsigned sixteen bit value for the current time in ms
//             (-> wrap-around occurs every 65536ms, or about 6.5 seconds)
// **************************************************************************
unsigned short BCITIME::GetBCItime_ms()
{
#ifdef _WIN32
LARGE_INTEGER   prectime, prectimebase;

 // calculate the current time
 QueryPerformanceCounter(&prectime);
 QueryPerformanceFrequency(&prectimebase);
 return (unsigned short)((double)prectime.QuadPart/(double)prectimebase.QuadPart*1000);
#else
  struct timespec t;
  ::clock_gettime( CLOCK_REALTIME, &t );
  return ( t.tv_sec * 1000 ) + t.tv_nsec / 1000000;
#endif
}


// **************************************************************************
// Function:   TimeDiff
// Purpose:    calculates the difference between two times (i.e., time2-time1)
//             takes roll-over into account (in case time2 < time1)
// Parameters: time1, time2 - two 16 bit integers
// Returns:    time2-time1, if time2-time1 >= 0
//             or time2-time1+65536, if time2-time1 < 0
// **************************************************************************
unsigned short BCITIME::TimeDiff(unsigned short time1, unsigned short time2)
{
  const int maxdiffPlusOne = 1 << ( 8 * sizeof( time1 ) );
  return ( ( time2 + maxdiffPlusOne ) - time1 ) % maxdiffPlusOne;
}





