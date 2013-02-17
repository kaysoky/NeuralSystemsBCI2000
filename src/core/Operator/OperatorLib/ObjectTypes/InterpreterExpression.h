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
#ifndef INTERPRETER_EXPRESSION_H
#define INTERPRETER_EXPRESSION_H

#include "Expression.h"

class CommandInterpreter;

class InterpreterExpression : public Expression
{
 public:
  InterpreterExpression( CommandInterpreter&, const std::string& inExpr = "" );
  
  InterpreterExpression& AllowAssignment()
    { mAllowAssignment = true; return *this; }
  InterpreterExpression& ForbidAssignment()
    { mAllowAssignment = false; return *this; }
  
  double Evaluate() const
    { return Execute(); }
  double Execute() const;

 protected:
  Node* Variable( const std::string& );
  Node* VariableAssignment( const std::string&, Node* );

  Node* State( const std::string& );
  Node* StateAssignment( const std::string&, Node* );
  
  void AssertAssignment() const;

  static bool StateExists( CommandInterpreter&, const std::string& );
  static void AssertState( CommandInterpreter&, const std::string& );

 private:
  CommandInterpreter& mrInterpreter;
  bool mAllowAssignment;

  class StateNode : public Node
  {
   public:
    StateNode( CommandInterpreter& interpreter, const std::string& name )
    : mrInterpreter( interpreter ), mName( name ) {}
   protected:
    double OnEvaluate();
   private:
    CommandInterpreter& mrInterpreter;
    std::string mName;
  };

  class StateAssignmentNode : public Node
  {
   public:
    StateAssignmentNode( CommandInterpreter& interpreter, const std::string& name, Node* rhs )
    : mrInterpreter( interpreter ), mName( name ) { Add( rhs ); }
   protected:
    double OnEvaluate();
   private:
    CommandInterpreter& mrInterpreter;
    std::string mName;
  };
};

#endif // INTERPRETER_EXPRESSION_H
