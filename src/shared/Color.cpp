////////////////////////////////////////////////////////////////////////////////
//
// File: Color.cpp
//
// Description: A class that centralizes color types, conversion routines,
//              and formatted/unformatted i/o.
//
// Author: Juergen Mellinger
//
// Date:   Dec 10, 2003
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Color.h"

#include <math.h>
#include <iostream>
#include <string>
#include <cassert>

using namespace std;

void
RGBColor::WriteToStream( std::ostream& os ) const
{
  ios_base::fmtflags flags = os.flags();
  os << hex << showbase << ( mValue & 0x00ffffff );
  os.flags( flags );
}

void
RGBColor::ReadFromStream( std::istream& is )
{
  ios_base::fmtflags flags = is.flags();
  is >> hex >> showbase >> mValue;
  if( mValue & 0xff000000 )
    is.setstate( ios::failbit );
  is.flags( flags );
}

RGBColor
RGBColor::HSVColor( float H, float S, float V )
{
  // According to Foley and VanDam.
  // All input components range from 0 to 1 - EPS.
  float h = 6.0 * ::fmod( H, 1.0 );
  if( h < 0.0 )
    h += 6.0;
  int i = ::floor( h );
  float f = h - i;
  if( !( i % 2 ) )
    f = 1.0 - f;
  unsigned int m = ( V * ( 1.0 - S ) ) * 0x100,
               n = ( V * ( 1.0 - S * f ) ) * 0x100,
               v = V * 0x100;
  if( m > 0xff )
    m = 0xff;
  if( n > 0xff )
    n = 0xff;
  if( v > 0xff )
    v = 0xff;
  RGBColor result;
  switch( i )
  {
    case 0:
      result = RGBColor( v, n, m );
      break;
    case 1:
      result = RGBColor( n, v, m );
      break;
    case 2:
      result = RGBColor( m, v, n );
      break;
    case 3:
      result = RGBColor( m, n, v );
      break;
    case 4:
      result = RGBColor( n, m, v );
      break;
    case 5:
      result = RGBColor( v, m, n );
      break;
    default:
      assert( false );
  }
  return result;
}

Colorlist::Colorlist( const RGBColor* values )
{
  const RGBColor* c = values;
  while( *c != End )
    push_back( *c++ );
}

