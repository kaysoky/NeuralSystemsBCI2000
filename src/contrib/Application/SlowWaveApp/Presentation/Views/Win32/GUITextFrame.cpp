//////////////////////////////////////////////////////////////////////////////
//
// File: GUITextFrame.cpp
//
// Date: Nov 12, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUITextFrame implements the GUI specific details
//              of the text frame class.
//
// Changes:
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "OSIncludes.h"

#ifndef VCL
# error This is the VCL implementation of TGUITextFrame.
#endif

#include <string>

#include "GUITextFrame.h"
#include "Utils/Util.h"

TGUITextFrame::TGUITextFrame()
: TGUIView( textFrameViewZ ),
  memo( NULL )
{
    memo = new TMemo( ( TComponent* )NULL );
    memo->Parent = GetForm();
    memo->BiDiMode = bdLeftToRight; // We try to handle bidirectional input ourselves.
}

TGUITextFrame::~TGUITextFrame()
{
    delete memo;
}

void
TGUITextFrame::Paint()
{
    // The TMemo field will take care of itself.
}

void
TGUITextFrame::Resized()
{
    TInputSequence currentContent = GetText();
    TGUIView::Resized();
    if( biDi != lr )
      memo->WordWrap = false;
    else
      memo->WordWrap = true;
    TFont   *font = memo->Font;
    font->Name = fontName;
    font->Size = MAX( 1, TGUIView::GetForm()->ClientHeight * fontSize );
    font->Style = fontStyle;
    font->Color = fontColor;
    memo->Alignment = textAlignment;
    memo->Color = TGUIView::GetElementColor( textFrameBackground ).cl;
    memo->BoundsRect = viewTRect;
    SetText( currentContent );
}

void
TGUITextFrame::Bell() const
{
    Beep();
}

void
TGUITextFrame::SetText( const TInputSequence& inText )
{
    memo->Text = InputToDisplay( inText ).c_str();
    switch( biDi )
    {
      case lr:
      default:
        memo->SelStart = memo->Text.Length();
        SendMessage( memo->Handle, WM_VSCROLL, SB_BOTTOM, 0 );
        break;
      case rl:
      case bd:
        memo->SelStart = 0;
        SendMessage( memo->Handle, WM_HSCROLL, SB_TOP, 0 );
        break;
    }
}


const TInputSequence&
TGUITextFrame::GetText() const
{
    return DisplayToInput( memo->Text.c_str() );
}

TPresError
TGUITextFrame::LoadText( const char *inFileName )
{
    TPresError  retVal = presNoError;
    try
    {
        memo->Lines->LoadFromFile( inFileName );
    }
    catch( ... )
    {
        retVal = presFileOpeningError;
    }
    return retVal;
}

TPresError
TGUITextFrame::SaveText( const char *inFileName )
{
    TPresError  retVal = presNoError;
    try
    {
        memo->Lines->SaveToFile( inFileName );
    }
    catch( ... )
    {
        retVal = presFileOpeningError;
    }
    return retVal;
}

void
TGUITextFrame::ShowCursor()
{
    TGUIView::GetForm()->ActiveControl = memo;
}

void
TGUITextFrame::HideCursor()
{
    TForm   *form = TGUIView::GetForm();
    form->ActiveControl = form;
}

