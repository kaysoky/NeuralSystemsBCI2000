//---------------------------------------------------------------------------

#ifndef UBCItimeH
#define UBCItimeH
//---------------------------------------------------------------------------

class BCITIME
{
private:	// User declarations
public:		// User declarations
        BCITIME::BCITIME();
        BCITIME::~BCITIME();
        unsigned short GetBCItime_ms();
        unsigned short TimeDiff(unsigned short time1, unsigned short time2);
};

#endif

