////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The LinearClassifier applies a matrix multiplication to its
//   input data.
//   Input data has 2 indices (N channels x M elements), and output data
//   has a single index (C channels x 1 element), thus the linear classifier
//   acts as a N x M x C matrix, determining the output after summation over
//   N and M.
//
//   The Classifier parameter is a sparse matrix definition in which each row
//   corresponds to a single matrix entry.
//   Columns correspond to
//   1) input channel,
//   2) input element (bin in the spectral case, time offset in the ERP case),
//   3) output channel,
//   4) weight (value of the matrix entry).
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "LinearClassifier.h"
#include "BCIError.h"
#include <algorithm>

RegisterFilter( LinearClassifier, 2.D );

using namespace std;

LinearClassifier::LinearClassifier()
{
 BEGIN_PARAMETER_DEFINITIONS

   "Filtering matrix Classifier= 2 "
     "[ input%20channel input%20element%20(bin) output%20channel weight ] "
     "               1                       4                1      1   "
     "               1                       6                2      1   "
     " % % // Linear classification matrix in sparse representation",

 END_PARAMETER_DEFINITIONS

}


LinearClassifier::~LinearClassifier()
{
}


void
LinearClassifier::Preflight( const SignalProperties& Input,
                                   SignalProperties& Output ) const
{
  // Determine the classifier matrix format:
  int controlSignalChannels = 0;
  const ParamRef& Classifier = Parameter( "Classifier" );
  if( Classifier->NumColumns() != 4 )
    bcierr << "Classifier parameter must have 4 columns "
           << "(input channel, input element, output channel, weight)"
           << endl;
  else
  {
    for( int row = 0; row < Classifier->NumRows(); ++row )
    {
      if( Classifier( row, 2 ) < 1 )
        bcierr << "Output channels must be positive integers"
               << endl;

      float ch = Input.ChannelIndex( Classifier( row, 0 ) );
      if( ch < 0 )
        bcierr << DescribeEntry( row, 0 )
               << " points to negative input index"
               << endl;
      else if( ::floor( ch ) > Input.Channels() )
        bcierr << "Channel specification in "
               << DescribeEntry( row, 0 )
               << " exceeds number of input channels"
               << endl;
      if( ::fmod( ch, 1.0f ) > 1e-2 )
        bciout << "Channel specification in physical units: "
               << DescribeEntry( row, 0 )
               << " does not exactly meet a single channel"
               << endl;

      float el = Input.ElementIndex( Classifier( row, 1 ) );
      if( el < 0 )
        bcierr << DescribeEntry( row, 1 )
               << " points to negative input index"
               << endl;
      if( ::floor( el ) > Input.Elements() )
        bcierr << "Element (bin) specification in "
               << DescribeEntry( row, 1 )
               << " exceeds number of input elements"
               << endl;
      if( ::fmod( el, 1.0f ) > 1e-2 )
        bciout << "Element (bin) specification in physical units: "
               << DescribeEntry( row, 1 )
               << " does not exactly meet a single element"
               << endl;

      int outputChannel =  Classifier( row, 2 );
      controlSignalChannels = max( controlSignalChannels, outputChannel );
    }
  }
  // Requested output signal properties.
  Output = SignalProperties( controlSignalChannels, 1, Input.Type() );
  // Output description.
  Output.ChannelUnit() = Input.ChannelUnit();
  Output.ValueUnit().SetRawMin( Input.ValueUnit().RawMin() )
                    .SetRawMax( Input.ValueUnit().RawMax() );

  float secsPerBlock = Parameter( "SampleBlockSize" ) / Parameter( "SamplingRate" );
  Output.ElementUnit().SetOffset( 0 ).SetGain( secsPerBlock ).SetSymbol( "s" );
  int visualizationTime = Output.ElementUnit().PhysicalToRaw( "15s" );
  Output.ElementUnit().SetRawMin( 0 ).SetRawMax( visualizationTime - 1 );
}


void
LinearClassifier::Initialize( const SignalProperties& Input,
                              const SignalProperties& /*Output*/ )
{
  const ParamRef& Classifier = Parameter( "Classifier" );
  size_t numEntries = Classifier->NumRows();
  mInputChannels.resize( numEntries );
  mInputElements.resize( numEntries );
  mOutputChannels.resize( numEntries );
  mWeights.resize( numEntries );
  for( size_t entry = 0; entry < numEntries; ++entry )
  {
    mInputChannels[ entry ] = Input.ChannelIndex( Classifier( entry, 0 ) );
    mInputElements[ entry ] = Input.ElementIndex( Classifier( entry, 1 ) );
    mOutputChannels[ entry ] = Classifier( entry, 2 ) - 1;
    mWeights[ entry ] = Classifier( entry, 3 );
  }
}


void
LinearClassifier::Process( const GenericSignal& Input, GenericSignal& Output )
{
  for( int ch = 0; ch < Output.Channels(); ++ch )
    for( int el = 0; el < Output.Elements(); ++el )
      Output( ch, el ) = 0.0;

  for( size_t i = 0; i < mWeights.size(); ++i )
    Output( mOutputChannels[ i ], 0 )
      += Input( mInputChannels[ i ], mInputElements[ i ] ) * mWeights[ i ];
}


const std::string&
LinearClassifier::DescribeEntry( int inRow, int inCol ) const
{
  ParamRef Classifier = Parameter( "Classifier" );
  ostringstream oss;
  oss << "Classifier("
      << Classifier->RowLabels()[inRow] << ","
      << Classifier->ColumnLabels()[inCol] << ")="
      << Classifier( inRow, inCol );
  static string result;
  result = oss.str();
  return result;
}

