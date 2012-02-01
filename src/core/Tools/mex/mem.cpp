///////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, schalk@wadsworth.org,
//          juergen.mellinger@uni-tuebingen.de
// Description: A Matlab (mex) subroutine that applies the MEM spectral
//  estimator available in the BCI2000 AR signal processing module.
//  For the calling syntax, see the USAGE macro below.
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
///////////////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "mex.h"

#include "mexutils.h"
#include "ARThread.h"
#include <limits>

#define USAGE \
"The mem command estimates a power spectrum using Burg's MEM algorithm.\n" \
"The calling syntax is:\n" \
        "[spectrum, frequencies] = mem(signal, parms)\n" \
        " with <signal> having dimensions (values x channels),\n" \
        " <spectrum> having dimensions (values x channels x blocks)\n" \
        " and with <parms> being a vector of parameter values:\n" \
        " model order,\n" \
        " first bin center,\n" \
        " last bin center,\n" \
        " bin width,\n" \
        " evaluations per bin,\n" \
        " detrend option (optional, 0: none, 1: mean, 2: linear; defaults to none),\n" \
        " frequency (optional, defaults to 1),\n" \
        " sample block size (optional, defaults to size(signal,1)),\n" \
        " window length in sample blocks (optional, defaults to 1)\n"

using namespace std;

const double eps = numeric_limits<double>::epsilon();

void mexFunction( int nlhs, mxArray* plhs[],
                  int nrhs, const mxArray* prhs[] )
{
    if( PrintVersion( __FILE__, nrhs, prhs ) )
        return;

    const mxArray* inSignalArray = prhs[0],
                 * inParmsArray = prhs[1];

    // Check for proper number of arguments
    if( nrhs != 2 )
        ::mexErrMsgTxt( "Two input arguments required -- " USAGE );
    if( nlhs > 2 )
        ::mexErrMsgTxt( "Too many output arguments -- " USAGE );

    int numSamples  = ::mxGetM( inSignalArray ),
        numChannels = ::mxGetN( inSignalArray );
    mwSize numParms = ::mxGetNumberOfElements( inParmsArray );

    if( !::mxIsDouble( inSignalArray ) || ::mxIsComplex( inSignalArray ) )
        ::mexErrMsgTxt( "Expected real double input signal -- " USAGE );
    if( !::mxIsDouble( inParmsArray ) || ::mxIsComplex( inParmsArray ) )
        ::mexErrMsgTxt( "Expected real double parameter vector -- " USAGE );
    if ( numParms < 5 )
        ::mexErrMsgTxt( "Expected at least 5 MEM parameter values -- " USAGE );

    double* inSignal = ::mxGetPr( inSignalArray ),
          * inParms  = ::mxGetPr( inParmsArray ),
          * p = inParms;
    double  modelOrder        = *p++,
            firstBinCenter    = *p++,
            lastBinCenter     = *p++,
            binWidth          = *p++,
            evaluationsPerBin = *p++,
            detrendOption     = numParms > ( p - inParms ) ? *p++ : ARThread::none,
            frequency         = numParms > ( p - inParms ) ? *p++ : 1.0,
            sampleBlockSize   = numParms > ( p - inParms ) ? *p++ : numSamples,
            windowLength      = numParms > ( p - inParms ) ? *p++ : 1;

    if( modelOrder >= numSamples )
        ::mexErrMsgTxt( "The number of input samples must exceed the model order." );
    if( binWidth <= 0.0 || binWidth > frequency/2 )
        ::mexErrMsgTxt( "Bin width must be between 0 and half the sampling rate." );
    if( firstBinCenter < 0.0 || firstBinCenter > frequency/2 )
        ::mexErrMsgTxt( "First bin center must be between 0 and half the sampling rate." );
    if( lastBinCenter < firstBinCenter || lastBinCenter > frequency/2 )
        ::mexErrMsgTxt( "Last bin center must be between firstBinCenter and half the sampling rate." );
    if( evaluationsPerBin < 1 )
        ::mexErrMsgTxt( "There must be at least 1 evaluation per bin." );
    if( frequency < eps )
        ::mexErrMsgTxt( "Frequency must be > 0." );
    if( sampleBlockSize < 1 )
        ::mexErrMsgTxt( "Sample block size must be >= 1." );
    if( windowLength * sampleBlockSize < 1 )
        ::mexErrMsgTxt( "Window must contain at least one sample." );
    switch( static_cast<int>( detrendOption ) ) 
    {
        case ARThread::none:
        case ARThread::mean:
        case ARThread::linear:
            break;
        default:
            ::mexErrMsgTxt( "Unknown detrend option." );
    }

    int numBins = static_cast<int>( ::floor( ( lastBinCenter - firstBinCenter + eps ) / binWidth + 1 ) ),
        numBlocks = static_cast<int>( numSamples / sampleBlockSize ),
        iSampleBlockSize = static_cast<int>( sampleBlockSize );
    const mwSize dims[] = { numBins, numChannels, numBlocks };
    plhs[0] = ::mxCreateNumericArray( 3, dims, mxDOUBLE_CLASS, mxREAL );
    double* outSpectrum = ::mxGetPr( plhs[0] );
    if( nlhs > 1 )
    {
        plhs[1] = ::mxCreateDoubleMatrix( numBins, 1, mxREAL );
        double* outBinFreqs = ::mxGetPr( plhs[1] );
        for( int bin = 0; bin < numBins; ++bin )
            outBinFreqs[bin] = ( firstBinCenter + bin * binWidth );
    }

    vector<ARThread*> threads;
    threads.resize( min( numChannels, OSThread::NumberOfProcessors() ) );
    for( size_t i = 0; i < threads.size(); ++i )
        threads[i] = new ARThread;
    for( int ch = 0; ch < numChannels; ++ch )
        threads[ch % threads.size()]->AddChannel( ch );
    for( size_t i = 0; i < threads.size(); ++i )
        threads[i]->SetWindowLength( static_cast<int>( sampleBlockSize * windowLength ) )
                   .SetDetrend( static_cast<int>( detrendOption ) )
                   .SetModelOrder( static_cast<int>( modelOrder ) )
                   .SetFirstBinCenter( firstBinCenter / frequency )
                   .SetBinWidth( binWidth / frequency )
                   .SetNumBins( numBins )
                   .SetEvaluationsPerBin( static_cast<int>( evaluationsPerBin ) )
                   .SetOutputType( ARThread::SpectralPower );

    GenericSignal input( numChannels, iSampleBlockSize ),
                  output( numChannels, numBins );
    for( int block = 0, blockNum = 0; block <= numSamples - iSampleBlockSize; block += iSampleBlockSize, ++blockNum )
    {
        for( int ch = 0; ch < numChannels; ++ch )
            for( int s = 0; s < iSampleBlockSize; ++s )
                input( ch, s ) = inSignal[s + block + ch*iSampleBlockSize];

        for( size_t i = 0; i < threads.size(); ++i )
            threads[i]->Start( input, output );
        for( size_t i = 0; i < threads.size(); ++i )
            threads[i]->Wait();

        for( int ch = 0; ch < numChannels; ++ch )
            for( int bin = 0; bin < numBins; ++bin )
                outSpectrum[numChannels*numBins*blockNum + numBins*ch + bin] = output( ch, bin );
    }
    for( size_t i = 0; i < threads.size(); ++i )
        delete threads[i];
}
