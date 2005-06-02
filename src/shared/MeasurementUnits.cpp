/////////////////////////////////////////////////////////////////////////////
//
// File: MeasurementUnits.cpp
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "MeasurementUnits.h"

#include "UBCIError.h"
#include <cmath>

using namespace std;

float MeasurementUnits::sUnitsPerSec = 1.0;
float MeasurementUnits::sUnitsPerHertz = 1.0;
float MeasurementUnits::sUnitsPerVolt = 1.0;

float
MeasurementUnits::ToUnit( const char* inValue, const char* inUnitName, float inUnitValue )
{
  string value( inValue );
  float  unit = 1.0;
  size_t pos = value.find_first_not_of( "0123456789.Ee+- " );
  if( pos != value.npos )
  {
    string unitFromValue = value.substr( pos ),
           unitName = inUnitName;
    if( unitFromValue.length() >= unitName.length()
        && unitFromValue.substr( unitFromValue.length() - unitName.length() ) == unitName )
    {
      unit *= inUnitValue;
      unitFromValue.erase( unitFromValue.length() - unitName.length(), unitName.length() );
    }
    const struct
    {
      const char* name;
      float       value;
    } prefixes[] =
    {
      "", 1.0,
      "p", 1e-12,
      "n", 1e-9,
      "u", 1e-6, "mu", 1e-6,
      "m", 1e-3,
      "k", 1e3,
      "M", 1e6,
      "G", 1e9,
      "T", 1e12,
    };
    const int numPrefixes = sizeof( prefixes ) / sizeof( *prefixes );
    int i = 0;
    while( i < numPrefixes && unitFromValue != prefixes[ i ].name )
      ++i;
    if( i < numPrefixes )
      unit *= prefixes[ i ].value;
    else
      bcierr << "Unknown measurement unit in expression '" << inValue << "',"
             << " expected '" << inUnitName << "'" << endl;
    value.erase( pos );
  }
  return unit * ::atof( value.c_str() );
}


