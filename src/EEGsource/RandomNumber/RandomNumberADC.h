//---------------------------------------------------------------------------

#ifndef RandomNumberADCH
#define RandomNumberADCH
//---------------------------------------------------------------------------
#endif

class RandomNumberADC : public GenericADC
{
public:
                RandomNumberADC(PARAMLIST *, STATELIST *);
    virtual     ~RandomNumberADC();
    virtual int ADInit();
    virtual int ADReadDataBlock();
    virtual int ADShutdown();

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
};
