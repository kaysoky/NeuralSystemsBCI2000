//---------------------------------------------------------------------------

#ifndef UBitRateH
#define UBitRateH
//---------------------------------------------------------------------------

#include <System.hpp>

class BITRATE
{
private:	// User declarations
        int             hits, misses, choices;
        TDateTime       starttime;
public:		// User declarations
        BITRATE::BITRATE();
        BITRATE::~BITRATE();
        void    Initialize(int choices);
        void    Push(bool hit);
        float   TotalBitsTransferred();
        float   BitsPerTrial();
        float   BitsPerMinute();
};
#endif
