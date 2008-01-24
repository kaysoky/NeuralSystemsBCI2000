//////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A simple expression parser for use within BCI2000.
//              See Expression.h for details about expressions.
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
#include "Expression.h"
#include "ClassName.h"

using namespace std;

const Expression&
Expression::operator=( const Expression& e )
{
  mExpression = e.mExpression;
  mpSignal = NULL;
  mInput.str( "" );
  mInput.clear();
  mValue = 0;
  return *this;
}


double
Expression::Evaluate( const GenericSignal* signal )
{
  mInput.clear();
  mInput.str( mExpression );
  mpSignal = signal;
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
Expression::Signal( double inChannel, double inElement ) const
{
  if( mpSignal == NULL )
  {
    ReportError( "Trying to access NULL signal" );
    return 0;
  }
  if( inChannel < 0 || inChannel >= mpSignal->Channels() )
  {
    ReportError( "Channel index out of range" );
    return 0;
  }
  if( inElement < 0 || inElement >= mpSignal->Elements() )
  {
    ReportError( "Element index out of range" );
    return 0;
  }
  return ( *mpSignal )( inChannel, inElement );
}

void
Expression::ReportError( const char* inMessage ) const
{
  bcierr__ << "Expression::Evaluate: When evaluating \"" << mExpression << "\": "
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
