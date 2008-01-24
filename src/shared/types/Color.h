////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that centralizes color types, conversion routines,
//              and formatted/unformatted i/o.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef COLOR_H
#define COLOR_H

#include <iostream>
#include <vector>
#include <string>
#include "ValueList.h"

class RGBColor
{
 public:
  enum ColorNames
  { //                R G B
    NullColor   = 0xff000000,
    Black       = 0x00000000,
    Maroon      = 0x00800000,
    Green       = 0x00008000,
    Olive       = 0x00808000,
    Navy        = 0x00000080,
    Purple      = 0x00800080,
    Teal        = 0x00008080,
    Gray        = 0x00808080,
    Silver      = 0x00c0c0c0,
    Red         = 0x00ff0000,
    Lime        = 0x0000ff00,
    Yellow      = 0x00ffff00,
    Blue        = 0x000000ff,
    Fuchsia     = 0x00ff00ff,
    Aqua        = 0x0000ffff,
    LtGray      = 0x00c0c0c0,
    DkGray      = 0x00808080,
    White       = 0x00ffffff,
  };

  public:
    RGBColor()
      : mValue( 0 )
      {}
    RGBColor( int value )
      : mValue( value )
      {}
    RGBColor( int r, int g, int b )
      : mValue( ( r << 16 ) + ( g << 8 ) + b )
      {}
    operator int() const
      { return mValue; }

    const RGBColor& operator*=( float f )
      { *this = *this * f; return *this; }
    RGBColor operator*( float ) const;

    int R() const
      { return ( mValue >> 16 ) & 0xff; }
    int G() const
      { return ( mValue >> 8 ) & 0xff; }
    int B() const
      { return mValue & 0xff; }

    std::ostream& WriteToStream( std::ostream& ) const;
    std::istream& ReadFromStream( std::istream& );

    // Convert to an MS Windows encoded RGB value and back.
    int             ToWinColor() const;
    static RGBColor FromWinColor( int );

    // Return luminance as a greyscale color value.
    RGBColor ToGray() const;
    // Create a RGB color from Hue, Saturation, and Value.
    static RGBColor FromHSV( float h, float s, float v );

  private:
    int mValue;
};

RGBColor operator*( float, RGBColor );

std::ostream& operator<<( std::ostream&, const RGBColor& );
std::istream& operator>>( std::istream&, RGBColor& );


class ColorList: public ValueList<RGBColor>
{
  public:
    enum
    {
      End = RGBColor::NullColor,
    };
    ColorList()
      {}
    // Create a color list from a RGBColor array. The last entry in the array
    // must be Colorlist::End.
    ColorList( const RGBColor* );
};

std::ostream& operator<<( std::ostream&, const ColorList& );
std::istream& operator>>( std::istream&, ColorList& );

////////////////////////////////////////////////////////////////////////////////
// inline definitions
////////////////////////////////////////////////////////////////////////////////
inline
RGBColor
operator*( float f, RGBColor c )
{
  return c * f;
}

inline
std::ostream&
operator<<( std::ostream& s, const RGBColor& c )
{
  return c.WriteToStream( s );
}

inline
std::istream&
operator>>( std::istream& s, RGBColor& c )
{
  return c.ReadFromStream( s );
}

inline
std::ostream&
operator<<( std::ostream& s, const ColorList& c )
{
  return c.WriteToStream( s );
}

inline
std::istream&
operator>>( std::istream& s, ColorList& c )
{
  return c.ReadFromStream( s );
}

#endif // COLOR_H

