//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <Math.hpp>

#include "UBitRate.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

// **************************************************************************
// Function:   BITRATE
// Purpose:    the constructor for the BITRATE object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
BITRATE::BITRATE()
{
 hits=misses=0;
}


// **************************************************************************
// Function:   ~BITRATE
// Purpose:    the destructor for the BITRATE object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
BITRATE::~BITRATE()
{
}


// **************************************************************************
// Function:   Initialize
// Purpose:    initializes start time, hits, misses, and number of choices
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void BITRATE::Initialize(int new_choices)
{
 hits=0;
 misses=0;
 choices=new_choices;
 starttime=TDateTime::CurrentDateTime();
}


// **************************************************************************
// Function:   Push
// Purpose:    stores the result of one trial
// Parameters: hit ... true  = hit
//                     false = miss
// Returns:    N/A
// **************************************************************************
void BITRATE::Push(bool hit)
{
 if (hit)
    hits++;
 else
    misses++;
}


// **************************************************************************
// Function:   TotalBitsTransferred
// Purpose:    Calculates the total number of transferred bits
// Parameters: N/A
// Returns:    information transfer rate in bits
// **************************************************************************
float BITRATE::TotalBitsTransferred()
{
float res;

 res=BitsPerTrial();
 res*=(float)(hits+misses);

 return(res);
}


// **************************************************************************
// Function:   BitsPerTrial
// Purpose:    Calculates average transferred bits/trial
// Parameters: N/A
// Returns:    information transfer rate in bits/trial
// **************************************************************************
float BITRATE::BitsPerTrial()
{
float p, res;

 // calculate average accuracy
 if (hits+misses > 0)
    p=(float)hits/(float)(hits+misses);
 else
    p=0;

 // calculate bits/trial
 if (choices > 1)
    {
    if (p >= 1/(float)choices)
       {
       if ( (1-p) > 0 )
          res=(float)(Log2((float)choices)+p*Log2(p)+(1-p)*Log2((1-p)/((float)choices-1)));
       else
          res= (float)(Log2((float)choices));
       }
    else
       res=0;
    }
 else
    res=0;

 return(res);
}


// **************************************************************************
// Function:   BitsPerMinute
// Purpose:    Calculates average transferred bits/minute from Initialize() to now
// Parameters: N/A
// Returns:    information transfer rate in bits/minute
// **************************************************************************
float BITRATE::BitsPerMinute()
{
float minutes;

 minutes=(float)((double)TDateTime::CurrentDateTime()-(double)starttime)*1440;

 if (minutes > 0.0000000001)
    return(TotalBitsTransferred()/minutes);
 else
    return(0);
}



