/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef TDTBCIH
#define TDTBCIH

#include "GenericADC.h"
#include "TDTADC.h"
#include "RPCOXLib_OCX.h"
#include "ZBUSXLib_OCX.h"
#include <stdio.h>

class TDTBCI : public GenericADC
{
public:
	TDTBCI();
    ~TDTBCI();

    //
    void Preflight(const SignalProperties&, SignalProperties&) const;
    void Initialize(const SignalProperties&, const SignalProperties&);
    void Process( const GenericSignal&, GenericSignal&);
    void Halt();

    void reset();

private:
    class TRPcoX *RPcoX1;
    class TRPcoX *RPcoX2;
    class TZBUSx *ZBus;

    void dropSamples(GenericSignal& outputSignal);
    
    int	mSourceCh;
    int	mSampleBlockSize;
    int mSamplingRate;
    int mOffset;
    int nChannels;
    int nProcessors;
    double LPFfreq;
    double HPFfreq;
    double notchBW;
    double TDTsampleRate;
    double TDTgain;
    int TDTbufSize;
    int blockSize;
    short connectType;
    int curindex, stopIndex, indexMult;
    int devAddr[2];
    bool use2RX5;
    bool useECG;
    float ECGgain;
    int ECGchannel;
    int ECGoffset;
    int ECGstopIndex;
    int mUseFrontPanel;
    int mEEGchannels;
    int mFrontPanelChannels;
    float mDigitalGain;
    float mFrontPanelGain;
    FILE * logFile;
    
	float *dataA;// = new float[valuesToRead];
	float *dataB;// = new float[valuesToRead];
	float *dataC;// = new float[valuesToRead];
	float *dataD;// = new float[valuesToRead];
    float *dataE;
    /*float *dataA2;
    float *dataB2;
    float *dataC2;
    float *dataD2;*/
    //float *ECGdata;
};

//---------------------------------------------------------------------------
#endif
