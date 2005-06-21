//---------------------------------------------------------------------------

#ifndef TDTBCIH
#define TDTBCIH

#include <Classes.hpp>
#include <string>
#include <vector>
#include <math.h>
#include "RPCOXLib_OCX.h"   
#include "GenericADC.h"
#include "TDTADC.h"
#include "UBCITime.h"

class TDTBCI : public GenericADC
{
public:
	TDTBCI();
    ~TDTBCI();

    //
    void Preflight(const SignalProperties&, SignalProperties&) const;
    void Initialize();
    void Process( const GenericSignal*, GenericSignal*);
    void Halt();

    // Things that should be made public
    static TRPcoX *RPcoX1;
	AnsiString buildTarget(int ch);


private:
	int	mSoftwareCh;
    int	mSampleBlockSize;
    int mSamplingRate;
    int mOffset;
    int nChannels;
    double LPFfreq;
    double HPFfreq;
    double notchBW;
    double TDTsampleRate;
    double TDTgain;
    int blockSize;
    int curindex, stopIndex, indexMult;

    
	float *dataA;// = new float[valuesToRead];
	float *dataB;// = new float[valuesToRead];
	float *dataC;// = new float[valuesToRead];
	float *dataD;// = new float[valuesToRead];
};

//---------------------------------------------------------------------------
#endif
