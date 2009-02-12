///////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, schalk@wadsworth.org,
//          juergen.mellinger@uni-tuebingen.de
// Description: A Matlab (mex) subroutine that applies the MEM spectral
//  estimator available in the BCI2000 AR signal processing module.
//  For the calling syntax, see the USAGE macro below.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "mex.h"

#include "mexutils.h"
#include "Detrend.h"
#include "MEMPredictor.h"
#include "TransferSpectrum.h"
#include <limits>

#define USAGE \
  "The calling syntax is:\n" \
  "[spectrum, frequencies] = mem(signal, parms)\n" \
  " with <signal> and <spectrum> having dimensions channels x values, and\n" \
  " with <parms> being a vector of parameter values:\n" \
  " model order,\n" \
  " first bin center,\n" \
  " last bin center,\n" \
  " bin width,\n" \
  " evaluations per bin,\n" \
  " detrend option (optional, 0: none, 1: mean, 2: linear; defaults to none),\n" \
  " frequency (optional, defaults to 1)"

using namespace std;

const double eps = numeric_limits<double>::epsilon();

enum detrendOptions
{
  none = 0,
  mean = 1,
  linear = 2,
};

void
mexFunction( int nlhs, mxArray* plhs[],
             int nrhs, const mxArray* prhs[] )
{
  if( PrintVersion( __FILE__, nrhs, prhs ) )
    return;
  
  const mxArray* inSignalArray = prhs[ 0 ],
               * inParmsArray = prhs[ 1 ];

  // Check for proper number of arguments
  if( nrhs != 2 )
    ::mexErrMsgTxt( "Two input arguments required -- " USAGE );
  if( nlhs > 2 )
    ::mexErrMsgTxt( "Too many output arguments -- " USAGE );

  int numSamples  = ::mxGetM( inSignalArray ),
      numChannels = ::mxGetN( inSignalArray ),
      numParms    = ::mxGetNumberOfElements( inParmsArray );

  if( !::mxIsDouble( inSignalArray ) || ::mxIsComplex( inSignalArray ) )
    ::mexErrMsgTxt( "Expected real double input signal -- " USAGE );
  if( !::mxIsDouble( inParmsArray ) || ::mxIsComplex( inParmsArray ) )
    ::mexErrMsgTxt( "Expected real double parameter vector -- " USAGE );
  if ( numParms < 5 )
    ::mexErrMsgTxt( "Expected at least 5 MEM parameter values -- " USAGE );

  double* inSignal = ::mxGetPr( inSignalArray ),
        * inParms  = ::mxGetPr( inParmsArray ),
        * p = inParms;
  int     modelOrder        = *p++;
  double  firstBinCenter    = *p++,
          lastBinCenter     = *p++,
          binWidth          = *p++;
  int     evaluationsPerBin = *p++,
          detrendOption     = numParms > ( p - inParms ) ? *p++ : none;
  double  frequency         = numParms > ( p - inParms ) ? *p++ : 1;

  firstBinCenter /= frequency;
  lastBinCenter /= frequency;
  binWidth /= frequency;

  if( modelOrder >= numSamples )
    ::mexErrMsgTxt( "The number of input samples must exceed the model order." );
  if( binWidth <= 0.0 || binWidth > 0.5 )
    ::mexErrMsgTxt( "Bin width must be between 0 and half the sampling rate." );
  if( firstBinCenter < 0.0 || firstBinCenter > 0.5 )
    ::mexErrMsgTxt( "First bin center must be between 0 and half the sampling rate." );
  if( lastBinCenter < firstBinCenter || lastBinCenter > 0.5 )
    ::mexErrMsgTxt( "Last bin center must be between firstBinCenter and half the sampling rate." );
  if( evaluationsPerBin < 1 )
    ::mexErrMsgTxt( "There must be at least 1 evaluation per bin." );
  switch( detrendOption )
  {
    case none:
    case mean:
    case linear:
      break;
    default:
      ::mexErrMsgTxt( "Unknown detrend option." );
   }

  int numBins = ::floor( ( lastBinCenter - firstBinCenter + eps ) / binWidth + 1 );
  plhs[ 0 ] = ::mxCreateDoubleMatrix( numBins, numChannels, mxREAL );
  double* outSpectrum = ::mxGetPr( plhs[ 0 ] );
  if( nlhs > 1 )
  {
    plhs[ 1 ] = ::mxCreateDoubleMatrix( numBins, 1, mxREAL );
    double* outBinFreqs = ::mxGetPr( plhs[ 1 ] );
    for( int bin = 0; bin < numBins; ++bin )
      outBinFreqs[ bin ] = ( firstBinCenter + bin * binWidth ) * frequency;
  }

  MEMPredictor<double> predictor;
  predictor.SetModelOrder( modelOrder );

  TransferSpectrum<double> spectrum;
  spectrum.SetFirstBinCenter( firstBinCenter )
          .SetNumBins( numBins )
          .SetBinWidth( binWidth )
          .SetEvaluationsPerBin( evaluationsPerBin );

  double* input = inSignal,
        * output = outSpectrum;
  while( input < inSignal + numSamples * numChannels )
  {
    valarray<double> signal( input, numSamples );
    const valarray<double>* detrendedSignal = &signal;
    switch( detrendOption )
    {
      case none:
        detrendedSignal = &signal;
        break;
      case mean:
        detrendedSignal = &Detrend::MeanDetrend( signal );
        break;
      case linear:
        detrendedSignal = &Detrend::LinearDetrend( signal );
        break;
    }
    const valarray<double>& bins = spectrum.Evaluate( predictor.TransferFunction( *detrendedSignal ) );
    for( size_t bin = 0; bin < bins.size(); ++bin )
      *output++ = bins[ bin ];
    input += numSamples;
  }
}
