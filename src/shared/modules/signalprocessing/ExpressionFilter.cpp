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
    "Filtering matrix StartRunExpressions= 0 1 "
      " % % % // expressions executed on StartRun"
      " (rows are channels; empty matrix for none; single row and column for global expression)",
    "Filtering matrix StopRunExpressions= 0 1 "
      " % % % // expressions executed on StopRun"
      " (rows are channels; empty matrix for none; single row and column for global expression)",
    "Filtering matrix Expressions= 0 1 "
      " % % % // expressions used to compute the output of the ExpressionFilter"
      " (rows are channels; empty matrix for none)",
    "Filtering int ShareExpressionVariables= 1 "
      " 1 0 1 // share variables between expressions (boolean)",
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
  ExpressionMatrix expressions,
                   startRunExpressions,
                   stopRunExpressions;
  VariablesMatrix  variables;
  LoadConfig( expressions, startRunExpressions, stopRunExpressions, variables );
  // Request output signal properties:
  Output = Input;
  size_t numRows = expressions.size(),
         numCols = numRows ? expressions[0].size() : 0;
  if( numCols != 0 )
    Output.SetChannels( numRows )
          .SetElements( numCols )
          .ElementUnit().SetGain( 1.0 ).SetOffset( 0.0 ).SetSymbol( "" );
  // Try evaluating expressions.
  EvaluateExpressions( startRunExpressions, variables );
  GenericSignal preflightInput( Input ), preflightOutput( Output );
  EvaluateExpressions( expressions, variables, &preflightInput, &preflightOutput );
  EvaluateExpressions( stopRunExpressions, variables );
}


void
ExpressionFilter::Initialize( const SignalProperties&,
                              const SignalProperties& )
{
  LoadConfig( mExpressions, mStartRunExpressions, mStopRunExpressions, mVariables );
}


void
ExpressionFilter::StartRun()
{
  EvaluateExpressions( mStartRunExpressions, mVariables );
}

void
ExpressionFilter::StopRun()
{
  EvaluateExpressions( mStopRunExpressions, mVariables );
}

void
ExpressionFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  if( mExpressions.empty() )
    Output = Input;
  else
    EvaluateExpressions( mExpressions, mVariables, &Input, &Output );
}

void
ExpressionFilter::LoadConfig(
  ExpressionMatrix& outExpressions,
  ExpressionMatrix& outStartRunExpressions,
  ExpressionMatrix& outStopRunExpressions,
  VariablesMatrix& outVariables ) const
{
  const ParamRef& Expressions = Parameter( "Expressions" ),
                & StartRunExpressions = Parameter( "StartRunExpressions" ),
                & StopRunExpressions = Parameter( "StopRunExpressions" );
  LoadExpressions( Expressions, outExpressions );
  LoadExpressions( StartRunExpressions, outStartRunExpressions );
  LoadExpressions( StopRunExpressions, outStopRunExpressions );

  int varRows = 1,
      varCols = 1;
  if( Parameter( "ShareExpressionVariables" ) == 0 )
  {
    varRows = max( varRows, Expressions->NumRows() );
    varRows = max( varRows, StartRunExpressions->NumRows() );
    varRows = max( varRows, StopRunExpressions->NumRows() );
    varCols = max( varCols, Expressions->NumColumns() );
    varCols = max( varCols, StartRunExpressions->NumColumns() );
    varCols = max( varCols, StopRunExpressions->NumColumns() );
  }
  outVariables.clear();
  outVariables.resize( varRows, vector<Expression::VariableContainer>( varCols ) );
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
ExpressionFilter::EvaluateExpressions(
  ExpressionMatrix& inExpressions,
  VariablesMatrix& ioVariables,
  const GenericSignal* inpSignal,
  GenericSignal* outpSignal )
{
  int numRows = max( inExpressions.size(), ioVariables.size() );
  for( int i = 0; i < numRows; ++i )
  {
    int ei = min<int>( i, inExpressions.size() - 1 ),
        vi = min<int>( i, ioVariables.size() - 1 );
    if( ei >= 0 && vi >= 0 )
    {
      int numCols = max( inExpressions[ei].size(), ioVariables[vi].size() );
      for( int j = 0; j < numCols; ++j )
      {
        int ej = min<int>( j, inExpressions[ei].size() - 1 ),
            vj = min<int>( j, ioVariables[vi].size() - 1 );
        if( ej >= 0 && vj >= 0 )
        {
          double result = inExpressions[ei][ej].Evaluate( inpSignal, &ioVariables[vi][vj] );
          if( outpSignal )
            ( *outpSignal )( i, j ) = result;
        }
      }
    }
  }
}
