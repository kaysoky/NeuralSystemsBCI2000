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
#include "PCHIncludes.h"
#pragma hdrstop

#include <limits>
#include "SpatialFilter.h"

using namespace std;

static const GenericSignal::ValueType eps = numeric_limits<GenericSignal::ValueType>::epsilon();

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
  "Filtering:SpatialFilter matrix SpatialFilter= 4 4 "
     "1 0 0 0 "
     "0 1 0 0 "
     "0 0 1 0 "
     "0 0 0 1 "
     "0 % % // columns represent input channels, rows represent output channels",
  "Filtering:SpatialFilter intlist SpatialFilterCAROutput= 0 % % % % "
    "// when using CAR filter type: list of output channels, or empty for all channels",
  "Filtering:SpatialFilter int SpatialFilterMissingChannels= 1 0 0 1 "
    "// how to handle missing channels "
    "0: ignore, "
    "1: report error "
    "(enumeration)",

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
  mThreadGroup.Preflight();
}

void
SpatialFilter::Initialize( const SignalProperties& Input,
                           const SignalProperties& Output )
{
  mThreadGroup.Clear();
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
    case fullMatrix:
    case sparseMatrix:
    case commonAverage:
      mThreadGroup.Process( Input, Output );
      break;

     default:
      Output = Input;
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

///////////////////////////////////////////////////////////////////////////////////////
// SpatialFilter::fullMatrix
///////////////////////////////////////////////////////////////////////////////////////
void
SpatialFilter::DoPreflightFull( const SignalProperties& Input,
                                      SignalProperties& Output ) const
{
  const ParamRef& SpatialFilter = Parameter( "SpatialFilter" );
  // Parameter/Input consistency.
  if( Input.Channels() != SpatialFilter->NumColumns() )
    bcierr << "The input signal's number of channels ("
           << Input.Channels() << ") must match "
           << "the number of columns in the SpatialFilter parameter ("
           << SpatialFilter->NumColumns() << ")"
           << endl;
  // Output signal description.
  Output = Input;
  Output.SetChannels( 0 ).SetChannels( SpatialFilter->NumRows() );
  if( !SpatialFilter->RowLabels().IsTrivial() )
    for( int i = 0; i < SpatialFilter->NumRows(); ++i )
      Output.ChannelLabels()[ i ] = SpatialFilter->RowLabels()[ i ];
}

void
SpatialFilter::DoInitializeFull( const SignalProperties& Input,
                                 const SignalProperties& /*Output*/ )
{
  int numRows = Parameter( "SpatialFilter" )->NumRows(),
      numCols = Parameter( "SpatialFilter" )->NumColumns();
  mFullMatrix = GenericSignal( numRows, numCols );
  for( int row = 0; row < numRows; ++row )
    for( int col = 0; col < numCols; ++col )
      mFullMatrix( row, col ) = Parameter( "SpatialFilter" )( row, col );
  mThreadGroup.Initialize( Input, mFullMatrix );
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

  const ParamRef& SpatialFilter = Parameter( "SpatialFilter" );
  if( SpatialFilter->NumColumns() != 3 )
  {
    bcierr << "The SpatialFilter parameter must have 3 columns when representing "
           << "a sparse matrix: input channel, output channel, weight"
           << endl;
  }
  else
  {
    // In the output field, the user may specify arbitrary labels but also
    // numeric indices. We make sure that labels will only be assigned to
    // indices that have not been specified as raw numbers.
    enum { input, output, weight };
    set<string> labels;
    set<int> indices;
    for( int i = 0; i < SpatialFilter->NumRows(); ++i )
    {
      string outputAddress = SpatialFilter( i, output );
      double outputIdx = Output.ChannelIndex( outputAddress );
      if( outputIdx < 0 )
        labels.insert( outputAddress );
      else
        indices.insert( static_cast<int>( outputIdx ) );
    }
    bool reportMissingChannels = ( Parameter( "SpatialFilterMissingChannels" ) != 0 );
    for( int i = 0; i < SpatialFilter->NumRows(); ++i )
    { // Remove output channels that depend on missing inputs.
      string inputChannelAddress = SpatialFilter( i, input );
      double inputIdx = Input.ChannelIndex( inputChannelAddress );
      if( inputIdx < 0 || inputIdx >= Input.Channels() )
      {
        if( reportMissingChannels )
          bcierr << "Invalid input channel specification \"" << inputChannelAddress
                 << "\" in SpatialFilter, row " << i + 1
                 << endl;
        string outputAddress = SpatialFilter( i, output );
        double outputIdx = Output.ChannelIndex( outputAddress );
        if( outputIdx < 0 )
          labels.erase( outputAddress );
        else
          indices.erase( static_cast<int>( outputIdx ) );
      }
    }
    int numOutputChannels = indices.empty() ? 0 : *indices.rbegin() + 1,
        availableIndices = numOutputChannels - static_cast<int>( indices.size() ),
        requiredChannels = numOutputChannels - availableIndices + static_cast<int>( labels.size() );
    if( requiredChannels > numOutputChannels )
      numOutputChannels = requiredChannels;
    if( numOutputChannels < 1 )
      bcierr << "SpatialFilter output is empty."
             << ( reportMissingChannels ? "" : " Set SpatialFilterMissingChannels to report invalid input channels." )
             << endl;
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
  const ParamRef& SpatialFilter = Parameter( "SpatialFilter" );
  int numRows = SpatialFilter->NumRows(),
      numCols = SpatialFilter->NumColumns();
  mSparseMatrix.clear();
  for( int row = 0; row < numRows; ++row )
  {
    enum { input, output, weight };
    SparseMatrixEntry entry =
    {
      static_cast<int>( Input.ChannelIndex( SpatialFilter( row, input ) ) ),
      static_cast<int>( Output.ChannelIndex( SpatialFilter( row, output ) ) ),
      SpatialFilter( row, weight )
    };
    if( entry.input >= 0 && entry.output >= 0 && ::fabs( entry.weight ) > eps )
      mSparseMatrix.push_back( entry );
  }
  mThreadGroup.Initialize( Input, mSparseMatrix );
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
    Output.SetChannels( 0 );
    Output.ChannelLabels().Clear();
    Output.ChannelUnit().Clear();
    for( int i = 0; i < Parameter( "SpatialFilterCAROutput" )->NumValues(); ++i )
    {
      string inputChannelAddress = Parameter( "SpatialFilterCAROutput" )( i );
      double inputIdx = Input.ChannelIndex( inputChannelAddress );
      if( inputIdx >= 0 && inputIdx < Input.Channels() )
      {
        Output.SetChannels( Output.Channels() + 1 );
        if( !Input.ChannelLabels().IsTrivial() )
          Output.ChannelLabels()[ i ] = inputChannelAddress;
      }
      else if( Parameter( "SpatialFilterMissingChannels" ) != 0 )
      {
        bcierr << "Invalid channel specification \"" << inputChannelAddress
               << "\" in SpatialFilterCAROutput(" << i << ")."
               << " The channel does not exist, or is outside the allowed range."
               << endl;
      }
    }
  }
}

void
SpatialFilter::DoInitializeCAR( const SignalProperties& Input,
                                const SignalProperties& /*Output*/ )
{
  mCAROutputList.clear();
  if (Parameter("SpatialFilterCAROutput")->NumValues() > 0)
  {
    string inputChannelAddress;
    int inputIdx;
    for (int i = 0; i < Parameter("SpatialFilterCAROutput")->NumValues(); ++i)
    {
      inputChannelAddress = (string)Parameter("SpatialFilterCAROutput")(i);
      inputIdx = static_cast<int>( Input.ChannelIndex( inputChannelAddress ) );
      if( inputIdx >= 0 && inputIdx < Input.Channels() )
        mCAROutputList.push_back(inputIdx);
    }
  }
  else
  {
    for (int i = 0; i < Input.Channels(); ++i)
      mCAROutputList.push_back(i);
  }
  mThreadGroup.Initialize( Input, mCAROutputList );
}

