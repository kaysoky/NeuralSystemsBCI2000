/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef NIADCH
#define NIADCH
//---------------------------------------------------------------------------

#include "NIDAQmx.h"

#include "GenericADC.h"

#define NIDAQ_ERR_GENERICERR    0
#define NIDAQ_ERR_NOERR         1

#define NIDAQ_MAX_CHANNELS      128
#define NIDAQ_MAX_BUFFERS       50      // how many buffers do we want ?

#define NIDAQ_iDBmodeON         1
#define NIDAQ_iDBmodeOFF        0

#define NIDAQ_MODE_CLEARALLMESSAGES   0
#define NIDAQ_MODE_ADDMESSAGE         1
#define NIDAQ_MODE_REMOVEMESSAGE      2

class NIADC : public GenericADC
{
protected:
i16     iStatus;
i16     iRetVal;
i16     iDevice;
i16     iChan;
i16     iGain;
f64     dSampRate;
f64     dScanRate;
f64     dGainAdjust;
f64     dOffset;
i16     iUnits;
i16     iSampTB;
i16     iScanTB;
u16     uSampInt;
u16     uScanInt;
i16     iHalfReady;
i16     iDAQstopped;
u32     ulRetrieved;
i16     iIgnoreWarning;
i16     iYieldON;
int     i, numChans;
u32     SampleBlockSize, ulCount;
i16     iNumMUXBrds;
i16     chanVector[NIDAQ_MAX_CHANNELS], gainVector[NIDAQ_MAX_CHANNELS];
i16     *piBuffer, *piHalfBuffer[NIDAQ_MAX_BUFFERS];
// f64     pdVoltBuffer[NIDAQ_MAX_BUFFER];
i32     lTimeout;
u32     ulPtsTfr;

        int     samplingRate;
        int     blocksize;
        int     channels;
private:
        int     Start();
        int     Stop();
        int     ADConfig();
        void    GetData();
        int     cur_buffers;                    // how many data buffers do we currently have ?
        TCriticalSection *data_critsec;         // critical section for data FIFO
        static  NIADC* cur_adc;
        static void CallBack( int, int, WPARAM, LPARAM );
public:
        NIADC::NIADC();       // overwrite constructor
        NIADC::~NIADC();
        virtual void     Preflight(const SignalProperties&, SignalProperties&) const;
        virtual void     Initialize(const SignalProperties&, const SignalProperties&);
        virtual void     Process(const GenericSignal&, GenericSignal&);
        virtual void     Halt();

};

#endif

