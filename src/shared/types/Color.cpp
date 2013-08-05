////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that centralizes color types, conversion routines,
//              and formatted/unformatted i/o.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
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
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Color.h"
#include "BCIAssert.h"
#include "FastConv.h"

#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

RGBColor
RGBColor::operator*( double f ) const
{
  int r = ToInt( f * R() ),
      g = ToInt( f * G() ),
      b = ToInt( f * B() );
  return RGBColor( min( r, 0xff ), min( g, 0xff ), min( b, 0xff ) );
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
  if( c == static_cast<int>( RGBColor::InvalidColor ) )
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
RGBColor::FromHSV( double H, double S, double V )
{
  // According to Foley and VanDam.
  // All input components range from 0 to 1 - EPS.
  double h = 6.0f * ::fmod( H, 1.0 );
  if( h < 0.0 )
    h += 6.0;
  int i = FloorToInt( h );
  if( i < 0 || i >= 6 ) // E.g., h == NaN.
    return RGBColor::NullColor;
  double f = h - i;
  if( !( i % 2 ) )
    f = 1.0 - f;
  int m = ToInt( ( V * ( 1.0 - S ) ) * 0x100 ),
      n = ToInt( ( V * ( 1.0 - S * f ) ) * 0x100 ),
      v = ToInt( V * 0x100 );
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
      ;
  }
  return result;
}

ColorList::ColorList( const RGBColor* values )
{
  const RGBColor* c = values;
  while( *c != static_cast<RGBColor>( End ) )
    push_back( *c++ );
}

