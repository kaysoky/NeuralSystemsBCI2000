////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Arithmetic expression type for the script interpreter.
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

#include "ExpressionType.h"
#include "CommandInterpreter.h"
#include "Expression.h"
#include "BCIException.h"

using namespace std;
using namespace Interpreter;

class InterpreterExpression : public Expression
{
 public:
  InterpreterExpression( CommandInterpreter& inInterpreter )
    : Expression( inInterpreter.GetRemainder() ),
      mrInterpreter( inInterpreter )
    { ThrowOnError( true ); }
  Node* Variable( const string& );
  Node* VariableAssignment( const string&, Node* );
  Node* State( const string& name )
    { return new StateNode( mrInterpreter, name ); }
  Node* StateAssignment( const string& name, Node* rhs )
    { return new StateAssignmentNode( mrInterpreter, name, rhs ); }
  static bool StateExists( CommandInterpreter&, const string& );
  static void AssertState( CommandInterpreter&, const string& );

 private:
  CommandInterpreter& mrInterpreter;

  class StateNode : public Node
  {
   public:
    StateNode( CommandInterpreter& interpreter, const string& name )
    : mrInterpreter( interpreter ), mName( name ) {}
   protected:
    double OnEvaluate();
   private:
    CommandInterpreter& mrInterpreter;
    string mName;
  };

  class StateAssignmentNode : public Node
  {
   public:
    StateAssignmentNode( CommandInterpreter& interpreter, const string& name, Node* rhs )
    : mrInterpreter( interpreter ), mName( name ) { Add( rhs ); }
   protected:
    double OnEvaluate();
   private:
    CommandInterpreter& mrInterpreter;
    string mName;
  };
};

ExpressionType ExpressionType::sInstance;
const ObjectType::MethodEntry ExpressionType::sMethodTable[] =
{
  METHOD( Evaluate ),
  METHOD( Clear ),
  END
};

bool
ExpressionType::Evaluate( CommandInterpreter& inInterpreter )
{
  InterpreterExpression exp( inInterpreter );
  exp.Compile( inInterpreter.ExpressionVariables() );
  inInterpreter.Out() << exp.Execute( &inInterpreter.StateMachine().ControlSignal() );
  return true;
}

bool
ExpressionType::Clear( CommandInterpreter& inInterpreter )
{
  string token = inInterpreter.GetToken();
  if( ::stricmp( token.c_str(), "variables" ) )
  {
    inInterpreter.Unget();
    return false;
  }
  inInterpreter.ExpressionVariables().clear();
  return true;
}

// InterpreterExpression definitions
bool
InterpreterExpression::StateExists( CommandInterpreter& inInterpreter, const string& inName )
{
  return inInterpreter.StateMachine().States().Exists( inName );
}

void
InterpreterExpression::AssertState( CommandInterpreter& inInterpreter, const string& inName )
{
  if( !StateExists( inInterpreter, inName ) )
    throw bciexception_( "State \"" << inName << "\" does not exist" );
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
  if( StateExists( mrInterpreter, inName ) )
    return new StateAssignmentNode( mrInterpreter, inName, inRhs );
  return ArithmeticExpression::VariableAssignment( inName, inRhs );
}

double
InterpreterExpression::StateNode::OnEvaluate()
{
  Lock<StateMachine> lock( mrInterpreter.StateMachine() );
  AssertState( mrInterpreter, mName );
  return mrInterpreter.StateMachine().GetStateValue( mName.c_str() );
}

double
InterpreterExpression::StateAssignmentNode::OnEvaluate()
{
  double rhs = mChildren[0]->Evaluate();
  Lock<StateMachine> lock( mrInterpreter.StateMachine() );
  AssertState( mrInterpreter, mName );
  if( !mrInterpreter.StateMachine().SetStateValue( mName.c_str(), static_cast<State::ValueType>( rhs ) ) )
    throw bciexception_( "Could not set value of state " << mName );
  return rhs;
}

