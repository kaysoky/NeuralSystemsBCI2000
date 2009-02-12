////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The ARFilter fits a Maximum Entropy AR model to a window
//   of past input data.
//   Its output can be configured to be
//   - raw AR coefficients,
//   - the model's amplitude spectrum,
//   - the model's intensity spectrum.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef AR_FILTER_H
#define AR_FILTER_H

#include "GenericFilter.h"
#include "MEMPredictor.h"
#include "TransferSpectrum.h"
#include <vector>

class ARFilter : public GenericFilter
{
 private:
  typedef float               Real;
  typedef std::complex<float> Complex;
  typedef std::valarray<Real> DataVector;

  enum OutputTypes
  {
    SpectralAmplitude = 0,
    SpectralPower = 1,
    ARCoefficients = 2,
  };

  enum DetrendOptions
  {
    none = 0,
    mean = 1,
    linear = 2,
  };

 public:
          ARFilter();
  virtual ~ARFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );

 private:
  int                     mOutputType,
                          mDetrend;
  std::vector<DataVector> mBuffer;
  MEMPredictor<float>     mMEMPredictor;
  TransferSpectrum<float> mTransferSpectrum;
};

#endif // AR_FILTER_H


