/////////////////////////////////////////////////////////////////////////////
//
// File: BackgroundView.cpp
//
// Date: Nov 8, 2001
//
// Author: Juergen Mellinger
//
// Description:
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

#include "BackgroundView.h"

TBackgroundView::TBackgroundView( ParamList *inParamList )
: TPresView( inParamList )
{
}

TBackgroundView::~TBackgroundView()
{
}

TPresError
TBackgroundView::Initialize(            ParamList   *inParamList,
                                const   TGUIRect    &inRect )
{
    viewRect = inRect;
    TGUIView::Resized();
    return presNoError;
}

