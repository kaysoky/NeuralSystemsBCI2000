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
#include <map>
#include <set>

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

  bool IsValid( const Context& = Context() );
  bool Compile( const Context& = Context() );
  double Evaluate();

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
  bool Parse();
  double DoEvaluate();
  void ReportErrors();
  void Cleanup();
  void ClearStatements();

  std::string        mExpression;
  std::istringstream mInput;
  std::ostringstream mErrors;
  NodeList           mStatements;
  Context            mContext;
  enum { none, attempted, success } mCompilationState;

  // Memory management.
  template<typename T>
  T*
  Track( T* inNew, Node* inOld1 = NULL, Node* inOld2 = NULL, Node* inOld3 = NULL )
  {
    if( inNew )
    {
      mAllocations.insert( Pointer<T>( inNew ) );
      if( inOld1 )
        mAllocations.erase( Pointer<Node>( inOld1 ) );
      if( inOld2 )
        mAllocations.erase( Pointer<Node>( inOld2 ) );
      if( inOld3 )
        mAllocations.erase( Pointer<Node>( inOld3 ) );
    }
    return inNew;
  }

  template<typename T>
  T*
  Track( T* inNew, NodeList* inNodes )
  {
    if( inNew )
    {
      for( NodeList::iterator i = inNodes->begin(); i != inNodes->end(); ++i )
        mAllocations.erase( Pointer<Node>( *i ) );
      mAllocations.erase( Pointer<NodeList>( inNodes ) );
      delete inNodes;
      mAllocations.insert( Pointer<T>( inNew ) );
    }
    return inNew;
  }

  class StoredPointer
  {
   public:
    StoredPointer( void* p = NULL ) : mp( p ) {}
    bool operator<( const StoredPointer& p ) const { return Pointer() < p.Pointer(); }
    void* Pointer() const { return mp; }
    virtual void Delete() {};
   private:
    void* mp;
  };

  template<typename T>
  class Pointer : public StoredPointer
  {
   public:
    Pointer( T* inPtr ) : StoredPointer( inPtr ) {}
    virtual void Delete() { delete reinterpret_cast<T*>( StoredPointer::Pointer() ); }
  };

  typedef std::set<StoredPointer> PointerContainer;
  PointerContainer mAllocations;
};

#endif // ARITHMETIC_EXPRESSION_H

