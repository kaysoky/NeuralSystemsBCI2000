////////////////////////////////////////////////////////////////////////////////
//
// File: Color.h
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
#ifndef ColorH
#define ColorH

#include <iostream>
#include <vector>
#include "VisList.h"

enum ColorNames
{
  NullColor = 0xff000000,
  Black = 0x0,
  Maroon = 0x80,
  Green = 0x8000,
  Olive = 0x8080,
  Navy = 0x800000,
  Purple = 0x800080,
  Teal = 0x808000,
  Gray = 0x808080,
  Silver = 0xc0c0c0,
  Red = 0xff,
  Lime = 0xff00,
  Yellow = 0xffff,
  Blue = 0xff0000,
  Fuchsia = 0xff00ff,
  Aqua = 0xffff00,
  LtGray = 0xc0c0c0,
  DkGray = 0x808080,
  White = 0xffffff,
};

class RGBColor
{
  public:
    RGBColor() : mValue( 0 ) {}
    RGBColor( int value ) : mValue( value ) {}
    RGBColor( int r, int g, int b ) : mValue( ( r << 16 ) + ( g << 8 ) + b ) {}
    operator int() const { return mValue; }
    void WriteToStream( std::ostream& ) const;
    void ReadFromStream( std::istream& );

    // Create a RGB color from Hue, Saturation, and Value.
    static RGBColor HSVColor( float h, float s, float v );

  private:
    int mValue;
};

class Colorlist: public VisList<RGBColor>
{
  public:
    enum
    {
      End = NullColor,
    };
    Colorlist() {}
    // Create a color list from a RGBColor array. The last entry in the array
    // must be Colorlist::End.
    Colorlist( const RGBColor* );
};

inline std::ostream& operator<<( std::ostream& s, const RGBColor& c )
{
  c.WriteToStream( s );
  return s;
}

inline std::istream& operator>>( std::istream& s, RGBColor& c )
{
  c.ReadFromStream( s );
  return s;
}

inline std::ostream& operator<<( std::ostream& s, const Colorlist& c )
{
  c.WriteToStream( s );
  return s;
}

inline std::istream& operator>>( std::istream& s, Colorlist& c )
{
  c.ReadFromStream( s );
  return s;
}

#endif // ColorH

