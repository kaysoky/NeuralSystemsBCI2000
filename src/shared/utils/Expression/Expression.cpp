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

bool
Expression::IsValid( const GenericSignal* signal )
{
  mpSignal = signal;
  return ArithmeticExpression::IsValid();
}

double
Expression::Evaluate( const GenericSignal* signal )
{
  mpSignal = signal;
  return ArithmeticExpression::Evaluate();
}

double
Expression::State( const char* inName )
{
  return Environment::State( inName );
}

double
Expression::Signal( const string& inChannelAddress, const string& inElementAddress )
{
  if( mpSignal == NULL )
  {
    Errors() << "Trying to access NULL signal" << endl;
    return 0;
  }
  int channel = mpSignal->Properties().ChannelIndex( inChannelAddress );
  if( channel < 0 || channel >= mpSignal->Channels() )
  {
    Errors() << "Channel index or address ("
             << inChannelAddress
             << ") out of range";
    return 0;
  }
  int element = mpSignal->Properties().ElementIndex( inElementAddress );
  if( element < 0 || element >= mpSignal->Elements() )
  {
    Errors() << "Element index or address ("
             << inElementAddress
             << ") out of range";
    return 0;
  }
  return ( *mpSignal )( channel, element );
}


