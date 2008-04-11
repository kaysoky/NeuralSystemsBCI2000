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

#include "Expression.h"
#include <sstream>

using namespace std;


const Expression&
Expression::operator=( const Expression& e )
{
  this->ArithmeticExpression::operator=( e );
  mpSignal = NULL;
  return *this;
}

double
Expression::Evaluate( const GenericSignal* signal )
{
  mpSignal = signal;
  return ArithmeticExpression::Evaluate();
}

double
Expression::State( const char* inName ) const
{
  return Environment::State( inName );
}

double
Expression::Signal( const string& inChannelAddress, const string& inElementAddress ) const
{
  if( mpSignal == NULL )
  {
    ReportError( "Trying to access NULL signal" );
    return 0;
  }
  int channel = mpSignal->Properties().ChannelIndex( inChannelAddress );
  if( channel < 0 || channel >= mpSignal->Channels() )
  {
    ostringstream oss;
    oss << "Channel index or address ("
        << inChannelAddress
        << ") out of range";
    ReportError( oss.str().c_str() );
    return 0;
  }
  int element = mpSignal->Properties().ElementIndex( inElementAddress );
  if( element < 0 || element >= mpSignal->Elements() )
  {
    ostringstream oss;
    oss << "Element index or address ("
        << inElementAddress
        << ") out of range";
    ReportError( oss.str().c_str() );
    return 0;
  }
  return ( *mpSignal )( channel, element );
}


