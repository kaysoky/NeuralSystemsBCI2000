#ifndef DTADCH
#define DTADCH

#include <windows.h>
#include <olmem.h>
#include <olerrors.h>
#include <oldaapi.h>

#include "dtfun.h"
#include "GenericADC.h"

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
  int ADConfig();

 private:
  int   samplerate;
  int   blocksize;
  int   channels;
  int   SleepTime;
  UINT  ChanType;
  UINT  ListSize;
  DBL   dGain;
  int   ClkSource;
  DBL   dfFreq;
  UINT  Bufferpts;
  int   StartFlag;
};

#endif // DTADCH
