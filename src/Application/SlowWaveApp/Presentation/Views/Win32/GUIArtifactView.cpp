/////////////////////////////////////////////////////////////////////////////
//
// File: GUIArtifactView.cpp
//
// Date: Nov 7, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUIArtifactView class implements the GUI specific details
//              of the TArtifactView class.
//
// Changes:
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "OSIncludes.h"

#ifndef VCL
# error This is the VCL implementation of TGUIArtifactView.
#endif

#include "GUIArtifactView.h"
#include "ViewsRes.h"

TGUIArtifactView::TGUIArtifactView()
: TGUIView( artifactViewZ ),
  indicatorVisible( false ),
  indicatorBuffer( NULL ),
  indicTRect( 0, 0, 0, 0 )
{
}

TGUIArtifactView::~TGUIArtifactView()
{
    delete indicatorBuffer;
}

void
TGUIArtifactView::Resized()
{
    TGUIView::Resized();
    if( indicatorBuffer != NULL )
    {
        indicTRect.Left = viewTRect.Left + ( viewTRect.Width() - indicatorBuffer->Width ) / 2;
        indicTRect.Top = viewTRect.Top + ( viewTRect.Height() - indicatorBuffer->Height ) / 2;
        indicTRect.Right = indicTRect.Left + indicatorBuffer->Width;
        indicTRect.Bottom = indicTRect.Top + indicatorBuffer->Height;
    }
}

void
TGUIArtifactView::Paint()
{
    if( indicatorVisible )
        GetCanvas()->Draw( indicTRect.Left, indicTRect.Top, indicatorBuffer );
}

TPresError
TGUIArtifactView::InitIndicator()
{
    TPresError  result = presNoError;

    delete indicatorBuffer;
    indicatorBuffer = new Graphics::TBitmap;
    try
    {
        indicatorBuffer->LoadFromResourceName( ( int )HInstance, cArtIndicatorName );
        indicatorBuffer->Transparent = true;
        indicatorBuffer->TransparentMode = tmAuto;

        Resized();
    }
    catch ( const EResNotFound& )
    {
        result = presResNotFoundError;
        gPresErrors.AddError( result, cArtIndicatorName );
    }

    return result;
}

void
TGUIArtifactView::ShowIndicator()
{
    indicatorVisible = true;
    TGUIView::InvalidateRect( indicTRect );
}

void
TGUIArtifactView::HideIndicator()
{
    indicatorVisible = false;
    TGUIView::InvalidateRect( indicTRect );
}


