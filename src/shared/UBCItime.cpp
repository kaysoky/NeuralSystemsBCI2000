//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "UBCItime.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


// **************************************************************************
// Function:   GetBCItime_ms
// Purpose:    gets the current time from Windows' high-performance timers
// Parameters:
// Returns:    an unsigned sixteen bit value for the current time in ms
//             (-> wrap-around occurs every 65536ms, or about 6.5 seconds)
// **************************************************************************
unsigned short BCITIME::GetBCItime_ms()
{
LARGE_INTEGER   prectime, prectimebase;

 // calculate the current time
 QueryPerformanceCounter(&prectime);
 QueryPerformanceFrequency(&prectimebase);
 return (unsigned short)((double)prectime.QuadPart/(double)prectimebase.QuadPart*1000);
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
#if 1
 if ((int)time2-(int)time1 >= 0)
    return(time2-time1);
 else
    return((unsigned short)((int)time2-(int)time1+(int)65536));
#else // jm's version
  const int maxdiffPlusOne = 1 << ( 8 * sizeof( time1 ) );
  return ( ( time2 + maxdiffPlusOne ) - time1 ) % maxdiffPlusOne;
#endif
}

