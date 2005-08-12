//////////////////////////////////////////////////////////////////////////////////////
//
// File:        Expression.cpp
// Date:        Aug 12, 2005
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A simple BCI2000 expression parser.
//              See Expression.h for details about expressions.
//
//////////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include <vcl.h>
#include <SysUtils.hpp>
#include <cmath>
#include "Expression.h"

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
  catch( const EMathError& e )
  {
    ReportError( e.Message.c_str() );
    mValue = 0;
  }
  catch( const exception& e )
  {
    ReportError( e.what() );
    mValue = 0;
  }
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
Expression::ReportError( const char* message ) const
{
  __bcierr << "Expression::Evaluate: When evaluating \"" << mExpression << "\": "
           << message << endl;
}

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
