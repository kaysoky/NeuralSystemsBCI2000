//---------------------------------------------------------------------------

#ifndef RandomNumberADCH
#define RandomNumberADCH
//---------------------------------------------------------------------------
#endif

class RandomNumberADC : public GenericADC
{
protected:
        PARAMLIST       *paramlist;
        STATELIST       *statelist;
        short           sineminamplitude, sinemaxamplitude;
        short           noiseminamplitude, noisemaxamplitude;
        float           sinefrequency;
        short           DCoffset;
        short           sinechannel;
        bool            modulateamplitude;
        //char            multstate[256];
public:
        RandomNumberADC::RandomNumberADC(PARAMLIST *, STATELIST *);
        RandomNumberADC::~RandomNumberADC();
        int     ADInit();
        int     ADReadDataBlock();
        int     ADShutdown();
};
