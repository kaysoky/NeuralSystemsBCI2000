////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: An ADC class for testing purposes.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SIGNAL_GENERATOR_ADC_H
#define SIGNAL_GENERATOR_ADC_H

#include "GenericADC.h"
#include "PrecisionTime.h"
#include "Expression.h"
#include "RandomGenerator.h"

class SignalGeneratorADC : public GenericADC
{
 public:
               SignalGeneratorADC();
  virtual      ~SignalGeneratorADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void StartRun();
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual void Halt();

 private:
  // Configuration
  float  mSamplingRate,
         mSineFrequency,
         mSineAmplitude,
         mNoiseAmplitude,
         mDCOffset;
  Expression mOffsetMultiplier;
  int    mSineChannelX,
         mSineChannelY,
         mSineChannelZ;
  bool   mModulateAmplitude;
  // Internal State
  float  mAmplitudeX,
         mAmplitudeY,
         mAmplitudeZ,
         mSinePhase;
  PrecisionTime   mLasttime;
  RandomGenerator mRandomGenerator;
};

#endif // SIGNAL_GENERATOR_ADC_H

