////////////////////////////////////////////////////////////////////////////////
//
// File: RDAClientADC.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Jan 3, 2003
//
// Description: A source class that interfaces to the BrainAmp RDA socket
//              interface.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef RDACLIENTADCH
#define RDACLIENTADCH

#include <string>
#ifdef BCI_2000_STRICT
# include "UBCITime.h"
# include "GenericADC.h"
# define GenericSource GenericADC
# define MaxElements MaxElements()
# define Channels    Channels()
#else // BCI_2000_STRICT
# include "SPGenerics.h"
#endif // BCI_2000_STRICT
#include "RDAQueue.h"

class RDAClientADC : public GenericSource
{
public:
    enum
    {
#ifdef BCI_2000_STRICT
      noError = 1,
      parameterError = 0,
      netinitError = 0,
      connectionError = 0,
      memoryError = 0,
#else // BCI_2000_STRICT
      noError = 0,
      parameterError,
      netinitError,
      connectionError,
      memoryError,
#endif // BCI_2000_STRICT
    };

                  RDAClientADC( PARAMLIST*, STATELIST* );
    virtual       ~RDAClientADC();
    virtual int   ADInit();
    virtual int   ADReadDataBlock( GenericIntSignal* s );
    virtual int   ADReadDataBlock()
                  { return ADReadDataBlock( &sourceSignal ); }
    virtual int   ADShutdown();

    // To avoid new/delete confusion, we don't use any data members from the
    // base class, and we don't use a member instance instead of a pointer
    // to the signal.
    virtual const GenericIntSignal* Signal() { return &sourceSignal; }
    virtual int   GetSampleRate() const      { return samplingRate; }

  protected:
        PARAMLIST *paramlist;
        STATELIST *statelist;

  private:
        std::string       hostName;
        size_t            softwareCh;
        RDAQueue          inputQueue;
        GenericIntSignal  sourceSignal;
        float             samplingRate;

#ifndef BCI_2000_STRICT
  public:
    virtual int   Initialize( PARAMLIST* p, STATEVECTOR* s )
                  { paramlist = p; statevector = s; return ADInit(); }
            char* GetBoardName()
                  { return "RDAClient"; }
  private:
    STATEVECTOR *statevector;
#endif // BCI_2000_STRICT
};

#endif // RDACLIENTADCH


