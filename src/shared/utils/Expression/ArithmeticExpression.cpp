//////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A simple expression parser for use within BCI2000.
//   ArithmeticExpression provides expression parsing; its Expression
//   descendant, in addition, allows access to State and Signal values.
//   For details about expression syntax, see Expression.h.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#if( defined( _BORLANDC_ ) && !defined( BCI_TOOL ) )
# define VCL_EXCEPTIONS 1
#endif

#ifdef VCL_EXCEPTIONS
# include <vcl.h>
#endif
#include <cmath>
#include <string>
#include <stdexcept>
#include <iostream>
#include "ArithmeticExpression.h"
#include "BCIError.h"

using namespace std;

ArithmeticExpression::ArithmeticExpression()
: mValue( 0 )
{
}

ArithmeticExpression::ArithmeticExpression( const std::string& s )
: mExpression( s ), mValue( 0 )
{
}

ArithmeticExpression::ArithmeticExpression( const ArithmeticExpression& e )
: mExpression( e.mExpression ), mValue( 0 )
{
}

const ArithmeticExpression&
ArithmeticExpression::operator=( const ArithmeticExpression& e )
{
  mExpression = e.mExpression;
  mInput.str( "" );
  mInput.clear();
  mErrors.str( "" );
  mErrors.clear();
  mValue = 0;
  return *this;
}


double
ArithmeticExpression::Evaluate()
{
  Parse();
  if( !mErrors.str().empty() )
    bcierr__ << "ArithmeticExpression::Evaluate: When evaluating \""
             << mExpression << "\": "
             << mErrors.str() << endl;
  return mValue;
}

bool
ArithmeticExpression::IsValid()
{
  Parse();
  return mErrors.str().empty();
}

double
ArithmeticExpression::State( const char* )
{
  Errors() << "Use an Expression rather than an ArithmeticExpression to access states"
           << endl;
  return 0;
}

double
ArithmeticExpression::Signal( const string&, const string& )
{
  Errors() << "Use an Expression rather than an ArithmeticExpression to access a signal"
           << endl;
  return 0;
}

void
ArithmeticExpression::Parse()
{
  mInput.clear();
  mInput.str( mExpression );
  mErrors.clear();
  mErrors.str( "" );
  try
  {
    if( ExpressionParser::yyparse( this ) != 0 )
      mValue = 0;
  }
  catch( const char* s )
  {
    Errors() << s << endl;
    mValue = 0;
  }
  catch( const exception& e )
  {
    Errors() << e.what() << endl;
    mValue = 0;
  }
#ifdef VCL_EXCEPTIONS
  catch( const class EMathError& e )
  {
    Errors() << e.Message.c_str() << endl;
    mValue = 0;
  }
#endif
  catch( ... )
  {
    Errors() << "Unknown exception caught" << endl;
    mValue = 0;
  }
}

#ifdef VCL_EXCEPTIONS
// _matherr() error handling interface.
static void
ThrowMathError( const char* functionName )
{
  throw runtime_error( string( "Error in function " ) + functionName + "()" );
}

int
_matherr( struct _exception* e )
{
  ThrowMathError( e->name );
  return true;
}

int
_matherrl( struct _exceptionl* e )
{
  ThrowMathError( e->name );
  return true;
}
#endif // VCL_EXCEPTIONS
