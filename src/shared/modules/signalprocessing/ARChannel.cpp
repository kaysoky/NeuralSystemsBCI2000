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
#include "PCHIncludes.h"
#pragma hdrstop

#include "ARChannel.h"

using namespace std;

const float eps = std::numeric_limits<float>::epsilon();
ARChannel::ARChannel(int channel, ARparms parms)
{
	mChannel = channel;
	mParms = parms;
	mOutputElements = 0;
	mSamples = static_cast<int>( mParms.SBS*mParms.numWindows );
	mBuf.resize(mSamples, 0.0);
	mInputSignal.resize(mParms.SBS, 0.0);
	mMEMPredictor.SetModelOrder(mParms.modelOrder);
	int numBins, numCoefs;
	numCoefs = mParms.modelOrder;
	numBins = static_cast<int>( ::floor( ( mParms.lastBinCenter - mParms.firstBinCenter + eps ) / mParms.binWidth + 1 ) );
	switch (mParms.outputType)
	{
	case SpectralAmplitude:
	case SpectralPower:

		mTransferSpectrum
			.SetFirstBinCenter( mParms.firstBinCenter )
			.SetBinWidth( mParms.binWidth )
			.SetNumBins( numBins )
			.SetEvaluationsPerBin( mParms.evalsPerBin );
		mOutputElements = numBins;
		break;
	case ARCoefficients:

		mOutputElements = numCoefs;
		break;
	}

	mCoeff.resize(numCoefs, 0.0);
	mSpectrum.resize(numBins, 0.0);
}

ARChannel::~ARChannel()
{
}

void ARChannel::UpdateBuffer(const GenericSignal *in)
{
	for (int s = 0; s < in->Elements(); s++)
		mInputSignal[s] = (*in)(mChannel,s);
}

void ARChannel::UpdateBuffer(double *in)
{
	int chOffset = mChannel*mParms.SBS;
	for (int s = 0; s < mParms.SBS; s++)
		mInputSignal[s] = in[s+chOffset];
}

void ARChannel::Calculate()
{
	size_t i = 0, j = mParms.SBS;

	while( j < mBuf.size() )
		mBuf[ i++ ] = mBuf[ j++ ];
	// Fill the rightmost part of the buffer with new input:
	j = mParms.SBS - ( mBuf.size() - i );
	while( i < mBuf.size() )
		mBuf[ i++ ] = mInputSignal[j++];

	DataVector* inputData;
	switch( mParms.detrend )
	{
	case none:
		inputData = &mBuf;
		break;

	case mean:
		inputData = new DataVector;
		Detrend::MeanDetrend( mBuf, *inputData );
		break;

	case linear:
		inputData = new DataVector;
		Detrend::LinearDetrend( mBuf, *inputData );
		break;

	default:
		//bcierr << "Unknown detrend option" << endl;
		break;
	}

	mMEMPredictor.TransferFunction( *inputData, mTransferFunction );

	switch( mParms.outputType )
	{
	case SpectralAmplitude:
		{
			mTransferSpectrum.Evaluate( mTransferFunction, mSpectrum );
			for( size_t bin = 0; bin < mSpectrum.size(); ++bin )
				mSpectrum[bin] = sqrt(mSpectrum[bin]);
		} break;

	case SpectralPower:
		{
			mTransferSpectrum.Evaluate( mTransferFunction, mSpectrum );
		} break;

	case ARCoefficients:
		{
			const Polynomial<Complex>::Vector& coeff = mTransferFunction.Denominator().Coefficients();
			for( size_t i = 1; i < coeff.size(); ++i )
				mCoeff[i-1] = coeff[i].real();
		} break;

	default:
		//bcierr << "Unknown output type" << endl;
		break;
	}
  if( mParms.detrend != 0 )
	  delete inputData;
}

//------------------------------------------------------------
ARthread::ARthread()
{
#if QT_CORE_LIB
	setAutoDelete(false);
#endif // QT_CORE_LIB
	Clear();
}

ARthread::~ARthread(){
	Clear();
}

void ARthread::Clear()
{
	for (size_t i = 0; i < mAR.size(); i++){
		delete mAR[i];
	}
	mAR.clear();
}


void ARthread::Init(int startCh, int endCh, int blockSize, ARparms parms)
{
	mStartCh = startCh;
	mEndCh = endCh;
	mBlockSize = blockSize;

	for (int ch = mStartCh; ch < mEndCh; ch += mBlockSize){
		ARChannel *tmp = new ARChannel(ch, parms);
		mAR.push_back(tmp);
	}
	mOutputElements = mAR[0]->GetOutputElements();
}

void ARthread::Process()
{
	for (size_t i = 0; i < mAR.size(); i++)
		mAR[i]->Calculate();
}

void ARthread::UpdateBuffer(const GenericSignal *in)
{
	for (size_t i = 0; i < mAR.size(); i++)
		mAR[i]->UpdateBuffer(in);
}

void ARthread::UpdateBuffer(double *in)
{
	for (size_t i = 0; i < mAR.size(); i++)
		mAR[i]->UpdateBuffer(in);
}

void ARthread::UpdatePower(GenericSignal *out)
{
	for (size_t i = 0; i < mAR.size(); i++){
		int ch = mAR[i]->getChannel();
		mOutputElements = mAR[i]->GetOutputElements();
		for (size_t s = 0; s < mAR[i]->mSpectrum.size(); s++){
			(*out)(ch,s) = (mAR[i]->mSpectrum)[s];
		}
	}
}

void ARthread::UpdatePower(double *out)
{
	for (size_t i = 0; i < mAR.size(); i++){
		int ch = mAR[i]->getChannel();
		mOutputElements = mAR[i]->GetOutputElements();
		for (size_t s = 0; s < mAR[i]->mSpectrum.size(); s++){
			out[s + ch*mOutputElements] = (mAR[i]->mSpectrum)[s];
		}
	}
}

void ARthread::UpdateCoeffs(GenericSignal *out)
{
	for (size_t i = 0; i < mAR.size(); i++){
		int ch = mAR[i]->getChannel();
		mOutputElements = mAR[i]->GetOutputElements();
		for (size_t s = 0; s < mAR[i]->mCoeff.size(); s++){
			(*out)(ch,s) = (mAR[i]->mCoeff)[s].real();
		}
	}
}

void ARthread::UpdateCoeffs(double *out)
{
	for (size_t i = 0; i < mAR.size(); i++){
		int ch = mAR[i]->getChannel();
		mOutputElements = mAR[i]->GetOutputElements();
		for (size_t s = 0; s < mAR[i]->mCoeff.size(); s++){
			out[s + ch*mOutputElements] = (mAR[i]->mCoeff)[s].real();
		}
	}
}

