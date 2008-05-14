//////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A simple expression parser for use within BCI2000.
//   ArithmeticExpression provides expression parsing; its Expression
//   descendant, in addition, allows access to State and Signal values.
//   For details about expression syntax, see Expression.h.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARITHMETIC_EXPRESSION_H
#define ARITHMETIC_EXPRESSION_H

#include <sstream>
#include <string>

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
  ~ArithmeticExpression()
    {}
  const ArithmeticExpression& operator=( const ArithmeticExpression& e );

  bool   IsValid();
  double Evaluate();

 protected:
  virtual double State( const char* );
  virtual double Signal( const std::string&, const std::string& );
  std::ostream& Errors()
    { return mErrors; }

 private:
  void Parse();

  std::string        mExpression;
  std::istringstream mInput;
  std::ostringstream mErrors;
  double             mValue;
};

#endif // ARITHMETIC_EXPRESSION_H

