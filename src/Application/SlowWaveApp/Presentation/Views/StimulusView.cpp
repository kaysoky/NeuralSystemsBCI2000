/////////////////////////////////////////////////////////////////////////////
//
// File: StimulusView.cpp
//
// Date: Nov 16, 2001
//
// Author: Juergen Mellinger
//
// Description: A view that presents a visual or auditory stimulus
//              when ProcessTaskBegin() is called.
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

#include "StimulusView.h"
#include "PresParams.h"
#include "WavePlayer.h"
#include "UParameter.h"

TStimulusView::TStimulusView( PARAMLIST *inParamList )
: TPresView( inParamList ),
  visStimDuration( 0 ),
  wavePlayer( NULL )
{
    PARAM_ENABLE( inParamList, PRVisStimDuration );
}

TStimulusView::~TStimulusView()
{
    PARAM_DISABLE( curParamList, PRVisStimDuration );
    delete wavePlayer;
}

TPresError
TStimulusView::Initialize(          PARAMLIST   *inParamList,
                            const   TGUIRect    &inRect )
{
    viewRect = inRect;
    TGUIView::Resized();

    PARAM_GET_NUM( inParamList, PRVisStimDuration, visStimDuration );

    wavePlayer = new TWavePlayer;

    return presNoError;
}

void
TStimulusView::AttachStimuli( const TTargetSeqEntry &inTargetEntry )
{
    TGUIStimulusView::AttachVisualStimulus( inTargetEntry.visFile.c_str() );
    wavePlayer->AttachFile( inTargetEntry.audioFile.c_str() );
}

void
TStimulusView::ProcessTaskBegin( const TEventArgs& )
{
    wavePlayer->Play();
    TGUIStimulusView::PresentVisualStimulus( visStimDuration );
}

void
TStimulusView::ProcessStopBegin( const TEventArgs& )
{
    if( wavePlayer != NULL )
        wavePlayer->Stop();
    TGUIStimulusView::AttachVisualStimulus( NULL );
}


