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
  short  sineminamplitude, sinemaxamplitude;
  short  noiseminamplitude, noisemaxamplitude;
  float  sinefrequency;
  short  DCoffset;
  size_t sinechannel, sinechannelx;
  bool   modulateamplitude;
  BCITIME mLasttime;
};

#endif // RandomNumberADCH

