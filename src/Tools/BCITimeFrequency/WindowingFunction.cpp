//////////////////////////////////////////////////////////////////////////////////////
//
// File:        WindowingFunction.cpp
//
// Author:      juergen.mellinger@uni-tuebingen.de
//
// Description: A class that encapsulates details about windowing functions
//              used for sidelobe suppression in spectral analysis.
//
//////////////////////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "WindowingFunction.h"
#include <math.h>

#define RANGECHECK( i )  \
  if( ( i ) < 0 || ( i ) >= sizeof( sWindowProperties ) / sizeof( *sWindowProperties ) ) \
    throw "Unknown Window type in " __FUNC__;

#define ENTRY( x )  (x),#x
const struct WindowingFunction::WindowProperties
WindowingFunction::sWindowProperties[] =
{
  { ENTRY( None ),     { 0,    0   } },
  { ENTRY( Hamming ),  { 0.46, 0   } },
  { ENTRY( Hann ),     { 0.5,  0   } },
  { ENTRY( Blackman ), { 0.5, 0.08 } },
};

const char*
WindowingFunction::WindowNames( int i )
{
  RANGECHECK( i );
  return sWindowProperties[ i ].mName;
}

WindowingFunction::WindowingFunction()
: mWindow( None )
{
}

WindowingFunction::WindowingFunction( int i )
: mWindow( static_cast<Window>( i ) )
{
  RANGECHECK( i );
}

const char*
WindowingFunction::Name() const
{
  return WindowNames( mWindow );
}

WindowingFunction::NumType
WindowingFunction::Value( WindowingFunction::NumType inWhere ) const
{
  if( inWhere < 0.0 || inWhere >= 1.0 )
    throw "Argument out of range in " __FUNC__;
    
  const NumType& a1 = sWindowProperties[ mWindow ].mGenerationCoeffs[ 0 ],
               & a2 = sWindowProperties[ mWindow ].mGenerationCoeffs[ 1 ];
  return 1.0 - a1 - a2 + a1 * ::cos( M_PI * inWhere ) + a2 * ::cos( 2 * M_PI * inWhere );
}

