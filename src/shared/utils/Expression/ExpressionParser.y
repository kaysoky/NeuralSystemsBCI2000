%output="ExpressionParser.cpp"
%defines
%{
////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: Bison grammar file for a simple expression parser.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////
#include <sstream>
#include <string>
#include <cmath>
#include <cstring>
#include "ArithmeticExpression.h"
#include "BCIError.h"

#pragma warn -8004

using namespace std;

%}

%pure-parser
%parse-param { ::ArithmeticExpression* pInstance }
%lex-param   { ::ArithmeticExpression* pInstance }
%union
{
  double       value;
  const char*  name;
  std::string* str;
}

%{
namespace ExpressionParser
{
%}

/* Bison declarations.  */
%token <value>   NUMBER
%token <name>    NAME SIGNAL
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
input: /* empty */                  { pInstance->mValue = 0; }
     | exp                          { pInstance->mValue = $1; }
;

exp:   NAME                         { $$ = pInstance->State( $1 ); }
     | NUMBER                       { $$ = $1;       }
     | exp '+' exp                  { $$ = $1 + $3;  }
     | exp '-' exp                  { $$ = $1 - $3;  }
     | exp '*' exp                  { $$ = $1 * $3;  }
     | exp '/' exp                  { $$ = $1 / $3;  }
     | '-' exp %prec NEG            { $$ = -$2;      }
     | exp '^' exp                  { $$ = ::pow( $1, $3 ); }
     | exp '&' '&' exp              { $$ = $1 && $4; }
     | exp '|' '|' exp              { $$ = $1 || $4; }
     | exp '=' '=' exp              { $$ = $1 == $4; }
     | exp '!' '=' exp              { $$ = $1 != $4; }
     | exp '~' '=' exp              { $$ = $1 != $4; }
     | exp '>' exp                  { $$ = $1 > $3;  }
     | exp '<' exp                  { $$ = $1 < $3;  }
     | exp '>' '=' exp              { $$ = $1 >= $4; }
     | exp '<' '=' exp              { $$ = $1 <= $4; }
     | '~' exp %prec NEG            { $$ = !$2;      }
     | '!' exp %prec NEG            { $$ = !$2;      }
     | '(' exp ')'                  { $$ = $2;       }
     | exp '?' exp ':' exp          { $$ = $1 ? $3 : $5 }
     | SIGNAL '(' addr ',' addr ')' { $$ = pInstance->Signal( *$3, *$5 ); delete $3; delete $5; }
;

addr:  exp                          { ostringstream oss; oss << $1; $$ = new string( oss.str() ); }
     | exp NAME                     { ostringstream oss; oss << $1 << $2; $$ = new string( oss.str() ); }
     | '"' NAME '"'                 { $$ = new string( $2 ); }
     | '\'' NAME '\''               { $$ = new string( $2 ); }
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
      static string name;
      name = "";
      while( ::isalnum( c ) )
      {
        name += c;
        pInstance->mInput.ignore();
        c = pInstance->mInput.peek();
      }
      pLval->name = name.c_str();
      if( ::stricmp( pLval->name, "signal" ) == 0 )
        token = SIGNAL;
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

