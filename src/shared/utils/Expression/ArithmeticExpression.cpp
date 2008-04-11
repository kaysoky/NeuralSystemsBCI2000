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
#include "ArithmeticExpression.h"
#include "BCIError.h"

using namespace std;

const ArithmeticExpression&
ArithmeticExpression::operator=( const ArithmeticExpression& e )
{
  mExpression = e.mExpression;
  mInput.str( "" );
  mInput.clear();
  mValue = 0;
  return *this;
}


double
ArithmeticExpression::Evaluate()
{
  mInput.clear();
  mInput.str( mExpression );
  try
  {
    if( ExpressionParser::yyparse( this ) != 0 )
      mValue = 0;
  }
  catch( const exception& e )
  {
    ReportError( e.what() );
    mValue = 0;
  }
#ifdef VCL_EXCEPTIONS
  catch( const class EMathError& e )
  {
    ReportError( e.Message.c_str() );
    mValue = 0;
  }
#endif
  return mValue;
}

double
ArithmeticExpression::State( const char* ) const
{
  ReportError( "Use an Expression rather than an ArithmeticExpression to access states" );
  return 0;
}

double
ArithmeticExpression::Signal( const string&, const string& ) const
{
  ReportError( "Use an Expression rather than an ArithmeticExpression to access a signal" );
  return 0;
}

void
ArithmeticExpression::ReportError( const char* inMessage ) const
{
  bcierr__ << "ArithmeticExpression::Evaluate: When evaluating \"" << mExpression << "\": "
           << inMessage << endl;
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
#endif // BCI_TOOL
