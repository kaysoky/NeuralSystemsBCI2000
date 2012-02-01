%output="ExpressionParser.cpp"
%defines
%{
////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: Bison grammar file for a simple expression parser.
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
////////////////////////////////////////////////////////////////////////
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <cmath>
#include <cstring>
#include <cstdio>
#include "ArithmeticExpression.h"
#include "BCIError.h"

// Disable compiler warnings for generated code.
#ifdef __BORLANDC__
# pragma warn -8004
#elif defined( _MSC_VER )
# pragma warning (disable:4065)
#endif

namespace ExpressionParser
{

// Addresses of built-in operators cannot be taken, so we need to 
// provide them as functions.
double Plus( double a, double b )                  { return a + b; }
double Minus( double a, double b )                 { return a - b; }
double Minus( double a )                           { return -a; }
double Multiply( double a, double b )              { return a * b; }
double Divide( double a, double b )                { return a / b; }
double And( double a, double b )                   { return a && b; }
double Or( double a, double b )                    { return a || b; }
double Equal( double a, double b )                 { return a == b; }
double NotEqual( double a, double b )              { return a != b; }
double Greater( double a, double b )               { return a > b; }
double GreaterEqual( double a, double b )          { return a >= b; }
double Less( double a, double b )                  { return a < b; }
double LessEqual( double a, double b )             { return a <= b; }
double Not( double a )                             { return !a; }
double Conditional( double a, double b, double c ) { return a ? b : c; }

} // namespace ExpressionParser

using namespace std;

%}

%pure-parser
%error-verbose
%parse-param { ::ArithmeticExpression* p }
%lex-param   { ::ArithmeticExpression* p }
%union
{
  double                         value;
  std::string*                   str;
  ExpressionParser::Node*        node;
  ExpressionParser::AddressNode* address;
  ExpressionParser::NodeList*    list;
}


%{
namespace ExpressionParser
{
%}

/* Bison declarations.  */
%token <value>   NUMBER
%token <str>     NAME STATE SIGNAL_ /* avoid interference with Qt's SIGNAL macro */
%left '?' ':'
%left '&' '|'
%left '=' '~' '!' '>' '<'
%left '-' '+'
%left '*' '/'
%left NEG     /* negation--unary minus */
%right '^'    /* exponentiation */
%left '.'     /* element operator */
%left ';'

%type <node>  input statements statement exp assignment
%type <str>   quoted statename
%type <address> addr
%type <list>  args list

%% /* The grammar follows.  */
input: /* empty */                   { $$ = NULL; }
     | statements                    { $$ = $1; }
;

statements:
       statement                     { $$ = $1; p->Add( $1 ); p->Track( reinterpret_cast<Node*>( NULL ), $1 ); }
     | statements ';' statement      { $$ = $3; p->Add( $3 ); p->Track( reinterpret_cast<Node*>( NULL ), $3 ); }
     | statements ';'                { $$ = $1; }
     | ';'                           { $$ = NULL; }
;

statement:     
       exp                           { $$ = $1; }
     | assignment                    { $$ = $1; }
;

exp:   NUMBER                        { $$ = new ConstantNode( $1 );                             p->Track( $$ ); }
     | exp '+' exp                   { $$ = new FunctionNode<2>( true, &Plus,         $1, $3 ); p->Track( $$, $1, $3 ); }
     | exp '-' exp                   { $$ = new FunctionNode<2>( true, &Minus,        $1, $3 ); p->Track( $$, $1, $3 ); }
     | exp '*' exp                   { $$ = new FunctionNode<2>( true, &Multiply,     $1, $3 ); p->Track( $$, $1, $3 ); }
     | exp '/' exp                   { $$ = new FunctionNode<2>( true, &Divide,       $1, $3 ); p->Track( $$, $1, $3 ); }
     | '-' exp %prec NEG             { $$ = new FunctionNode<1>( true, &Minus,        $2     ); p->Track( $$, $2 ); }
     | exp '^' exp                   { $$ = new FunctionNode<2>( true, &pow,          $1, $3 ); p->Track( $$, $1, $3 ); }
     | exp '&' '&' exp               { $$ = new FunctionNode<2>( true, &And,          $1, $4 ); p->Track( $$, $1, $4 ); }
     | exp '|' '|' exp               { $$ = new FunctionNode<2>( true, &Or,           $1, $4 ); p->Track( $$, $1, $4 ); }
     | exp '=' '=' exp               { $$ = new FunctionNode<2>( true, &Equal,        $1, $4 ); p->Track( $$, $1, $4 ); }
     | exp '!' '=' exp               { $$ = new FunctionNode<2>( true, &NotEqual,     $1, $4 ); p->Track( $$, $1, $4 ); }
     | exp '~' '=' exp               { $$ = new FunctionNode<2>( true, &NotEqual,     $1, $4 ); p->Track( $$, $1, $4 ); }
     | exp '>' exp                   { $$ = new FunctionNode<2>( true, &Greater,      $1, $3 ); p->Track( $$, $1, $3 ); }
     | exp '<' exp                   { $$ = new FunctionNode<2>( true, &Less,         $1, $3 ); p->Track( $$, $1, $3 ); }
     | exp '>' '=' exp               { $$ = new FunctionNode<2>( true, &GreaterEqual, $1, $4 ); p->Track( $$, $1, $4 ); }
     | exp '<' '=' exp               { $$ = new FunctionNode<2>( true, &LessEqual,    $1, $4 ); p->Track( $$, $1, $4 ); }
     | '~' exp %prec NEG             { $$ = new FunctionNode<1>( true, &Not,          $2     ); p->Track( $$, $2 ); }
     | '!' exp %prec NEG             { $$ = new FunctionNode<1>( true, &Not,          $2     ); p->Track( $$, $2 ); }
     | '(' exp ')'                   { $$ = $2; }
     | exp '?' exp ':' exp           { $$ = new FunctionNode<3>( true, &Conditional, $1, $3, $5 ); p->Track( $$, $1, $3, $5 ); }
     
     | NAME                          { $$ = p->Variable( *$1 );                 p->Track( $$ ); }
     | NAME '(' args ')'             { $$ = p->Function( *$1, *$3 );            p->Track( $$, $3 ); }
     | NAME '.' NAME '(' args ')'    { $$ = p->MemberFunction( *$1, *$3, *$5 ); p->Track( $$, $5 ); }

     | STATE '(' statename ')'       { $$ = p->State( *$3 );                    p->Track( $$ ); }
     | SIGNAL_ '(' addr ',' addr ')' { $$ = p->Signal( $3, $5 );                p->Track( $$, $3, $5 ); }
;

assignment: /* assignment uses the := operator to prevent unwanted assignment */
       NAME ':' '=' exp              { $$ = p->VariableAssignment( *$1, $4 ); p->Track( $$, $4 ); }
     | STATE '(' statename ')' ':' '=' exp
                                     { $$ = p->StateAssignment( *$3, $7 ); p->Track( $$, $7 ); }
;

args: /* empty */                    { $$ = new NodeList; p->Track( $$ ); }
     | list                          { $$ = $1; }
;

list:  exp                           { $$ = new NodeList; $$->push_back( $1 ); p->Track( $$ ); }
     | list ',' exp                  { $$ = $1; $$->push_back( $3 ); }
;

quoted:
       '"' NAME '"'                  { $$ = $2; }
     | '\'' NAME '\''                { $$ = $2; }
;

addr:  exp                           { $$ = new AddressNode( $1, NULL ) ; p->Track( $$, $1 ); }
     | exp NAME                      { $$ = new AddressNode( $1, $2 );    p->Track( $$, $1 ); }
     | quoted                        { $$ = new AddressNode( NULL, $1 );  p->Track( $$ ); }
;

statename:
       NAME                          { $$ = $1; }
     | quoted                        { $$ = $1; }
;

%%

  int
  yylex( YYSTYPE* pLval, ArithmeticExpression* pInstance )
  {
    int token = -1;

    pInstance->mInput >> ws;
    int c = pInstance->mInput.peek();
    if( c == EOF )
      token = 0;
    else if( ::isdigit( c ) )
    {
      if( pInstance->mInput >> pLval->value )
        token = NUMBER;
    }
    else if( ::isalnum( c ) )
    {
      pLval->str = pInstance->Track( new string );
      while( ::isalnum( c ) )
      {
        *pLval->str += c;
        pInstance->mInput.ignore();
        c = pInstance->mInput.peek();
      }
      if( ::stricmp( pLval->str->c_str(), "state" ) == 0 )
        token = STATE;
      else if( ::stricmp( pLval->str->c_str(), "signal" ) == 0 )
        token = SIGNAL_;
      else
        token = NAME;
    }
    else if( c == '/' ) // handle comments
    {
      pInstance->mInput.ignore();
      token = c;
      if( pInstance->mInput.peek() == '/' )
      {
        pInstance->mInput.ignore( numeric_limits<streamsize>::max(), '\n' );
        token = yylex( pLval, pInstance );
      }
    }
    else
    {
      pInstance->mInput.ignore();
      token = c;
    }
    return token;
  }

  void
  yyerror( ArithmeticExpression* pInstance, const char* pError )
  {
    pInstance->mErrors << pError << endl;
  }

} // namespace ExpressionParser


