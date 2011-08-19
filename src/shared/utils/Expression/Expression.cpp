//////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A simple expression parser for use within BCI2000.
//              See Expression.h for details about expressions.
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
//////////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Expression.h"
#include <sstream>

using namespace std;


Expression&
Expression::SetOptionalAccess( State::ValueType inDefaultValue )
{
  mOptionalAccess = true;
  mDefaultValue = inDefaultValue;
  return *this;
}

Expression&
Expression::ClearOptionalAccess()
{
  mOptionalAccess = false;
  return *this;
}

bool
Expression::IsValid( const GenericSignal* inSignal )
{
  mpSignal = inSignal;
  bool result = ArithmeticExpression::IsValid();
  mpSignal = NULL;
  return result;
}

double
Expression::Evaluate( const GenericSignal* inSignal )
{
  mpSignal = inSignal;
  double result = ArithmeticExpression::Evaluate();
  mpSignal = NULL;
  return result;
}

double
Expression::State( const string& inName )
{
  return mOptionalAccess
   ? Environment::OptionalState( inName, mDefaultValue )
   : Environment::State( inName );
}

double
Expression::Signal( const string& inChannelAddress, const string& inElementAddress )
{
  if( mpSignal == NULL )
  {
    Errors() << "Trying to access NULL signal" << endl;
    return 0;
  }
  int channel = static_cast<int>( mpSignal->Properties().ChannelIndex( inChannelAddress ) );
  if( channel < 0 || channel >= mpSignal->Channels() )
  {
    Errors() << "Channel index or address ("
             << inChannelAddress
             << ") out of range";
    return 0;
  }
  int element = static_cast<int>( mpSignal->Properties().ElementIndex( inElementAddress ) );
  if( element < 0 || element >= mpSignal->Elements() )
  {
    Errors() << "Element index or address ("
             << inElementAddress
             << ") out of range";
    return 0;
  }
  return ( *mpSignal )( channel, element );
}


