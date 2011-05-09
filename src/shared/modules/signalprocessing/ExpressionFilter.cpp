////////////////////////////////////////////////////////////////////////////////
//  $Id$
//  Author:      juergen.mellinger@uni-tuebingen.de
//  Description: A filter that uses arithmetic expressions to compute its
//    output.
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
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "ExpressionFilter.h"

using namespace std;

RegisterFilter( ExpressionFilter, 2.D2 );

ExpressionFilter::ExpressionFilter()
{
  BEGIN_PARAMETER_DEFINITIONS
    "Filtering matrix Expressions= 0 1 "
      " % % % // expressions used to compute the output of the ExpressionFilter"
                                   " (rows are channels; empty matrix for none)",
  END_PARAMETER_DEFINITIONS
}


ExpressionFilter::~ExpressionFilter()
{
}


void
ExpressionFilter::Preflight( const SignalProperties& Input,
                                   SignalProperties& Output ) const
{
  // Evaluate all expressions to check for errors.
  GenericSignal preflightInput( Input );
  // Are the entries of the Expressions parameter ok?
  int numRows = Parameter( "Expressions" )->NumRows(),
      numCols = Parameter( "Expressions" )->NumColumns();
  for( int row = 0; row < numRows; ++row )
    for( int col = 0; col < numCols; ++col )
      Expression( Parameter( "Expressions" )( row, col ) ).Evaluate( &preflightInput );

  // Request output signal properties:
  Output = Input;
  if( numRows != 0 && numCols != 0 )
    Output.SetChannels( numRows )
          .SetElements( numCols )
          .ElementUnit().SetGain( 1.0 ).SetOffset( 0.0 ).SetSymbol( "" );
}


void
ExpressionFilter::Initialize( const SignalProperties&,
                              const SignalProperties& )
{
  // Read the expression matrix parameter into the mExpressions array.
  mExpressions.clear();
  int numRows = Parameter( "Expressions" )->NumRows(),
      numCols = Parameter( "Expressions" )->NumColumns();
  if( numRows != 0 && numCols != 0 )
  {
    mExpressions.resize( numRows, vector<Expression>( numCols ) );
    for( int row = 0; row < numRows; ++row )
      for( int col = 0; col < numCols; ++col )
        mExpressions[ row ][ col ] = Expression( Parameter( "Expressions" )( row, col ) );
  }
}


void
ExpressionFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  if( !mExpressions.empty() )
  { // Use expressions to compute the output signal.
    for( size_t channel = 0; channel < mExpressions.size(); ++channel )
      for( size_t sample = 0; sample < mExpressions[ channel ].size(); ++sample )
        Output( channel, sample ) = mExpressions[ channel ][ sample ].Evaluate( &Input );
  }
  else
  {
    Output = Input;
  }
}

