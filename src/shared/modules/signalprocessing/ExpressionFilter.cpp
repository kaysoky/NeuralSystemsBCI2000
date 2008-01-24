////////////////////////////////////////////////////////////////////////////////
//  $Id$
//  Author:      juergen.mellinger@uni-tuebingen.de
//  Description: A filter that uses arithmetic expressions to compute its
//    output.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "ExpressionFilter.h"

using namespace std;

RegisterFilter( ExpressionFilter, 2.D1 );

ExpressionFilter::ExpressionFilter()
{
  BEGIN_PARAMETER_DEFINITIONS
    "Filtering string WarningExpression= % "
      " % % % // expression that results in a warning when it evaluates to true",
    "Filtering matrix Expressions= 0 1 "
      " % % % // expressions used to compute the output of the ExpressionFilter (empty matrix for none)",
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
  // Is the WarningExpression parameter ok?
  Expression( Parameter( "WarningExpression" ) ).Evaluate( &preflightInput );
  // Are the entries of the Expressions parameter ok?
  int numRows = Parameter( "Expressions" )->NumRows(),
      numCols = Parameter( "Expressions" )->NumColumns();
  for( int row = 0; row < numRows; ++row )
    for( int col = 0; col < numCols; ++col )
      Expression( Parameter( "Expressions" )( row, col ) ).Evaluate( &preflightInput );

  // Request output signal properties:
  if( numRows == 0 || numCols == 0 )
    Output = Input;
  Output = SignalProperties( numRows, numCols );
}


void
ExpressionFilter::Initialize( const SignalProperties& Input,
                              const SignalProperties& Output )
{
  // Initialize the warning expression.
  mWarningExpression = Expression( Parameter( "WarningExpression" ) );
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
  // Check whether to issue the warning.
  if( mWarningExpression.Evaluate( &Input ) )
    bciout << Parameter( "WarningExpression" )
           << " evaluated to true"
           << endl;

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

