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
        void    Initialize(int choices);
        void    Push(bool hit);
        float   TotalBitsTransferred() const;
        float   BitsPerTrial() const;
        float   BitsPerMinute() const;
};
#endif
