////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        StateTransform.cpp
// Date:        Jan 13, 2006
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that transforms state values according to rules.
//              Whenever a given state's value changes, it replaces the new
//              value by a user-defined expression.
// $Log$
// Revision 1.1  2006/01/17 18:16:05  mellinger
// Initial version.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "StateTransform.h"

using namespace std;

RegisterFilter( StateTransform, 4 );


StateTransform::StateTransform()
{
 BEGIN_PARAMETER_DEFINITIONS
  "StateTransform matrix StateTransforms= 1 2 ResultCode (ResultCode>0)*(1+(Signal(0,0)>0)) % % % "
    "// Replace the state from the first column with the expression from the second",
 END_PARAMETER_DEFINITIONS
}


StateTransform::~StateTransform()
{
}


void
StateTransform::Preflight( const SignalProperties& input,
                                 SignalProperties& output ) const
{
  GenericSignal preflightSignal( input );
  ParamRef StateTransforms = Parameter( "StateTransforms" );
  for( size_t i = 0; i < StateTransforms->GetNumRows(); ++i )
  {
    State( StateTransforms( i, 0 ) );
    Expression( StateTransforms( i, 1 ) ).Evaluate( &preflightSignal );
  }
  // Request output signal properties:
  output = input;
}


void
StateTransform::Initialize2( const SignalProperties& input,
                             const SignalProperties& output )
{
  mStateNames.clear();
  mExpressions.clear();
  mPreviousInputStateValues.clear();
  mPreviousOutputStateValues.clear();

  ParamRef StateTransforms = Parameter( "StateTransforms" );
  for( size_t i = 0; i < StateTransforms->GetNumRows(); ++i )
  {
    mStateNames.push_back( string( StateTransforms( i, 0 ) ) );
    mExpressions.push_back( Expression( StateTransforms( i, 1 ) ) );
  }
  mPreviousInputStateValues.resize( mStateNames.size(), -1 );
  mPreviousOutputStateValues.resize( mStateNames.size(), -1 );
}


void
StateTransform::Process( const GenericSignal* input, GenericSignal* output )
{
  for( size_t i = 0; i < mStateNames.size(); ++i )
  {
    int currentInputStateValue = State( mStateNames[ i ].c_str() );
    if( currentInputStateValue != mPreviousInputStateValues[ i ] )
      mPreviousOutputStateValues[ i ] = mExpressions[ i ].Evaluate( input );
    State( mStateNames[ i ].c_str() ) = mPreviousOutputStateValues[ i ];
    mPreviousInputStateValues[ i ] = currentInputStateValue;
  }
  *output = *input;
}
