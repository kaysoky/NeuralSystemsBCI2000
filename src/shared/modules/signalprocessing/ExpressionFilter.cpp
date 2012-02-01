////////////////////////////////////////////////////////////////////////////////
//  $Id$
//  Author:      juergen.mellinger@uni-tuebingen.de
//  Description: A filter that uses arithmetic expressions to compute its
//    output.
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
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "ExpressionFilter.h"

using namespace std;

RegisterFilter( ExpressionFilter, 2.D2 );

ExpressionFilter::ExpressionFilter()
{
  BEGIN_PARAMETER_DEFINITIONS
    "Filtering string StartRunExpression= % "
      " % % % // expression executed on StartRun",
    "Filtering string StopRunExpression= % "
      " % % % // expression executed on StopRun",
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
  // Test whether configuration can be loaded.
  Expression startRunExpression( Parameter( "StartRunExpression" ) ),
             stopRunExpression( Parameter( "StopRunExpression" ) );
  ExpressionMatrix expressions;
  LoadExpressions( Parameter( "Expressions" ), expressions );
  VariableContainer variables;
  // Request output signal properties:
  Output = Input;
  size_t numRows = expressions.size(),
         numCols = numRows ? expressions[0].size() : 0;
  if( numCols != 0 )
    Output.SetChannels( numRows )
          .SetElements( numCols )
          .ElementUnit().SetGain( 1.0 ).SetOffset( 0.0 ).SetSymbol( "" );
  // Try evaluating expressions.
  startRunExpression.Compile( variables );
  startRunExpression.Evaluate();
  GenericSignal preflightInput( Input ), preflightOutput( Output );
  CompileExpressions( expressions, variables );
  EvaluateExpressions( expressions, &preflightInput, &preflightOutput );
  stopRunExpression.Compile( variables );
  stopRunExpression.Evaluate();
}


void
ExpressionFilter::Initialize( const SignalProperties&,
                              const SignalProperties& )
{
  mStartRunExpression = Expression( Parameter( "StartRunExpression" ) );
  mStopRunExpression = Expression( Parameter( "StopRunExpression" ) );
  LoadExpressions( Parameter( "Expressions" ), mExpressions );
  mVariables.clear();
  mStartRunExpression.Compile( mVariables );
  CompileExpressions( mExpressions, mVariables );
  mStopRunExpression.Compile( mVariables );
}


void
ExpressionFilter::StartRun()
{
  mStartRunExpression.Evaluate();
}

void
ExpressionFilter::StopRun()
{
  mStopRunExpression.Evaluate();
}

void
ExpressionFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  if( mExpressions.empty() )
    Output = Input;
  else
    EvaluateExpressions( mExpressions, &Input, &Output );
}

void
ExpressionFilter::LoadExpressions( const ParamRef& inParam, ExpressionMatrix& outMatrix )
{
  // Read an expression parameter into an expression matrix.
  outMatrix.clear();
  int numRows = inParam->NumRows(),
      numCols = inParam->NumColumns();
  if( numRows != 0 && numCols != 0 )
  {
    outMatrix.resize( numRows, vector<Expression>( numCols ) );
    for( int row = 0; row < numRows; ++row )
      for( int col = 0; col < numCols; ++col )
        outMatrix[ row ][ col ] = Expression( inParam( row, col ) );
  }
}


void
ExpressionFilter::CompileExpressions(
  ExpressionMatrix& inExpressions,
  VariableContainer& ioVariables )
{
  for( size_t i = 0; i < inExpressions.size(); ++i )
    for( size_t j = 0; j < inExpressions[i].size(); ++j )
      inExpressions[i][j].Compile( &ioVariables );
}

void
ExpressionFilter::EvaluateExpressions(
  ExpressionMatrix& inExpressions,
  const GenericSignal* inpSignal,
  GenericSignal* outpSignal )
{
  for( size_t i = 0; i < inExpressions.size(); ++i )
  {
    for( size_t j = 0; j < inExpressions[i].size(); ++j )
    {
      double result = inExpressions[i][j].Evaluate( inpSignal );
      if( outpSignal )
        ( *outpSignal )( i, j ) = result;
    }
  }
}
