#ifndef RandomNumberADCH
#define RandomNumberADCH

#include "GenericADC.h"
#include "UBCITime.h"

class RandomNumberADC : public GenericADC
{
 public:
               RandomNumberADC();
  virtual      ~RandomNumberADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal*, GenericSignal* );
  virtual void Halt();

 private:
  int    samplerate;
  float  sineminamplitude, sinemaxamplitude,
         noiseminamplitude, noisemaxamplitude,
         sinefrequency,
         DCoffset;
  size_t sinechannel, sinechannelx;
  bool   modulateamplitude;
  int    cur_mousexpos, cur_mouseypos;
  bool   mTrueRandom;
  BCITIME mLasttime;
  int mCount;  
};

#endif // RandomNumberADCH

