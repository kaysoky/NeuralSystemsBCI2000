////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de,
//          Adam Wilson
// Description: A class that encapsulates a single thread that computes
//   spectra for a subset of input channels, and writes them into an output
//   signal. Multiple threads may operate on the same input and output signal
//   concurrently, provided that their channel sets do not overlap.
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
#ifndef AR_THREAD_H
#define AR_THREAD_H

#include "ReusableThread.h"
#include "Runnable.h"
#include "MEMPredictor.h"
#include "TransferSpectrum.h"
#include "GenericSignal.h"
#include <vector>

class ARThread : public ReusableThread, private Runnable
{
 public:
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
  ARThread();

  ARThread& SetModelOrder( int order )
    { mMEMPredictor.SetModelOrder( order ); return *this; }
  ARThread& SetWindowLength( size_t length )
    { mWindowLength = length; return Modified(); }
  ARThread& SetDetrend( int detrend )
    { mDetrend = detrend; return *this; }
  ARThread& SetOutputType( int outputType )
    { mOutputType = outputType; return *this; }
  ARThread& SetFirstBinCenter( double firstBinCenter )
    { mTransferSpectrum.SetFirstBinCenter( firstBinCenter ); return *this; }
  ARThread& SetBinWidth( double binWidth )
    { mTransferSpectrum.SetBinWidth( binWidth ); return *this; }
  ARThread& SetNumBins( int numBins )
    { mTransferSpectrum.SetNumBins( numBins ); mSpectrum.resize( numBins ); return *this; }
  ARThread& SetEvaluationsPerBin( int evaluationsPerBin )
    { mTransferSpectrum.SetEvaluationsPerBin( evaluationsPerBin ); return *this; }

  ARThread& AddChannel( int ch )
    { mChannels.push_back( ch ); return Modified(); }

  ARThread& Start( const GenericSignal&, GenericSignal& );

 private:
  ARThread& Modified();
  void OnRun();

 private:
  const GenericSignal* mpInput;
  GenericSignal* mpOutput;
  std::vector<int> mChannels;

  typedef double Real;
  typedef std::valarray<Real> DataVector;

  size_t mWindowLength;
  int mDetrend,
      mOutputType;
  std::vector<DataVector> mInputBuffers;
  DataVector mDetrendBuffer,
             mSpectrum;
  Ratpoly<Real> mTransferFunction;
  MEMPredictor<Real> mMEMPredictor;
  TransferSpectrum<Real> mTransferSpectrum;
};

#endif // AR_THREAD_H
