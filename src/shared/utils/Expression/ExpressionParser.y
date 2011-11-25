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
////////////////////////////////////////////////////////////////////////
#include <sstream>
#include <string>
#include <vector>
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


using namespace std;

%}

%pure-parser
%parse-param { ::ArithmeticExpression* pInstance }
%lex-param   { ::ArithmeticExpression* pInstance }
%union
{
  double               value;
  std::string*         str;
  std::vector<double>* list;
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

%type <value> input statements statement exp assignment
%type <str>   quoted statename addr
%type <list>  args list

%% /* The grammar follows.  */
input: /* empty */                   { pInstance->mValue = 0; }
     | statements                    { pInstance->mValue = $1; }
;

statements:
       statement                     { $$ = $1; }
     | statements ';' statement      { $$ = $3; }
     | statements ';'                { $$ = $1; }
     | ';'                           { $$ = 0; }
;

statement:     
       exp                           { $$ = $1; }
     | assignment                    { $$ = $1; }
;

exp:   NUMBER                        { $$ = $1;       }
     | exp '+' exp                   { $$ = $1 + $3;  }
     | exp '-' exp                   { $$ = $1 - $3;  }
     | exp '*' exp                   { $$ = $1 * $3;  }
     | exp '/' exp                   { $$ = $1 / $3;  }
     | '-' exp %prec NEG             { $$ = -$2;      }
     | exp '^' exp                   { $$ = ::pow( $1, $3 ); }
     | exp '&' '&' exp               { $$ = $1 && $4; }
     | exp '|' '|' exp               { $$ = $1 || $4; }
     | exp '=' '=' exp               { $$ = $1 == $4; }
     | exp '!' '=' exp               { $$ = $1 != $4; }
     | exp '~' '=' exp               { $$ = $1 != $4; }
     | exp '>' exp                   { $$ = $1 > $3;  }
     | exp '<' exp                   { $$ = $1 < $3;  }
     | exp '>' '=' exp               { $$ = $1 >= $4; }
     | exp '<' '=' exp               { $$ = $1 <= $4; }
     | '~' exp %prec NEG             { $$ = !$2;      }
     | '!' exp %prec NEG             { $$ = !$2;      }
     | '(' exp ')'                   { $$ = $2;       }
     | exp '?' exp ':' exp           { $$ = $1 ? $3 : $5 }
     
     | NAME                          { $$ = pInstance->Variable( *$1 ); }
     | exp '.' NAME                  { $$ = pInstance->MemberVariable( $1, *$3 ); }
     | NAME '(' args ')'             { $$ = pInstance->Function( *$1, *$3 ); }
     | exp '.' NAME '(' args ')'     { $$ = pInstance->MemberFunction( $1, *$3, *$5 ); }

     | STATE '(' statename ')'       { $$ = pInstance->State( *$3 ); }
     | SIGNAL_ '(' addr ',' addr ')' { $$ = pInstance->Signal( *$3, *$5 ); }
;

assignment: /* assignment uses the := operator to prevent unwanted assignment */
       NAME ':' '=' exp              { $$ = $4; pInstance->VariableAssignment( *$1, $4 ); }
     | exp '.' NAME ':' '=' exp      { $$ = $6; pInstance->MemberVariableAssignment( $1, *$3, $6 ); }
     | STATE '(' statename ')' ':' '=' exp
                                     { $$ = $7; pInstance->StateAssignment( *$3, $7 ); }
;

args: /* empty */                    { $$ = pInstance->Allocate< vector<double> >(); }
     | list                          { $$ = $1; }
;

list:  exp                           { $$ = pInstance->Allocate< vector<double> >(); $$->push_back( $1 ); }
     | list ',' exp                  { $$ = $1; $$->push_back( $3 ); }
;

quoted:
       '"' NAME '"'                  { $$ = $2; }
     | '\'' NAME '\''                { $$ = $2; }
;

addr:  exp                           { ostringstream oss; oss << $1; $$ = pInstance->Allocate( oss.str() ); }
     | exp NAME                      { ostringstream oss; oss << $1 << *$2; $$ = pInstance->Allocate( oss.str() ); }
     | quoted                        { $$ = $1; }
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
      pLval->str = pInstance->Allocate<string>();
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
        pInstance->mInput.ignore( INT_MAX, '\n' );
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


