//////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A simple expression parser for use within BCI2000.
//   ArithmeticExpression provides expression parsing; its Expression
//   descendant, in addition, allows access to State and Signal values.
//   For details about expression syntax, see Expression.h.
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
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARITHMETIC_EXPRESSION_H
#define ARITHMETIC_EXPRESSION_H

#include <sstream>
#include <string>
#include <list>
#include <map>

#include "ExpressionNodes.h"
#include "ExpressionParser.hpp"

class ArithmeticExpression;

namespace ExpressionParser
{
  int  yyparse( ArithmeticExpression* );
  int  yylex( YYSTYPE*, ArithmeticExpression* );
  void yyerror( ArithmeticExpression*, const char* );
}

class ArithmeticExpression
{
  friend int  ExpressionParser::yyparse( ArithmeticExpression* );
  friend int  ExpressionParser::yylex( YYSTYPE*, ArithmeticExpression* );
  friend void ExpressionParser::yyerror( ArithmeticExpression*, const char* );

 public:
  typedef std::map<std::string, double> VariableContainer;
  static const VariableContainer Constants;

  struct Context
  {
    Context( VariableContainer* pVariables = NULL, const VariableContainer* pConstants = &Constants )
    : variables( pVariables ), constants( pConstants ) {}
    Context( VariableContainer& rVariables, const VariableContainer& rConstants = Constants )
    : variables( &rVariables ), constants( &rConstants ) {}

    VariableContainer* variables;
    const VariableContainer* constants;
  };

  ArithmeticExpression( const std::string& = "" );
  ArithmeticExpression( const ArithmeticExpression& );
  virtual ~ArithmeticExpression();

  const ArithmeticExpression& operator=( const ArithmeticExpression& );

  bool ThrowOnError() const
    { return mThrowOnError; }
  ArithmeticExpression& ThrowOnError( bool inThrow )
    { mThrowOnError = inThrow; return *this; }
  const std::string& AsString() const
    { return mExpression; }

  enum { none, attempted, success };
  int CompilationState() const
    { return mCompilationState; }

  bool IsValid( const Context& = Context() );
  bool Compile( const Context& = Context() );
  double Evaluate();
  double Execute()
    { return Evaluate(); }

 protected:
  typedef ExpressionParser::Node Node;
  typedef ExpressionParser::AddressNode AddressNode;
  typedef ExpressionParser::NodeList NodeList;

  void Add( Node* );

  virtual Node* Variable( const std::string& name );
  virtual Node* VariableAssignment( const std::string& name, Node* );

  virtual Node* Function( const std::string& name, const NodeList& );
  virtual Node* MemberFunction( const std::string& object, const std::string& function, const NodeList& );

  virtual Node* Signal( AddressNode*, AddressNode* );
  virtual Node* State( const std::string& );
  virtual Node* StateAssignment( const std::string&, Node* );

  std::ostream& Errors()
    { return mErrors; }

 private:
  typedef ExpressionParser::StringNode StringNode;
  typedef ExpressionParser::ObjPtr ObjPtr;

  NodeList* MakeList( Node* = 0 );

  Node* MakeConstant( double );
  Node* MakeVariable( StringNode* );
  Node* MakeVariableAssignment( StringNode*, Node* );

  Node* MakeFunction( double (*)() );
  Node* MakeFunction( double (*)( double ), Node* );
  Node* MakeFunction( double (*)( double, double ), Node*, Node* );
  Node* MakeFunction( double (*)( double, double, double ), Node*, Node*, Node* );
  Node* MakeFunction( StringNode*, const NodeList& );
  Node* MakeMemberFunction( StringNode*, StringNode*, const NodeList& );

  Node* MakeAddress( Node*, Node* = 0 );
  Node* MakeSignal( Node*, Node* );
  Node* MakeState( StringNode* );
  Node* MakeStateAssignment( StringNode*, Node* );

  double DoEvaluate();
  void ReportErrors();

  bool Parse();
  void CollectGarbage();

  Node* Track( Node* );
  std::list<ObjPtr> mParserObjects;

  std::string        mExpression;
  std::istringstream mInput;
  std::ostringstream mErrors;
  Context            mContext;
  bool               mThrowOnError;
  int                mCompilationState;
  NodeList           mStatements;
};

std::ostream& operator<<( std::ostream&, const ArithmeticExpression::VariableContainer& );

#endif // ARITHMETIC_EXPRESSION_H

