#ifndef BufferReadADCH
#define BufferReadADCH

#include "GenericADC.h"

class BufferReadADC : public GenericADC
{
 public:
               BufferReadADC();
  virtual      ~BufferReadADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal*, GenericSignal* );
  virtual void Halt();

//  short  sineminamplitude, sinemaxamplitude;
//  short  noiseminamplitude, noisemaxamplitude;
//  float  sinefrequency;
//  short  DCoffset;
//  size_t sinechannel;
//  bool   modulateamplitude;

 private:
//  int ADConfig();

 private:
  int   samplerate;
  int   blocksize;
  int   channels;
  int   SleepTime;
/*
  UINT  ChanType;
  UINT  ListSize;
  DBL   dGain;
  int   ClkSource;
  DBL   dfFreq;
  UINT  Bufferpts;
*/
  int   StartFlag;

};

#endif // BufferReadADCH

