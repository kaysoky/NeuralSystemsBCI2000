/////////////////////////////////////////////////////////////////////////////
//
// File: ScoreView.cpp
//
// Date: Nov 8, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes:
//
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "ScoreView.h"
#include "PresParams.h"
#include "UParameter.h"

TScoreView::TScoreView( PARAMLIST   *inParamList )
: TPresView( inParamList ),
  scoreViewMode( 0 )
{
    artifact.AttachOptionalState( "Artifact", 0 );
    PARAM_ENABLE( inParamList, PRScoreViewMode );
    PARAM_ENABLE( inParamList, PRScoreRect );
    PARAM_ENABLE( inParamList, PRScoreFont );
}

TScoreView::~TScoreView()
{
    PARAM_DISABLE( curParamList, PRScoreViewMode );
    PARAM_DISABLE( inParamList, PRScoreRect );
    PARAM_DISABLE( inParamList, PRScoreFont );
}

TPresError
TScoreView::Initialize(         PARAMLIST   *inParamList,
                        const   TGUIRect    &inRect )
{
    TPresError  err = presNoError;

    viewRect = inRect;
    PARAM   *paramPtr;
    PARAM_GET_PTR( inParamList, PRScoreRect, paramPtr );
    err = viewRect.ReadFromParam( paramPtr );
    if( err != presNoError )
        return err;

    PARAM_GET_PTR( inParamList, PRScoreFont, paramPtr );
    err = SetTextProperties( paramPtr );
    if( err != presNoError )
        return err;

    TGUIScoreView::Resized();

    PARAM_GET_NUM( inParamList, PRScoreViewMode, scoreViewMode );
    switch( scoreViewMode )
    {
        case 0: // off
            TGUIScoreView::Hide();
            break;
        case 1: // on
            TGUIScoreView::Show();
            break;
        default:
            assert( false );
    }

    successTrials = 0;
    totalTrials = 0;

    return presNoError;
}

TPresError
TScoreView::SetRect( const PARAM    *inParamPtr )
{
    TPresError err = viewRect.ReadFromParam( inParamPtr );
    if( err == presNoError )
        TGUIScoreView::Resized();
    return presNoError;
}

TPresError
TScoreView::SetTextProperties( const PARAM  *inParamPtr )
{
    TPresError  err = TTextProperties::SetTextProperties( inParamPtr );
    TGUIScoreView::Resized();
    return err;
}

void
TScoreView::ProcessEndOfClass(  const TEventArgs& )
{
    if( artifact.GetStateValue() == 0 )
    {
        totalTrials++;
        TGUIScoreView::InvalidateBuffer();
    }
}

void
TScoreView::ProcessSuccess( const TEventArgs& )
{
    successTrials++;
    TGUIScoreView::InvalidateBuffer();
}

void
TScoreView::ProcessBeginOfTrial( const TEventArgs& )
{
#ifdef ITI_BLANK_SCREEN
    switch( scoreViewMode )
    {
        case 0: // off
            break;
        case 1: // on
            TGUIScoreView::Show();
            break;
        default:
            assert( false );
    }
#endif
}

void
TScoreView::ProcessItiBegin( const TEventArgs& )
{
#ifdef ITI_BLANK_SCREEN
    TGUIScoreView::Hide();
#endif
}
