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
  typedef ArithmeticExpression::VariableContainer VariableContainer;

  Expression()
    : mpSignal( NULL ),
      mpVariables( NULL ),
      mOptionalAccess( false ),
      mDefaultValue( 0 ),
      mAllowStateAssignment( false )
    {}
  Expression( const std::string& s )
    : ArithmeticExpression( s ),
      mpSignal( NULL ),
      mpVariables( NULL ),
      mOptionalAccess( false ),
      mDefaultValue( 0 ),
      mAllowStateAssignment( false )
    {}
  ~Expression()
    {}

  Expression& SetOptionalAccess( State::ValueType inDefault = 0 );
  Expression& ClearOptionalAccess();

  bool   IsValid( const GenericSignal* = NULL, const VariableContainer* = NULL );
  double Evaluate( const GenericSignal* = NULL, VariableContainer* = NULL );

 protected:
  virtual double Variable( const std::string& name );
  virtual double Signal( const std::string&, const std::string& );
  virtual double State( const std::string& );
  virtual void   StateAssignment( const std::string&, double );

  const GenericSignal* mpSignal;
  const VariableContainer* mpVariables;
  bool                 mOptionalAccess;
  State::ValueType     mDefaultValue;
  bool                 mAllowStateAssignment;
};

#endif // EXPRESSION_H

