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

#include <cmath>
#include <string>
#include <stdexcept>
#include "Expression.h"

using namespace std;


const Expression&
Expression::operator=( const Expression& e )
{
  *this = e;
  mpSignal = NULL;
  return *this;
}

double
Expression::Evaluate( const GenericSignal* signal )
{
  mpSignal = signal;
  return Evaluate();
}

double
Expression::State( const char* inName ) const
{
  return Environment::State( inName );
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


