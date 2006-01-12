////////////////////////////////////////////////////////////////////////////////
//  $Id$
//  File:        ExpressionFilter.cpp
//
//  Description: A tutorial filter demonstrating
//               the use of boolean and arithmetic expressions in parameters.
//               -- Each time the WarningExpression expression evaluates to true,
//                  a warning is issued.
//               -- Filter output is determined by the expressions given in the
//                  Expressions matrix.
//
//  Date:        Aug 12, 2005
//  Author:      juergen.mellinger@uni-tuebingen.de
//  $Log$
//  Revision 1.2  2006/01/12 20:40:35  mellinger
//  Added CVS id and log to comment.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "ExpressionFilter.h"

using namespace std;

RegisterFilter( ExpressionFilter, 2.D1 );

ExpressionFilter::ExpressionFilter()
{
  BEGIN_PARAMETER_DEFINITIONS
    "Filtering string WarningExpression= ResultCode==TargetCode "
      " % % % // expression that results in a warning when it evaluates to true",
    "Filtering matrix Expressions= 2 2 "
      " Signal(0,0)^2*(ResultCode==0) Signal(0,1)^2*(ResultCode==0)"
      " Signal(1,0)^2*(ResultCode==0) Signal(1,1)^2*(ResultCode==0)"
      " % % % // expressions used to compute the output of the ExpressionFilter",
  END_PARAMETER_DEFINITIONS
}


ExpressionFilter::~ExpressionFilter()
{
}


void
ExpressionFilter::Preflight( const SignalProperties& inputProperties,
                                   SignalProperties& outputProperties ) const
{
  // Evaluate all expressions to check for errors.
  GenericSignal preflightInput( inputProperties );
  // Is the WarningExpression parameter ok?
  Expression( Parameter( "WarningExpression" ) ).Evaluate( &preflightInput );
  // Are the entries of the Expressions parameter ok?
  size_t numRows = Parameter( "Expressions" )->GetNumRows(),
         numCols = Parameter( "Expressions" )->GetNumColumns();
  for( size_t row = 0; row < numRows; ++row )
    for( size_t col = 0; col < numCols; ++col )
      Expression( Parameter( "Expressions", row, col ) ).Evaluate( &preflightInput );

  // Request output signal properties:
  outputProperties = SignalProperties( numRows, numCols );
}


void
ExpressionFilter::Initialize2( const SignalProperties& inputProperties,
                               const SignalProperties& outputProperties )
{
  // Initialize the warning expression.
  mWarningExpression = Expression( Parameter( "WarningExpression" ) );
  // Read the expression matrix parameter into the mExpressions array.
  mExpressions.clear();
  size_t numRows = Parameter( "Expressions" )->GetNumRows(),
         numCols = Parameter( "Expressions" )->GetNumColumns();
  mExpressions.resize( numRows, vector<Expression>( numCols ) );
  for( size_t row = 0; row < numRows; ++row )
    for( size_t col = 0; col < numCols; ++col )
      mExpressions[ row ][ col ] = Expression( Parameter( "Expressions", row, col ) );
}


void
ExpressionFilter::Process( const GenericSignal* input, GenericSignal* output )
{
  // Check whether to issue the warning.
  if( mWarningExpression.Evaluate( input ) )
    bciout << string( Parameter( "WarningExpression" ) )
           << " evaluated to true"
           << endl;

  // Use the expressions to compute the output signal.
  for( size_t channel = 0; channel < mExpressions.size(); ++channel )
    for( size_t sample = 0; sample < mExpressions[ channel ].size(); ++sample )
      ( *output )( channel, sample ) = mExpressions[ channel ][ sample ].Evaluate( input );
}

