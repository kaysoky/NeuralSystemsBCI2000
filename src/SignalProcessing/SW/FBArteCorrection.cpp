////////////////////////////////////////////////////////////////////////////////
//
// File: FBArteCorrection.cpp
//
// Description: Slow Wave Class Definition
//              written by Dr. Thilo Hinterberger 2000-2001
//              Copyright University of Tuebingen, Germany
//
// Changes:     2003, juergen.mellinger@uni-tuebingen.de: some bugs fixed.
//              Feb 8, 2004, jm: Adaptations to changes in BCI2000 framework,
//              minor reformulations, reformatting.
//              Feb 24, 2004, jm: Moved the TFBArteCorrection class into
//              separate files.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "FBArteCorrection.h"

#include "UBCIError.h"
#include <cmath>
#include <cassert>

#define SECTION "Filtering"

using namespace std;

RegisterFilter( TFBArteCorrection, 2.D2 );

TFBArteCorrection::TFBArteCorrection()
: mVisualize( 0 ),
  mVis( SOURCEID::Artefact, VISTYPE::GRAPH ),
  mArteMode( off )
{
  BEGIN_PARAMETER_DEFINITIONS
    SECTION " intlist ArteChList= 2 2 0"
      " 2 0 0 // Association of artefact channels with signal channels, 0 for no artefact channel",
    SECTION " floatlist ArteFactorList= 2 0.15 0"
      " 0 0 0 // Influence of artefact channel on input channel, -1: no artifact channel",
    SECTION " int ArteMode= 0"
      " 1 0 3 // Artefact correction mode: "
        "0 off, "
        "1 linear subtraction, "
        "2 subtraction if supporting, "
        "3 subtraction w/abort "
        "(enumeration)",
    "Visualize int VisualizeFBArteCorFiltering= 0"
      " 0 0 1 // visualize FBArte corrected signals (0=no 1=yes) (boolean)",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "Artifact 1 0 0 0",
  END_STATE_DEFINITIONS
}

void
TFBArteCorrection::Preflight( const SignalProperties& inSignalProperties,
                                    SignalProperties& outSignalProperties ) const
{
  for( size_t i = 0; i < Parameter( "ArteChList" )->GetNumValues(); ++i )
  {
    PreflightCondition( Parameter( "ArteChList", i ) >= 0 );
    PreflightCondition( Parameter( "ArteChList", i ) <= inSignalProperties.Channels() );
  }
  outSignalProperties = inSignalProperties;
}

void
TFBArteCorrection::Initialize()
{
  mArteChList.clear();
  for( size_t i = 0; i < Parameter( "ArteChList" )->GetNumValues(); ++i )
    mArteChList.push_back( Parameter( "ArteChList", i ) - 1 );

  mArteFactorList.clear();
  for( size_t i = 0; i < Parameter( "ArteFactorList" )->GetNumValues(); ++i )
    mArteFactorList.push_back( Parameter( "ArteFactorList", i ) );

  mArteMode = static_cast<ArteMode>( ( int )Parameter( "ArteMode" ) );
  mVisualize = Parameter( "VisualizeFBArteCorFiltering" );
  if( mVisualize )
  {
    mVis.Send( CFGID::WINDOWTITLE, "Artefact filtered" );
    mVis.Send( CFGID::MINVALUE, -100 );
    mVis.Send( CFGID::MAXVALUE, 100 );
    mVis.Send( CFGID::NUMSAMPLES, 256 );
  }
}

void
TFBArteCorrection::Process( const GenericSignal* inSignal, GenericSignal* outSignal )
{
  *outSignal = *inSignal;
  GenericSignal& ioSignal = *outSignal;

  for( size_t i = 0; i < mArteChList.size(); ++i )
    if( mArteChList[ i ] >= 0 )
      for( size_t j = 0; j < ioSignal.Elements(); ++j )
      {
        float controlSignal = ioSignal( i, j ),
              arteSignal = ioSignal( mArteChList[ i ], j ) * mArteFactorList[ i ];

        switch( mArteMode )
        {
          case off:
            break;

          case linearSubtraction:
            // Linear subtraction.
            ioSignal( i, j ) = controlSignal - arteSignal;
            break;

          case subtractionIfSupporting:
          case subtractionWithAbort:
            if( arteSignal * controlSignal > 0 )
            { // If they have same signs:
              if( ::fabs( arteSignal ) < ::fabs( controlSignal ) )
              // If artefact is not too big, correct the signal.
                ioSignal( i, j ) = controlSignal - arteSignal;
              else
              { // Artefact is too big.
                ioSignal( i, j ) = 0; // FB is supressed.(?)
                if( mArteMode == subtractionWithAbort )
                  State( "Artifact" ) = 1;
              }
            }
            break;

          default:
            assert( false );
        }
      }

  if( mVisualize )
    mVis.Send( &ioSignal );
}

