////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: Adam Wilson
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#ifndef ARCHANNEL_H
#define ARCHANNEL_H

#include <limits>
#include <vector>
#if QT_CORE_LIB
# include <QtCore>
# include <QRunnable>
#endif // QT_CORE_LIB

#include "Detrend.h"
#include "MeasurementUnits.h"
#include "MEMPredictor.h"
#include "TransferSpectrum.h"
#include "GenericSignal.h"
#include "BCIError.h"


struct ARparms{
	double fs;
	int detrend;
	double numWindows;
	int length;
	int numBins;
	double **lookupTable;
	int SBS;
	int outputType;
	int modelOrder;
	int evalsPerBin;
	double firstBinCenter;
	double lastBinCenter;
	double binWidth;
	};

class ARChannel
{
	typedef double               Real;
	typedef std::complex<double> Complex;
	typedef std::valarray<Real>  DataVector;
public:
	ARChannel(int channel, ARparms parms);
	~ARChannel();
	void UpdateBuffer(const GenericSignal *in);
	void UpdateBuffer(double *in);
	void Calculate();
	int GetOutputElements(){return mOutputElements;}
	int getChannel(){return mChannel;}

    std::valarray<double> mSpectrum;
    Polynomial<Complex>::Vector mCoeff;

private:
	unsigned short mChannel;
	unsigned short mSamples;
	int mOutputElements;
	ARparms mParms;

	DataVector mBuf, mInputSignal;
	Ratpoly<Complex> mTransferFunction;
	MEMPredictor<double>     mMEMPredictor;
	TransferSpectrum<double> mTransferSpectrum;

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
};

#ifndef QT_CORE_LIB
class ARthread
#else // !QT_CORE_LIB
class ARthread : public QRunnable
#endif // !QT_CORE_LIB
{
 public:
  	void Init(int startCh, int endCh, int blockSize, ARparms parms);
  	void UpdatePower(GenericSignal *out);
	void UpdatePower(double *out);
  	void UpdateCoeffs(GenericSignal *out);
	void UpdateCoeffs(double *out);
	int GetOutputElements(){return mOutputElements;}
  	ARthread();
  	~ARthread();
 private:
  	int mStartCh, mEndCh, mBlockSize;
  	std::vector<ARChannel *> mAR;
  	void Clear();
  	int mOutputElements;
 public:
  	void UpdateBuffer(const GenericSignal *in);
	void UpdateBuffer(double *in);
  	void Process();
 protected:
  	void run(){Process();}
};

#endif // ARCHANNEL_H
