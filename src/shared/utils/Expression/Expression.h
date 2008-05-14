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

#include "ArithmeticExpression.h"
#include "Environment.h"
#include "GenericSignal.h"

#include "ExpressionParser.hpp"

class Expression : public ArithmeticExpression, private Environment
{
 public:
  Expression()
    : mpSignal( NULL )
    {}
  Expression( const std::string& s )
    : ArithmeticExpression( s ), mpSignal( NULL )
    {}
  Expression( const Expression& e )
    : ArithmeticExpression( e ), mpSignal( NULL )
    {}
  ~Expression()
    {}
  const Expression& operator=( const Expression& e );

  bool   IsValid( const GenericSignal* = NULL );
  double Evaluate( const GenericSignal* = NULL );

 private:
  virtual double State( const char* );
  virtual double Signal( const std::string&, const std::string& );

  const GenericSignal* mpSignal;
};

#endif // EXPRESSION_H

