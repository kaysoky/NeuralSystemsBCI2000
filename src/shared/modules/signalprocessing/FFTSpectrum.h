////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de,
//          Adam Wilson
// Description: The ARSpectrum filter fits a Maximum Entropy AR model to
//   its input data. Its output can be configured to be
//   - raw AR coefficients,
//   - the model's amplitude spectrum,
//   - the model's intensity spectrum.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef FFT_SPECTRUM_H
#define FFT_SPECTRUM_H

#include <valarray>

#include "ThreadedFilter.h"
#include "SpectrumThread.h"
#include "FFTLibWrap.h"

class FFTThread : public SpectrumThread
{
 public:
  FFTThread() : mInputResampling( 1 ), mNormalizationFactor( 1 ), mSpectrumOversampling( 1 ), mOutputType( 0 ) {}

 protected:
  void OnPublish() const;
  void OnPreflight( const SignalProperties&, SignalProperties& ) const;
  void OnInitialize( const SignalProperties&, const SignalProperties& );
  void OnProcess( const GenericSignal&, GenericSignal& );

 private:
  typedef FFTLibWrapper::Complex Complex;
  typedef FFTLibWrapper::Real Real;
  typedef std::valarray<Complex> ComplexVector;

  ComplexVector mShiftCarrier,
                mSpectrum;
  ComplexFFT mFFT;
  Real mInputResampling,
       mNormalizationFactor;
  int mSpectrumOversampling;
  int mOutputType;
};

struct FFTSpectrum : public ThreadedFilter<FFTThread> {};

#endif // FFT_SPECTRUM_H
