//---------------------------------------------------------------------------

#ifndef DTADCH
#define DTADCH
//---------------------------------------------------------------------------
#endif

#include <olmem.h>
#include <olerrors.h>
#include <oldaapi.h>

#include "dtfun.h"

class DTADC : public GenericADC
{
protected:
        int blocksize;
        int channels;
        int SleepTime;
        PARAMLIST       *paramlist;
        STATELIST       *statelist;
        UINT ChanType;
        UINT ListSize;
        DBL dGain;
        int ClkSource;
        DBL dfFreq;
        UINT Bufferpts;
        int StartFlag;
     
        // DTFUN *dtfun;
public:
        DTADC::DTADC(PARAMLIST *, STATELIST *);       // overwrite contructor
        DTADC::~DTADC();
        int     ADInit();
        int     ADConfig();
        int     ADReadDataBlock();
        int     ADShutdown();
};
