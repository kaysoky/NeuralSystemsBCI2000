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
#include <vector>
#include <map>

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

  const ArithmeticExpression& operator=( const ArithmeticExpression& );

  typedef std::map<std::string, double> VariableContainer;
  static const VariableContainer Constants;

  bool   IsValid( const VariableContainer* = NULL, const VariableContainer* = &Constants );
  double Evaluate( VariableContainer* = NULL, const VariableContainer* = &Constants );

 protected:
  typedef std::vector<double> ArgumentList;

  virtual double Variable( const std::string& name );
  virtual void   VariableAssignment( const std::string& name, double value );

  virtual double MemberVariable( double objectRef, const std::string& name );
  virtual void   MemberVariableAssignment( double objectRef, const std::string& name, double value );

  virtual double Function( const std::string& name, const ArgumentList& );
  virtual double MemberFunction( double objectRef, const std::string& name, const ArgumentList& );

  virtual double Signal( const std::string&, const std::string& );
  virtual double State( const std::string& );
  virtual void   StateAssignment( const std::string&, double );

  std::ostream& Errors()
    { return mErrors; }

 private:
  void Parse();
  void Cleanup();

  std::string        mExpression;
  std::istringstream mInput;
  std::ostringstream mErrors;
  double             mValue;
  VariableContainer* mpVariables;
  const VariableContainer* mpConstants;

  // Memory management.
  template<typename T>
  T*
  Allocate( const T* inpOriginal = NULL )
  {
    Pointer<T>* p = new Pointer<T>( inpOriginal ? new T( *inpOriginal ) : new T );
    mAllocations.push_back( p );
    return *p;
  }
  template<typename T>
  T*
  Allocate( const T& inOriginal )
  {
    return Allocate( &inOriginal );
  }

  struct StoredPointer
  { virtual void Delete() = 0; };

  template<typename T>
  class Pointer : public StoredPointer
  {
   public:
    Pointer( T* inPtr = 0 ) : mPtr( inPtr ) {}
    virtual void Delete() { delete mPtr; mPtr = NULL; }
    operator T*() const { return mPtr; }
   private:
    T* mPtr;
  };

  typedef std::list<StoredPointer*> PointerContainer;
  PointerContainer mAllocations;
};

#endif // ARITHMETIC_EXPRESSION_H

