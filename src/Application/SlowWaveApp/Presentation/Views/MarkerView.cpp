/////////////////////////////////////////////////////////////////////////////
//
// File: MarkerView.cpp
//
// Date: Nov 8, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes: Feb 16, 2003, jm: Introduced TGUIMarkerView for
//          Zero Bar / Fixation Cross display.
//          May 13, 2003, jm: Introduced multiple auditory markers
//          at arbitrary temporal offsets.
//
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "MarkerView.h"
#include "PresParams.h"
#include "MidiPlayer.h"
#include "UParameter.h"
#include "Utils/Util.h"
#ifdef BCI2000
# include "MeasurementUnits.h"
#endif

TMarkerView::TMarkerView( PARAMLIST *inParamList )
: TPresView( inParamList ),
  visMarker( 0 ),
  currentMarker( audMarkers.begin() ),
  lastTimeOffset( 0.0 )
{
    PARAM_ENABLE( inParamList, PRVisMarker );
    PARAM_ENABLE( inParamList, PRAudMarkers );
    PARAM_ENABLE( inParamList, PRGMMarkerInstruments );
    PARAM_ENABLE( inParamList, PRGMMarkerVolumes );
    PARAM_ENABLE( inParamList, PRGMMarkerNotes );
}

TMarkerView::~TMarkerView()
{
    PARAM_DISABLE( curParamList, PRVisMarker );
    PARAM_DISABLE( curParamList, PRAudMarkers );
    PARAM_DISABLE( curParamList, PRGMMarkerInstruments );
    PARAM_DISABLE( curParamList, PRGMMarkerVolumes );
    PARAM_DISABLE( curParamList, PRGMMarkerNotes );
    ClearMarkers();
}

TPresError
TMarkerView::Initialize(       PARAMLIST *inParamList,
                         const TGUIRect  &inRect )
{
    viewRect = inRect;
    TGUIView::Resized();

    PARAM_GET_NUM( inParamList, PRVisMarker, visMarker );
    TGUIMarkerView::SetType( visMarker );
    TGUIMarkerView::Show();

    ClearMarkers();
    PARAM* audMarkersParam;
    size_t numMarkers = 0;
    PARAM_GET_PTR( inParamList, PRAudMarkers, audMarkersParam );
    if( audMarkersParam != NULL )
      numMarkers = audMarkersParam->GetNumValues();
    int instrument,
        volume,
        note;
    float timeOffset;
    for( size_t i = 0; i < numMarkers; ++i )
    {
      const char* value = audMarkersParam->GetValue( i );
      if( value == NULL )
        value = "";
      const PARAM* param = inParamList->GetParamPtr( value );
      if( param != NULL )
        value = param->GetValue();
#ifdef BCI2000
      timeOffset = MeasurementUnits::ReadAsTime( value );
#else
      timeOffset = ::atof( value );
#endif
      PARAM_GET_NUM_BY_INDEX( inParamList, PRGMMarkerInstruments, i, instrument );
      PARAM_GET_NUM_BY_INDEX( inParamList, PRGMMarkerVolumes, i, volume );
      PARAM_GET_NUM_BY_INDEX( inParamList, PRGMMarkerNotes, i, note );
      audMarker marker;
      marker.timeOffset = timeOffset;
      marker.midiPlayer = new TMidiPlayer( instrument, volume, note );
      audMarkers.push_back( marker );
    }
    audMarkers.sort();
    lastTimeOffset = 0.0;
    currentMarker = audMarkers.begin();

    return presNoError;
}

void
TMarkerView::ProcessTrialActive( const TEventArgs& args )
{
  if( args.timeOffset < lastTimeOffset )
    currentMarker = audMarkers.begin();
  lastTimeOffset = args.timeOffset;
  
  while( currentMarker != audMarkers.end()
         && args.timeOffset >= currentMarker->timeOffset )
  {
    currentMarker->midiPlayer->Play();
    ++currentMarker;
  }
}

void
TMarkerView::ProcessBeginOfTrial( const TEventArgs& )
{
#ifdef ITI_BLANK_SCREEN
    TGUIMarkerView::Show();
#endif
}

void
TMarkerView::ProcessItiBegin( const TEventArgs& )
{
#ifdef ITI_BLANK_SCREEN
    TGUIMarkerView::Hide();
#endif
}

void
TMarkerView::ProcessStopBegin( const TEventArgs& )
{
  for( audMarkerContainer::iterator i = audMarkers.begin(); i != audMarkers.end(); ++i )
    i->midiPlayer->StopSequence();
}

void
TMarkerView::ClearMarkers()
{
    for( audMarkerContainer::iterator i = audMarkers.begin(); i != audMarkers.end(); ++i )
      delete i->midiPlayer;
    audMarkers.clear();
}

