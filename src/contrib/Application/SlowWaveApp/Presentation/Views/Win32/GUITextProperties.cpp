/////////////////////////////////////////////////////////////////////////////
//
// File: GUITextProperties.cpp
//
// Date: Jan 7, 2002
//
// Author: Juergen Mellinger
//
// Description: A mix-in class for views that handle text properties.
//
// Changes:
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "OSIncludes.h"

#ifndef VCL
# error This is the VCL implementation of TGUIView.
#endif

#include "GUITextProperties.h"
#include "GUIView.h"

void
TGUITextProperties::SetFontName( const char *inName )
{
    fontName = inName;
}

void
TGUITextProperties::SetFontSize( float inSize )
{
    fontSize = inSize;
}

void
TGUITextProperties::SetFontStyle( TGUIFontStyle inStyle )
{
    fontStyle = TGUIView::GUIFontStyleToOSFontStyle( inStyle );
}

void
TGUITextProperties::SetFontColor( TGUIColor inColor )
{
    fontColor = TGUIView::GUIColorToOSColor( inColor );
}

void
TGUITextProperties::SetAlignment( TGUIAlignment inAlignment )
{
    textAlignment = TGUIView::GUIAlignmentToOSAlignment( inAlignment );
}

void
TGUITextProperties::SetVAlignment( TGUIVAlignment inAlignment )
{
    vTextAlignment = inAlignment;
}

void
TGUITextProperties::SetBiDi(   TGUIBiDi inBiDi )
{
    biDi = inBiDi;
}

