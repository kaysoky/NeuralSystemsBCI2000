#ifndef DTADCH
#define DTADCH

#include <windows.h>
#include <olmem.h>
#include <olerrors.h>
#include <oldaapi.h>

#include "dtfun.h"
#include "GenericADC.h"
#include "UGenericSignal.h"


class DTADC : public GenericADC
{
 public:
               DTADC();
  virtual      ~DTADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal*, GenericSignal* );
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
