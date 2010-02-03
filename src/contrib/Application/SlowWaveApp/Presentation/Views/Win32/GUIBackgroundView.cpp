/////////////////////////////////////////////////////////////////////////////
//
// File: GUIBackgroundView.cpp
//
// Date: Nov 7, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUIBackgroundView class implements the GUI specific details
//              of the TBackgroundView class.
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

#include "OSIncludes.h"

#ifndef VCL
# error This is the VCL implementation of TGUIBackgroundView.
#endif

#include "GUIBackgroundView.h"

TGUIBackgroundView::TGUIBackgroundView()
: TGUIView( backgroundViewZ )
{
}

void
TGUIBackgroundView::Paint()
{
    TCanvas *canvas = GetCanvas();
    canvas->Pen->Color = TGUIView::GetElementColor( fbBackground ).cl;
    canvas->Brush->Color = canvas->Pen->Color;
    canvas->FillRect( viewTRect );
}

