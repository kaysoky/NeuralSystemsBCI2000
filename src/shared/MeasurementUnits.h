/////////////////////////////////////////////////////////////////////////////
//
// File: MeasurementUnits.h
//
// Date: Feb 7, 2004
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: Utilities for handling measurement units and conversions.
//
// Changes:
//
//////////////////////////////////////////////////////////////////////////////
#ifndef MeasurementUnitsH
#define MeasurementUnitsH

#if 0
// This class converts plain numbers to strings such as "400ms".
class DisplayUnit
{
  public:
    DisplayUnit( float inValue, const std::string& inUnitName );
    DisplayUnit( const std::string& inUnitString );

    const std::string& GetName() const               { return unitName; }
    float              GetValue() const              { return unitValue; }
    const std::string& WriteWithUnit( float ) const;

  private:
    float       unitValue;
    std::string unitName;
};

inline
std::ostream& operator<<( std::ostream& os, const DisplayUnit& du )
{
  du.WriteToStream( os );
  return os;
}
#endif

// This class converts strings such as "123.3" or "12ms" to plain numbers that
// represent values in global units.
class MeasurementUnits
{
  public:
    static float   ReadAsTime( const char* value )
    { return ToUnit( value, "s", sUnitsPerSec ); }
    static void  InitializeTimeUnit( float inUnitsPerSec )
    { sUnitsPerSec = inUnitsPerSec; }

    static float   ReadAsFreq( const char* value )
    { return ToUnit( value, "Hz", sUnitsPerHertz ); }
    static void  InitializeFreqUnit( float inUnitsPerHertz )
    { sUnitsPerHertz = inUnitsPerHertz; }

  private:
    static float ToUnit( const char*, const char* inUnitName, float inUnitValue );
    static float sUnitsPerSec;
    static float sUnitsPerHertz;
};

#endif // MeasurementUnitsH
