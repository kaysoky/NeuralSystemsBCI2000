/////////////////////////////////////////////////////////////////////////////
//
// File: TextFrame.cpp
//
// Date: Oct 22, 2001
//
// Author: Juergen Mellinger
//
// Description: A frame to display text. This class is the GUI independent
//              interface used by the other presentation classes.
//              All GUI dependent functions are in the TGUITextFrame
//              class.
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

#include <list>
#include <string>

#include "TextFrame.h"
#include "PresErrors.h"
#include "Utils/Util.h"

using namespace std;

TTextFrame::TTextFrame()
{
}

TTextFrame::~TTextFrame()
{
}

TPresError
TTextFrame::SetRect( const PARAM    *inParamPtr )
{
    TPresError err = viewRect.ReadFromParam( inParamPtr );
    if( err == presNoError )
        TGUITextFrame::Resized();
    return presNoError;
}

TPresError
TTextFrame::SetTextProperties( const PARAM  *inParamPtr )
{
    TPresError  err = TTextProperties::SetTextProperties( inParamPtr );
    TGUITextFrame::Resized();
    return err;
}

void
TTextFrame::SetText( const TInputSequence &inText )
{
    TGUITextFrame::SetText( inText );
}

TInputSequence
TTextFrame::GetText() const
{
    return TGUITextFrame::GetText();
}

void
TTextFrame::AddText( const TInputSequence &inText )
{
    TInputSequence curText = GetText();
    // Interpret ASCII control characters.
    for( TInputSequence::const_iterator i = inText.begin(); i != inText.end(); ++i )
        switch( *i )
        {
            case '\a': // bell
                TGUITextFrame::Bell();
                break;
            case '\b': // backspace
                if( curText.length() > 0 )
                    curText = curText.substr( 0, curText.length() - 1 );
                break;
            default:
                curText += *i;
        }
    SetText( curText );
}

void
TTextFrame::LoadText( const char    *inFileName )
{
    Util::TPath         curPath;
    string              inputFileName = curPath + inFileName;
    if( TGUITextFrame::LoadText( inputFileName.c_str() ) != presNoError )
    {
#if 1
        gPresErrors.AddError( presFileOpeningError, inputFileName.c_str() );
#else
        SetText( "" );
#endif
    }
}

void
TTextFrame::SaveText( const char    *inFileName )
{
    // This is a work around for a strange problem with static
    // class variables being destructed before the application's main form
    // on program exit.
    // (SaveText() is called from ~PresSpellerModel().)
    Util::TPath         curPath( pathBuffer );
    string              outputFileName = curPath + inFileName;
    if( TGUITextFrame::SaveText( outputFileName.c_str() ) != presNoError )
        gPresErrors.AddError( presFileOpeningError, outputFileName.c_str() );
}

void
TTextFrame::ShowCursor()
{
    TGUITextFrame::ShowCursor();
}

void
TTextFrame::HideCursor()
{
    TGUITextFrame::HideCursor();
}


