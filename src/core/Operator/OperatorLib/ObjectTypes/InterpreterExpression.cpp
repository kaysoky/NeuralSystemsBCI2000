////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: An expression that is linked to a command interpreter object.
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

#include "InterpreterExpression.h"
#include "CommandInterpreter.h"
#include "StateMachine.h"

using namespace std;

InterpreterExpression::InterpreterExpression( CommandInterpreter& inInterpreter, const string& inExpr )
: Expression( inExpr.empty() ? inInterpreter.GetRemainingTokens() : inExpr ),
  mrInterpreter( inInterpreter ),
  mAllowAssignment( true )
{
  ThrowOnError( true );
  Compile( inInterpreter.ExpressionVariables() );
}

double
InterpreterExpression::Execute() const
{
  return Expression::Execute( &mrInterpreter.StateMachine().ControlSignal() );
}

bool
InterpreterExpression::StateExists( CommandInterpreter& inInterpreter, const string& inName )
{
  return inInterpreter.StateMachine().States().Exists( inName );
}

void
InterpreterExpression::AssertState( CommandInterpreter& inInterpreter, const string& inName )
{
  if( !StateExists( inInterpreter, inName ) )
    throw bciexception( "State \"" << inName << "\" does not exist" );
}

void
InterpreterExpression::AssertAssignment() const
{
  if( !mAllowAssignment )
    throw bciexception( "Assignment not supported in current context" );
}

Expression::Node*
InterpreterExpression::Variable( const string& inName )
{
  if( StateExists( mrInterpreter, inName ) )
    return new StateNode( mrInterpreter, inName );
  return ArithmeticExpression::Variable( inName );
}

Expression::Node*
InterpreterExpression::VariableAssignment( const string& inName, Node* inRhs )
{
  AssertAssignment();
  if( StateExists( mrInterpreter, inName ) )
    return new StateAssignmentNode( mrInterpreter, inName, inRhs );
  return ArithmeticExpression::VariableAssignment( inName, inRhs );
}

Expression::Node*
InterpreterExpression::State( const std::string& inName )
{
  return new StateNode( mrInterpreter, inName );
}

Expression::Node*
InterpreterExpression::StateAssignment( const std::string& inName, Node* inRhs )
{
  AssertAssignment();
  return new StateAssignmentNode( mrInterpreter, inName, inRhs );
}

// Nodes
double
InterpreterExpression::StateNode::OnEvaluate()
{
  Lock lock( mrInterpreter.StateMachine() );
  AssertState( mrInterpreter, mName );
  return mrInterpreter.StateMachine().GetStateValue( mName.c_str() );
}

double
InterpreterExpression::StateAssignmentNode::OnEvaluate()
{
  double rhs = mChildren[0]->Evaluate();
  Lock lock( mrInterpreter.StateMachine() );
  AssertState( mrInterpreter, mName );
  if( !mrInterpreter.StateMachine().SetStateValue( mName.c_str(), static_cast<State::ValueType>( rhs ) ) )
    throw bciexception( "Could not set value of state " << mName );
  return rhs;
}
