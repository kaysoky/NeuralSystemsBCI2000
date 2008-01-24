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

#include "SpatialFilter.h"
#include "MeasurementUnits.h"

using namespace std;

RegisterFilter( SpatialFilter, 2.B );

SpatialFilter::SpatialFilter()
{
 BEGIN_PARAMETER_DEFINITIONS

   "Filtering matrix SpatialFilter= 4 4 "
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
SpatialFilter::Initialize( const SignalProperties& /*Input*/,
                           const SignalProperties& /*Output*/ )
{
  size_t numRows = Parameter( "SpatialFilter" )->NumRows(),
         numCols = Parameter( "SpatialFilter" )->NumColumns();
  mFilterMatrix.clear();
  mFilterMatrix.resize( numRows, vector<double>( numCols ) );
  for( size_t row = 0; row < numRows; ++row )
    for( size_t col = 0; col < numCols; ++col )
      mFilterMatrix[ row ][ col ] = Parameter( "SpatialFilter" )( row, col );
}

void
SpatialFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  // Actually perform Spatial Filtering on the input and write it into the output signal.
  for( int sample = 0; sample < Input.Elements(); ++sample )
    for( int outChannel = 0; outChannel < Output.Channels(); ++outChannel )
    {
      double value = 0;
      for( int inChannel = 0; inChannel < Input.Channels(); ++inChannel )
        value += mFilterMatrix[ outChannel ][ inChannel ] * Input( inChannel, sample );
      Output( outChannel, sample ) = value;
    }
}


