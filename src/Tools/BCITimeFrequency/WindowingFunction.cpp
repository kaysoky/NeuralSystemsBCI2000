//////////////////////////////////////////////////////////////////////////////////////
//
// File:        WindowingFunction.cpp
//
// Author:      juergen.mellinger@uni-tuebingen.de
//
// Description: A class that encapsulates details about windowing functions
//              used for sidelobe suppression in spectral analysis.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "WindowingFunction.h"
#include <math.h>

#define RANGECHECK( i )  \
  if( ( i ) < 0 || ( i ) >= sizeof( sWindowProperties ) / sizeof( *sWindowProperties ) ) \
    throw "Unknown Window type in " __FUNC__;

#define ENTRY( x )  (x),#x,Compute##x
const struct WindowingFunction::WindowProperties
WindowingFunction::sWindowProperties[] =
{
  { ENTRY( None ) },
  { ENTRY( Hamming ) },
  { ENTRY( Hann ) },
  { ENTRY( Blackman ) },
};

WindowingFunction::NumType
WindowingFunction::ComputeNone( NumType )
{
  return 1.0;
}

WindowingFunction::NumType
WindowingFunction::ComputeHamming( NumType inWhere )
{
  return 1.0 - 0.46 + 0.46 * ::cos( M_PI * inWhere );
}

WindowingFunction::NumType
WindowingFunction::ComputeHann( NumType inWhere )
{
  return 1.0 - 0.5 + 0.5 * ::cos( M_PI * inWhere );
}

WindowingFunction::NumType
WindowingFunction::ComputeBlackman( NumType inWhere )
{
  return 1.0 - 0.5 - 0.08 + 0.5 * ::cos( M_PI * inWhere ) + 0.08 * ::cos( 2 * M_PI * inWhere );
}


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

  return sWindowProperties[ mWindow ].mComputeValue( inWhere );  
}

