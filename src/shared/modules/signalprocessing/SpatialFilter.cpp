////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de,
//          Adam Wilson
// Description: The SpatialFilter computes a linear transformation of its
//   input signal, given by a matrix-valued parameter.
//   In this matrix, input channels correspond to columns, and output channels
//   to rows.
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

#include <numeric>
#include "SpatialFilter.h"

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
   "Filtering:SpatialFilter int SFUseThreading= 1 0 0 1 "
     "//Use threading to calculate spatial filter (boolean)",

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
      DoPreflightNone( Input, Output );
      break;

    case fullMatrix:
      DoPreflightFull( Input, Output );
      break;

    case sparseMatrix:
      DoPreflightSparse( Input, Output );
      break;

    case commonAverage:
      DoPreflightCAR( Input, Output );
      break;

    default:
      bcierr << "Unexpected filter type ("
             << int( Parameter( "SpatialFilterType" ) )
             << ")"
             << endl;
   }
  Parameter("SFUseThreading");
}

void
SpatialFilter::Initialize( const SignalProperties& Input,
                           const SignalProperties& Output )
{
  mSF.Clear();
  mUseThreading = (bool)int(Parameter("SFUseThreading"));
  mSpatialFilterType = Parameter( "SpatialFilterType" );
  switch( mSpatialFilterType )
  {
    case none:
      DoInitializeNone( Input, Output );
      break;

    case fullMatrix:
      DoInitializeFull( Input, Output );
      break;

    case sparseMatrix:
      DoInitializeSparse( Input, Output );
      break;

    case commonAverage:
      DoInitializeCAR( Input, Output );
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
      DoProcessNone( Input, Output );
      break;

    case fullMatrix:
      DoProcessFull( Input, Output );
      break;

    case sparseMatrix:
      DoProcessSparse( Input, Output );
      break;

    case commonAverage:
      DoProcessCAR( Input, Output );
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////
// SpatialFilter::none
///////////////////////////////////////////////////////////////////////////////////////
void SpatialFilter::DoPreflightNone( const SignalProperties& Input,
                                           SignalProperties& Output ) const
{
  Output = Input;
}


void
SpatialFilter::DoInitializeNone( const SignalProperties& /*Input*/,
                                 const SignalProperties& /*Output*/ )
{
}


void
SpatialFilter::DoProcessNone( const GenericSignal& Input,
                                    GenericSignal& Output )
{
  Output = Input;
}

///////////////////////////////////////////////////////////////////////////////////////
// SpatialFilter::fullMatrix
///////////////////////////////////////////////////////////////////////////////////////
void
SpatialFilter::DoPreflightFull( const SignalProperties& Input,
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

void
SpatialFilter::DoInitializeFull( const SignalProperties& Input,
                                 const SignalProperties& /*Output*/ )
{
  size_t numRows = Parameter( "SpatialFilter" )->NumRows(),
         numCols = Parameter( "SpatialFilter" )->NumColumns();

  mFilterMatrix.resize(numRows);
  //mSignalBuffer.resize(Input.Channels());
  for( size_t row = 0; row < numRows; ++row )
  {
    mFilterMatrix[row].resize(numCols);
    for( size_t col = 0; col < numCols; ++col )
      mFilterMatrix[ row ][ col ] = Parameter( "SpatialFilter" )( row, col );
  }
  mSF.Init(Input.Channels(), numRows, Input.Elements(),&mFilterMatrix);
}

void
SpatialFilter::DoProcessFull( const GenericSignal& Input,
                                    GenericSignal& Output )
{
  mSF.Calculate(&Input, &Output, mUseThreading);
}


///////////////////////////////////////////////////////////////////////////////////////
// SpatialFilter::sparseMatrix
///////////////////////////////////////////////////////////////////////////////////////
void
SpatialFilter::DoPreflightSparse( const SignalProperties& Input,
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
      double inputIdx = Input.ChannelIndex( inputChannelAddress );
      if( inputIdx < 0 || inputIdx >= Input.Channels() )
        bcierr << "Invalid input channel specification \"" << inputChannelAddress
               << "\" in SpatialFilter, row " << i + 1
               << endl;

      string outputAddress = Parameter( "SpatialFilter" )( i, 1 );
      double outputIdx = Output.ChannelIndex( outputAddress );
      if( outputIdx < 0 )
        labels.insert( outputAddress );
      else
        indices.insert( static_cast<int>( outputIdx ) );
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

void
SpatialFilter::DoInitializeSparse( const SignalProperties& Input,
                                   const SignalProperties& Output )
{
  size_t numRows = Parameter( "SpatialFilter" )->NumRows(),
         numCols = Parameter( "SpatialFilter" )->NumColumns();
  mFilterMatrix.resize(numRows);
  string inputChannelAddress, outputChannelAddress;
  for( size_t row = 0; row < numRows; ++row )
  {
    mFilterMatrix[row].resize(numCols);
    inputChannelAddress = (string)Parameter( "SpatialFilter" )( row, 0 );
    outputChannelAddress = (string)Parameter( "SpatialFilter" )( row, 1 );
    mFilterMatrix[ row ][ 0 ] = Input.ChannelIndex( inputChannelAddress );
    mFilterMatrix[ row ][ 1 ] = Output.ChannelIndex( outputChannelAddress );
    if( mFilterMatrix[ row ][ 1 ] < 0 )
      bcierr << "Unexpected inconsistency in channel labels" << endl;
    mFilterMatrix[ row ][ 2 ] = Parameter( "SpatialFilter" )( row, 2 );
  }
}

void
SpatialFilter::DoProcessSparse( const GenericSignal& Input,
                                      GenericSignal& Output )
{
  for (int sample = 0; sample < Input.Elements(); ++sample)
  {
    for (int ch = 0; ch < Output.Channels(); ch++)
      Output(ch, sample) = 0;
    for (size_t entry = 0; entry < mFilterMatrix.size(); ++entry)
    {
      size_t chIn = static_cast<size_t>( mFilterMatrix[entry][0] ),
             chOut = static_cast<size_t>( mFilterMatrix[entry][1] );
      Output(chOut, sample) += Input(chIn, sample)*mFilterMatrix[entry][2];
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////
// SpatialFilter::commonAverage
///////////////////////////////////////////////////////////////////////////////////////
void
SpatialFilter::DoPreflightCAR( const SignalProperties& Input,
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
      double inputIdx = Input.ChannelIndex( inputChannelAddress );
      if( inputIdx < 0 || inputIdx >= Input.Channels() )
      bcierr << "Invalid channel specification \"" << inputChannelAddress
             << "\" in SpatialFilterCAROutput(" << i << ")."
             << " The channel does not exist, or is outside of the allowed range."
             << endl;

      // propagate the channel labels
      if( !Input.ChannelLabels().IsTrivial() )
        Output.ChannelLabels()[ i ] = inputChannelAddress;
    }
  }
}

void
SpatialFilter::DoInitializeCAR( const SignalProperties& Input,
                                const SignalProperties& /*Output*/ )
{
  mCARoutputList.clear();
  if (Parameter("SpatialFilterCAROutput")->NumValues() > 0)
  {
    string inputChannelAddress;
    int inputIdx;
    for (int i = 0; i < Parameter("SpatialFilterCAROutput")->NumValues(); ++i)
    {
      inputChannelAddress = (string)Parameter("SpatialFilterCAROutput")(i);
      inputIdx = static_cast<int>( Input.ChannelIndex( inputChannelAddress ) );
      mCARoutputList.push_back(inputIdx);
    }
  }
  else
  {
    for (int i = 0; i < Input.Channels(); ++i)
      mCARoutputList.push_back(i);
  }
}

void
SpatialFilter::DoProcessCAR( const GenericSignal& Input,
                                   GenericSignal& Output )
{
  for (int sample = 0; sample < Input.Elements(); ++sample)
  {
    double meanVal = 0;
    for (int channel = 0; channel < Input.Channels(); ++channel)
      meanVal += Input(channel, sample);
    meanVal /= double(Input.Channels());

    for (size_t outChannel = 0; outChannel < mCARoutputList.size(); outChannel++)
      Output(outChannel, sample) = Input(mCARoutputList[outChannel], sample) - meanVal;
  }
}



