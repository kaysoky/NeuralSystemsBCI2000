/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef DTADCH
#define DTADCH

#include <windows.h>
#include <olmem.h>
#include <olerrors.h>
#include <oldaapi.h>

#include "dtfun.h"
#include "GenericADC.h"
#include "GenericSignal.h"


class DTADC : public GenericADC
{
 public:
               DTADC();
  virtual      ~DTADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual void Halt();

 private:
  int   samplerate;
  int   blocksize;
  int   channels;
  int   SleepTime;
  // DBL   dGain;
  // int   ClkSource;
  // DBL   dfFreq;
  // UINT  Bufferpts;
  int   StartFlag;
  bool  Board2Active;
  int   channelsb1, channelsb2;
};

#endif // DTADCH
