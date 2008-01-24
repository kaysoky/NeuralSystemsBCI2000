//////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A simple expression parser for use within BCI2000.
//   Usage:
//   ex1 = Expression( "(1+ResultCode)==Signal(TargetCode,3)" );
//   double val1 = ex.Evaluate( inputSignalPtr );
//   ex2 = Expression( "TargetCode*(ResultCode==TargetCode)" );
//   double val2 = ex.Evaluate();
//   double val3 = Expression( Parameter( "ControlExpression" ) ).Evaluate();
//
//   Expressions may involve operators, numbers, BCI2000 states, and a GenericSignal
//   if given as an argument to the Evaluate() member function.
//   States are referred to by name, a signal is referred to in the form
//   "Signal(<channel>,<element>)".
//
//   Arithmetic operators: ^ unary- * / + -
//                         a^b evaluates to ::pow(a,b).
//   Comparison operators:  < > <= >= == != ~=
//   Logical operators:    ! ~ && ||
//                         ! and ~ are synonymous.
//   Operator precedence follows the order of appearance in the above list.
//   As usual, braces ( ) may be used to override operator precedence.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////////////
#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <sstream>
#include <string>
#include "Environment.h"
#include "GenericSignal.h"

#include "ExpressionParser.hpp"

class Expression;

namespace ExpressionParser
{
  int  yyparse( Expression* );
  int  yylex( YYSTYPE*, Expression* );
  void yyerror( Expression*, const char* );
}

class Expression : private EnvironmentBase
{
  friend int  ExpressionParser::yyparse( Expression* );
  friend int  ExpressionParser::yylex( YYSTYPE*, Expression* );
  friend void ExpressionParser::yyerror( Expression*, const char* );

 public:
  Expression()
    : mValue( 0 ), mpSignal( NULL )
    {}
  Expression( const std::string& s )
    : mExpression( s ), mValue( 0 ), mpSignal( NULL )
    {}
  Expression( const Expression& e )
    : mExpression( e.mExpression ), mValue( 0 ), mpSignal( NULL )
    {}
  ~Expression()
    {}
  const Expression& operator=( const Expression& e );

  double Evaluate( const GenericSignal* = NULL );

 private:
  double Signal( double, double ) const;
  void ReportError( const char* ) const;

  std::string            mExpression;
  const GenericSignal*   mpSignal;
  std::istringstream     mInput;
  double                 mValue;
};

#endif // EXPRESSION_H
