////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The SpatialFilter computes a linear transformation of its
//   input signal, given by a matrix-valued parameter.
//   In this matrix, input channels correspond to columns, and output channels
//   to rows.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include <numeric>
#include "SpatialFilter.h"
#include "MeasurementUnits.h"

using namespace std;

RegisterFilter( SpatialFilter, 2.B );

SpatialFilter::SpatialFilter()
{
 BEGIN_PARAMETER_DEFINITIONS

  "Filtering int SpatialFilterType= 1 2 0 3 "
     "// spatial filter type "
   "0: none, "
     "1: full matrix, "
     "2: sparse matrix, "
     "3: common average reference (CAR) "
     "(enumeration)",
  "Filtering:SpatialFilter intlist SpatialFilterCAROutput= 0 % % % % "
    "// list of output channels for the CAR if used",
   "Filtering:SpatialFilter matrix SpatialFilter= 4 4 "
     "1 0 0 0 "
     "0 1 0 0 "
     "0 0 1 0 "
     "0 0 0 1 "
   "0 % % // columns represent input channels, rows represent output channels",

 END_PARAMETER_DEFINITIONS
}


SpatialFilter::~SpatialFilter()
{
}


void
SpatialFilter::Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const
{
  switch( int( Parameter( "SpatialFilterType" ) ) )
  {
    case none:
      DoPreflight<none>( Input, Output );
      break;

    case fullMatrix:
      DoPreflight<fullMatrix>( Input, Output );
      break;

    case sparseMatrix:
      DoPreflight<sparseMatrix>( Input, Output );
      break;

    case commonAverage:
      DoPreflight<commonAverage>( Input, Output );
      break;

    default:
      bcierr << "Unexpected filter type ("
             << int( Parameter( "SpatialFilterType" ) )
             << ")"
             << endl;
   }
}

void
SpatialFilter::Initialize( const SignalProperties& Input,
                           const SignalProperties& Output )
{
  mSpatialFilterType = Parameter( "SpatialFilterType" );
  switch( mSpatialFilterType )
  {
    case none:
      DoInitialize<none>( Input, Output );
      break;

    case fullMatrix:
      DoInitialize<fullMatrix>( Input, Output );
      break;

    case sparseMatrix:
      DoInitialize<sparseMatrix>( Input, Output );
      break;

    case commonAverage:
      DoInitialize<commonAverage>( Input, Output );
      break;
  }
}

void
SpatialFilter::Process( const GenericSignal& Input,
                              GenericSignal& Output )
{
  switch( mSpatialFilterType )
  {
    case none:
      DoProcess<none>( Input, Output );
      break;

    case fullMatrix:
      DoProcess<fullMatrix>( Input, Output );
      break;
    
    case sparseMatrix:
      DoProcess<sparseMatrix>( Input, Output );
      break;

    case commonAverage:
      DoProcess<commonAverage>( Input, Output );
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////
// SpatialFilter::none
///////////////////////////////////////////////////////////////////////////////////////
template<>
void
SpatialFilter::DoPreflight<SpatialFilter::none>( const SignalProperties& Input,
                                                       SignalProperties& Output ) const
{
  Output = Input;
}

template<>
void
SpatialFilter::DoInitialize<SpatialFilter::none>( const SignalProperties& /*Input*/,
                                                  const SignalProperties& /*Output*/ )
{
}

template<>
void
SpatialFilter::DoProcess<SpatialFilter::none>( const GenericSignal& Input,
                                                     GenericSignal& Output )
{
  Output = Input;
}

///////////////////////////////////////////////////////////////////////////////////////
// SpatialFilter::fullMatrix
///////////////////////////////////////////////////////////////////////////////////////
template<>
void
SpatialFilter::DoPreflight<SpatialFilter::fullMatrix>( const SignalProperties& Input,
                                                             SignalProperties& Output ) const
{
  // Parameter/Input consistency.
  if( Input.Channels() != Parameter( "SpatialFilter" )->NumColumns() )
    bcierr << "The input signal's number of channels must match "
           << "the number of columns in the SpatialFilter parameter"
           << endl;
  // Output signal description.
  Output = Input;
  Output.SetChannels( 0 ).SetChannels( Parameter( "SpatialFilter" )->NumRows() );
  if( !Parameter( "SpatialFilter" )->RowLabels().IsTrivial() )
    for( int i = 0; i < Parameter( "SpatialFilter" )->NumRows(); ++i )
      Output.ChannelLabels()[ i ] = Parameter( "SpatialFilter" )->RowLabels()[ i ];
}

template<>
void
SpatialFilter::DoInitialize<SpatialFilter::fullMatrix>( const SignalProperties& Input,
                                                        const SignalProperties& /*Output*/ )
{
  size_t numRows = Parameter( "SpatialFilter" )->NumRows(),
         numCols = Parameter( "SpatialFilter" )->NumColumns();

  mFilterMatrix.resize(numRows);
  mSignalBuffer.resize(Input.Channels());
  for( size_t row = 0; row < numRows; ++row )
  {
    mFilterMatrix[row].resize(numCols);
    for( size_t col = 0; col < numCols; ++col )
      mFilterMatrix[ row ][ col ] = Parameter( "SpatialFilter" )( row, col );
  }
}

template<>
void
SpatialFilter::DoProcess<SpatialFilter::fullMatrix>( const GenericSignal& Input,
                                                           GenericSignal& Output )
{
  for( int sample = 0; sample < Input.Elements(); ++sample )
  {
    // Copy the input signal to the buffer
    for (int inCh = 0; inCh < Input.Channels(); ++inCh)
      mSignalBuffer[inCh] = Input(inCh, sample);

    for (int outCh = 0; outCh < Output.Channels(); ++outCh)
      Output( outCh, sample ) = std::inner_product(&mSignalBuffer[0],
                                                   &mSignalBuffer[Input.Channels()],
                                                   &mFilterMatrix[outCh][0],
                                                   NumType(0));
  }
}


///////////////////////////////////////////////////////////////////////////////////////
// SpatialFilter::sparseMatrix
///////////////////////////////////////////////////////////////////////////////////////
template<>
void
SpatialFilter::DoPreflight<SpatialFilter::sparseMatrix>( const SignalProperties& Input,
                                                               SignalProperties& Output ) const
{
  Output = Input;
  Output.ChannelLabels().Clear();
  Output.ChannelUnit().Clear();
  
  if( Parameter( "SpatialFilter" )->NumColumns() != 3 )
  {
    bcierr << "The SpatialFilter parameter must have 3 columns when representing "
           << "a sparse matrix: input channel, output channel, weight"
           << endl;
  }
  else
  {
    // In the output field, the user may specify arbitrary labels but also
    // numeric indices. We make sure that labels will only be assigned to
    // indices that have not been specified as numbers.
    set<string> labels;
    set<int>    indices;
    for( int i = 0; i < Parameter( "SpatialFilter" )->NumRows(); ++i )
    {
      string inputChannelAddress = Parameter( "SpatialFilter" )( i, 0 );
      int inputIdx = Input.ChannelIndex( inputChannelAddress );
      if( inputIdx < 0 || inputIdx >= Input.Channels() )
        bcierr << "Invalid input channel specification \"" << inputChannelAddress
               << "\" in SpatialFilter, row " << i + 1
               << endl;

      string outputAddress = Parameter( "SpatialFilter" )( i, 1 );
      int outputIdx = Output.ChannelIndex( outputAddress );
      if( outputIdx < 0 )
        labels.insert( outputAddress );
      else
        indices.insert( outputIdx );
    }
    int numOutputChannels = indices.empty() ? 0 : *indices.rbegin() + 1,
        freeIndices = numOutputChannels - indices.size(),
        requiredChannels = numOutputChannels - freeIndices + labels.size();
    if( requiredChannels > numOutputChannels )
      numOutputChannels = requiredChannels;
    Output.SetChannels( numOutputChannels );

    indices.insert( numOutputChannels );
    set<int>::const_iterator p = indices.begin();
    set<string>::const_iterator q = labels.begin();
    int idxBegin = 0;
    while( q != labels.end() )
    {
      for( int i = idxBegin; i < *p && q != labels.end(); ++i )
        Output.ChannelLabels()[i] = *q++;
      idxBegin = 1 + *p++;
    }
    if( p != indices.end() && q != labels.end() )
      bcierr << "Unexpected inconsistency when assigning channel labels" << endl;
  }
}

template<>
void
SpatialFilter::DoInitialize<SpatialFilter::sparseMatrix>( const SignalProperties& Input,
                                                          const SignalProperties& Output )
{
  size_t numRows = Parameter( "SpatialFilter" )->NumRows(),
         numCols = Parameter( "SpatialFilter" )->NumColumns();
  mFilterMatrix.resize(numRows);
  string inputChannelAddress, outputChannelAddress;
  for( size_t row = 0; row < numRows; ++row )
  {
    mFilterMatrix[row].resize(numCols);
    inputChannelAddress = Parameter( "SpatialFilter" )( row, 0 );
    outputChannelAddress = Parameter( "SpatialFilter" )( row, 1 );
    mFilterMatrix[ row ][ 0 ] = Input.ChannelIndex( inputChannelAddress );
    mFilterMatrix[ row ][ 1 ] = Output.ChannelIndex( outputChannelAddress );
    if( mFilterMatrix[ row ][ 1 ] < 0 )
      bcierr << "Unexpected inconsistency in channel labels" << endl;
    mFilterMatrix[ row ][ 2 ] = Parameter( "SpatialFilter" )( row, 2 );
  }
}

template<>
void
SpatialFilter::DoProcess<SpatialFilter::sparseMatrix>( const GenericSignal& Input,
                                                             GenericSignal& Output )
{
  for (int sample = 0; sample < Input.Elements(); ++sample)
  {
    for (int ch = 0; ch < Output.Channels(); ch++)
      Output(ch, sample) = 0;
    for (size_t entry = 0; entry < mFilterMatrix.size(); ++entry)
      Output(mFilterMatrix[entry][1], sample) += Input(mFilterMatrix[entry][0], sample)*mFilterMatrix[entry][2];
  }
}

///////////////////////////////////////////////////////////////////////////////////////
// SpatialFilter::commonAverage
///////////////////////////////////////////////////////////////////////////////////////
template<>
void
SpatialFilter::DoPreflight<SpatialFilter::commonAverage>( const SignalProperties& Input,
                                                                SignalProperties& Output ) const
{
  Output = Input;

  if ( Parameter( "SpatialFilterCAROutput" )->NumValues() > 0 )
  {
    Output.ChannelLabels().Clear();
    Output.ChannelUnit().Clear();
    Output.SetChannels( Parameter( "SpatialFilterCAROutput" )->NumValues() );
    for( int i = 0; i < Parameter( "SpatialFilterCAROutput" )->NumValues(); ++i )
    {
      string inputChannelAddress = Parameter( "SpatialFilterCAROutput" )( i );
      int inputIdx = Input.ChannelIndex( inputChannelAddress );
      if( inputIdx < 0 || inputIdx >= Input.Channels() )
      bcierr << "Invalid channel specification \"" << inputChannelAddress
             << "\" in SpatialFilterCAROutput(" << i << ")."
             << " The channel does not exist, or is outside of the allowed range."
             << endl;

      // propagate the channel labels
      Output.ChannelLabels()[ i ] = inputChannelAddress;
    }
  }
}

template<>
void
SpatialFilter::DoInitialize<SpatialFilter::commonAverage>( const SignalProperties& Input,
                                                           const SignalProperties& /*Output*/ )
{
  mCARoutputList.clear();
  if (Parameter("SpatialFilterCAROutput")->NumValues() > 0)
  {
    string inputChannelAddress;
    int inputIdx;
    for (int i = 0; i < Parameter("SpatialFilterCAROutput")->NumValues(); ++i)
    {
      inputChannelAddress = Parameter("SpatialFilterCAROutput")(i);
      inputIdx = Input.ChannelIndex( inputChannelAddress );
      mCARoutputList.push_back(inputIdx);
    }
  }
  else
  {
    for (int i = 0; i < Input.Channels(); ++i)
      mCARoutputList.push_back(i);
  }
}

template<>
void
SpatialFilter::DoProcess<SpatialFilter::commonAverage>( const GenericSignal& Input,
                                                              GenericSignal& Output )
{
  double meanVal = 0;
  for (int sample = 0; sample < Input.Elements(); ++sample)
  {
    for (int channel = 0; channel < Input.Channels(); ++channel)
      meanVal += Input(channel, sample);
    meanVal /= double(Input.Channels());

    for (size_t outChannel = 0; outChannel < mCARoutputList.size(); outChannel++)
      Output(outChannel, sample) = Input(mCARoutputList[outChannel], sample) - meanVal;
  }
}



