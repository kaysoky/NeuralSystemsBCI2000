/////////////////////////////////////////////////////////////////////////////
//
// File: GUI.cpp
//
// Date: Oct 22, 2001
//
// Author: Juergen Mellinger
//
// Description: This file contains the definitions for those objects from GUI.h
//              that need them.
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#define GUI_DEFINE_OBJECTS
#include "GUI.h"
#include "Param.h"
#include "ParamList.h"

TGUIRect::TGUIRect()
: left( 0.0 ), top( 0.0 ), right( 1.0 ), bottom( 1.0 )
{
}

TGUIRect::TGUIRect( const TGUIRect &inRect )
: left( inRect.left ),
  top( inRect.top ),
  right( inRect.right ),
  bottom( inRect.bottom )
{
}

TGUIRect::TGUIRect( float inLeft, float inTop, float inRight, float inBottom )
: left( inLeft ), top( inTop ), right( inRight ), bottom( inBottom )
{
}

TPresError
TGUIRect::ReadFromParam( const Param    *inParamPtr )
{
    const char*     valPtr;
    float           vals[ 4 ];
    if( inParamPtr == NULL )
        return presParamInaccessibleError;
    if( inParamPtr->NumValues() != 4 )
        return presParamOutOfRangeError;
    for( int i = 0; i < 4; i++ )
    {
        vals[ i ] = atof( inParamPtr->Value( i ).c_str() );
        // By convention, +/- 101.0 means +/- infinity which
        // does not make sense here but should not cause an error.
        if( vals[ i ] < -100.0 )
        {
            if( vals[ i ] < -101.0 )
                return presParamOutOfRangeError;
            else
                vals[ i ] = -100.0;
        }
        if( vals[ i ] > 100.0 )
        {
            if( vals[ i ] > 101.0 )
                return presParamOutOfRangeError;
            else
                vals[ i ] = 100.0;
        }
        vals[ i ] += 100.0;
        vals[ i ] /= 200.0;
    }
    left = vals[ 0 ];
    top = vals[ 1 ];
    right = vals[ 2 ];
    bottom = vals[ 3 ];
    return presNoError;
}

