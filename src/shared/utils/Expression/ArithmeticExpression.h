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
#ifndef ARITHMETIC_EXPRESSION_H
#define ARITHMETIC_EXPRESSION_H

#include <sstream>
#include <string>
#include <list>

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
  ArithmeticExpression();
  ArithmeticExpression( const std::string& );
  ArithmeticExpression( const ArithmeticExpression& );
  virtual ~ArithmeticExpression();

  const ArithmeticExpression& operator=( const ArithmeticExpression& e );

  bool   IsValid();
  double Evaluate();

 protected:
  virtual double State( const std::string& );
  virtual double Signal( const std::string&, const std::string& );
  std::ostream& Errors()
    { return mErrors; }

 private:
  void Parse();
  void Cleanup();
  std::string* AllocateCopy( const std::string& );

  std::string        mExpression;
  std::istringstream mInput;
  std::ostringstream mErrors;
  double             mValue;

  typedef std::list<std::string*> StringContainer;
  StringContainer mAllocatedStrings;
};

#endif // ARITHMETIC_EXPRESSION_H

