////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that centralizes color types, conversion routines,
//              and formatted/unformatted i/o.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Color.h"

#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>

using namespace std;

RGBColor
RGBColor::operator*( float f ) const
{
  float r = f * R(),
        g = f * G(),
        b = f * B();
  return RGBColor( min( r, float( 0xff ) ), min( g, float( 0xff ) ), min( b, float( 0xff ) ) );
}

ostream&
RGBColor::WriteToStream( ostream& os ) const
{
  ios_base::fmtflags flags = os.flags();
  os << "0x" << hex << uppercase << setw( 6 ) << setfill( '0' ) << right
     << ( mValue & 0x00ffffff );
  os.flags( flags );
  return os;
}

istream&
RGBColor::ReadFromStream( istream& is )
{
  ios_base::fmtflags flags = is.flags();
  is >> hex >> showbase >> mValue;
  if( mValue & 0xff000000 )
    is.setstate( ios::failbit );
  is.flags( flags );
  if( !is )
    mValue = NullColor;
  return is;
}

int
RGBColor::ToWinColor() const
{
  return R() + ( G() << 8 ) + ( B() << 16 );
}

RGBColor
RGBColor::FromWinColor( int c )
{
  if( c == RGBColor::InvalidColor )
    return RGBColor::NullColor;
    
  int r = c & 0xff,
      g = ( c >> 8 ) & 0xff,
      b = ( c >> 16 ) & 0xff;
  return RGBColor( r, g, b );
}

RGBColor
RGBColor::ToGray() const
{
  int gray = ( R() + G() + B() ) / 3;
  return RGBColor( gray, gray, gray );
}

RGBColor
RGBColor::FromHSV( float H, float S, float V )
{
  // According to Foley and VanDam.
  // All input components range from 0 to 1 - EPS.
  float h = 6.0 * ::fmod( H, 1.0f );
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
      result = RGBColor( m, n, v );
      break;
    case 1:
      result = RGBColor( m, v, n );
      break;
    case 2:
      result = RGBColor( n, v, m );
      break;
    case 3:
      result = RGBColor( v, n, m );
      break;
    case 4:
      result = RGBColor( v, m, n );
      break;
    case 5:
      result = RGBColor( n, m, v );
      break;
    default:
      assert( false );
  }
  return result;
}

ColorList::ColorList( const RGBColor* values )
{
  const RGBColor* c = values;
  while( *c != End )
    push_back( *c++ );
}

