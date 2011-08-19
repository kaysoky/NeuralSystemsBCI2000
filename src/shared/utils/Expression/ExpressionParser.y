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
  double       value;
  std::string* str;
}


%{
namespace ExpressionParser
{
%}

/* Bison declarations.  */
%token <value>   NUMBER
%token <str>     NAME SIGNAL_ /* avoid interference with Qt's SIGNAL macro */
%left '?' ':'
%left '&' '|'
%left '=' '~' '!' '>' '<'
%left '-' '+'
%left '*' '/'
%left NEG     /* negation--unary minus */
%right '^'    /* exponentiation */

%type <value> exp input
%type <str>   addr

%% /* The grammar follows.  */
input: /* empty */                   { pInstance->mValue = 0; }
     | exp                           { pInstance->mValue = $1; }
;

exp:   NAME                          { $$ = pInstance->State( *$1 ); }
     | NUMBER                        { $$ = $1;       }
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
     | SIGNAL_ '(' addr ',' addr ')' { $$ = pInstance->Signal( *$3, *$5 ); }
;

addr:  exp                           { ostringstream oss; oss << $1; $$ = pInstance->AllocateCopy( oss.str() ); }
     | exp NAME                      { ostringstream oss; oss << $1 << *$2; $$ = pInstance->AllocateCopy( oss.str() ); }
     | '"' NAME '"'                  { $$ = $2; }
     | '\'' NAME '\''                { $$ = $2; }
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
      pLval->str = pInstance->AllocateCopy( string() );
      while( ::isalnum( c ) )
      {
        *pLval->str += c;
        pInstance->mInput.ignore();
        c = pInstance->mInput.peek();
      }
      if( ::stricmp( pLval->str->c_str(), "signal" ) == 0 )
        token = SIGNAL_;
      else
        token = NAME;
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


