//////////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        Expression.cpp
// Date:        Aug 12, 2005
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A simple BCI2000 expression parser.
//              See Expression.h for details about expressions.
// $Log$
// Revision 1.5  2006/03/30 10:17:22  mellinger
// VC++ compatibility.
//
// Revision 1.4  2006/02/03 13:40:53  mellinger
// Compatibility with gcc and BCB 2006.
//
// Revision 1.3  2006/01/11 19:00:43  mellinger
// Removed VCL classes when compiled with BCI_TOOL flag; removed "using namespace" from header file; introduced CVS id and log.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////////////
#include "../PCHIncludes.h"
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
Expression::ReportError( const char* message ) const
{
  __bcierr << "Expression::Evaluate: When evaluating \"" << mExpression << "\": "
           << message << endl;
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
