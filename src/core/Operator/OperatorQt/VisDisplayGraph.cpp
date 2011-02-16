////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class for graph type visualization windows.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "VisDisplayGraph.h"

#include "ColorListChooser.h"

#include <limits>

#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QColorDialog>
#include <QPaintEvent>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>

using namespace std;

VisDisplayGraph::VisDisplayGraph( const std::string& inSourceID )
: VisDisplayBase( inSourceID ),
  mpContextMenu( NULL ),
  mpHPMenu( NULL ),
  mpLPMenu( NULL ),
  mpNotchMenu( NULL ),
  mpActEnlargeSignal( NULL ),
  mpActReduceSignal( NULL ),
  mpActFewerSamples( NULL ),
  mpActMoreSamples( NULL ),
  mpActMoreChannels( NULL ),
  mpActFewerChannels( NULL ),
  mpActToggleDisplayMode( NULL ),
  mpActToggleBaselines( NULL ),
  mpActToggleValueUnit( NULL ),
  mpActToggleChannelLabels( NULL ),
  mpActToggleColor( NULL ),
  mpActInvertDisplay( NULL ),
  mpActChooseColors( NULL ),
  mNumChannels( 0 ),
  mSignalElements( 0 ),
  mUserScaling( 0 ),
  mUserZoom( 0 )
{
  this->setAttribute( Qt::WA_NoSystemBackground, true );
  BuildContextMenu();
  Restore();
}

VisDisplayGraph::~VisDisplayGraph()
{
  Save();
  delete mpContextMenu;
}

void
VisDisplayGraph::SetConfig( ConfigSettings& inConfig )
{
  VisDisplayBase::SetConfig( inConfig );

  float minValue = 0,
        maxValue = 0;
  if( inConfig.Get( CfgID::MinValue, minValue ) )
    mDisplay.SetMinValue( minValue );
  if( inConfig.Get( CfgID::MaxValue, maxValue ) )
    mDisplay.SetMaxValue( maxValue );

  int userScaling = mUserScaling;
  mUserScaling = 0;
  for( int i = 0; i < userScaling; ++i )
    EnlargeSignal();
  for( int i = 0; i > userScaling; --i )
    ReduceSignal();

  size_t numSamples = NominalDisplaySamples();
  if( inConfig.Get( CfgID::NumSamples, numSamples ) )
    SetNominalDisplaySamples( numSamples );
  int userZoom = mUserZoom;
  mUserZoom = 0;
  for( int i = 0; i < userZoom; ++i )
    FewerSamples();
  for( int i = 0; i > userZoom; --i )
    MoreSamples();

  size_t channelGroupSize = mDisplay.ChannelGroupSize();
  if( inConfig.Get( CfgID::ChannelGroupSize, channelGroupSize ) )
  {
    if( channelGroupSize < 1 )
      channelGroupSize = numeric_limits<size_t>::max();
    mDisplay.SetChannelGroupSize( channelGroupSize );
  }
  int graphType = mDisplay.DisplayMode();
  if( inConfig.Get( CfgID::GraphType, graphType ) )
    switch( graphType )
    {
      case CfgID::Polyline:
        mDisplay.SetDisplayMode( SignalDisplay::polyline );
        break;
      case CfgID::Field2d:
        mDisplay.SetDisplayMode( SignalDisplay::field2d );
        break;
    }
  bool showBaselines = mDisplay.BaselinesVisible();
  if( inConfig.Get( CfgID::ShowBaselines, showBaselines ) )
    mDisplay.SetBaselinesVisible( showBaselines );

  bool invertedDisplay = mDisplay.Inverted();
  if( inConfig.Get( CfgID::InvertedDisplay, invertedDisplay ) )
    mDisplay.SetInverted( invertedDisplay );

  ColorList channelColors = mDisplay.ChannelColors();
  if( inConfig.Get( CfgID::ChannelColors, channelColors ) )
    mDisplay.SetChannelColors( channelColors );

  string unit;
  istringstream iss;
  if( inConfig.Get( CfgID::SampleUnit, unit ) )
  {
    double oldSampleUnit
     = NominalUnitsPerSample() * FilterUnitToValue( mDisplay.SampleUnit() );
    iss.clear();
    iss.str( unit );
    float unitsPerSample = 1;
    string sampleUnit = "";
    iss >> unitsPerSample >> sampleUnit;
    SetNominalUnitsPerSample( unitsPerSample );
    mDisplay.SetSampleUnit( sampleUnit );
    if( oldSampleUnit != unitsPerSample * FilterUnitToValue( sampleUnit ) )
    {
      mDisplayFilter.HPCorner( 0 );
      mDisplayFilter.LPCorner( 0 );
      mDisplayFilter.NotchCenter( 0 );
    }
  }
  int sampleOffset;
  if( inConfig.Get( CfgID::SampleOffset, sampleOffset ) )
    mDisplay.SetSampleOffset( sampleOffset );

  if( inConfig.Get( CfgID::ChannelUnit, unit ) )
  {
    iss.clear();
    iss.str( unit );
    float unitsPerChannel = 1;
    string channelUnit = "";
    iss >> unitsPerChannel >> channelUnit;
    mDisplay.SetUnitsPerChannel( unitsPerChannel ).SetChannelUnit( channelUnit );
  }

  if( inConfig.Get( CfgID::ValueUnit, unit ) )
  {
    iss.clear();
    iss.str( unit );
    float unitsPerValue = 1;
    string valueUnit = "";
    iss >> unitsPerValue >> valueUnit;
    mDisplay.SetUnitsPerValue( unitsPerValue ).SetValueUnit( valueUnit );
  }
  bool showValueUnit = mDisplay.ValueUnitVisible();
  if( inConfig.Get( CfgID::ShowValueUnit, showValueUnit ) )
    mDisplay.SetValueUnitVisible( showValueUnit );

  LabelList labels = mDisplay.XAxisMarkers();
  if( inConfig.Get( CfgID::XAxisMarkers, labels ) )
    mDisplay.SetXAxisMarkers( labels );
  labels = mDisplay.ChannelLabels();
  if( inConfig.Get( CfgID::ChannelLabels, labels ) )
  {
    mDisplay.SetChannelLabels( labels );
    mDisplay.SetChannelLabelsVisible( !labels.empty() );
  }

  // Sanity checks.
  if( mDisplay.MinValue() == mDisplay.MaxValue() )
    mDisplay.SetMaxValue( mDisplay.MinValue() + 1 );
}

void
VisDisplayGraph::Restore()
{
  VisDisplayBase::Restore();
  SyncDisplay();
  this->show();
}

void
VisDisplayGraph::Save() const
{
  VisDisplayBase::Save();
}

void
VisDisplayGraph::HandleSignal( const GenericSignal& s )
{
  if( s.Channels() < 1 || s.Elements() < 1 )
    return;

  int curChannels = mNumChannels,
      curSamples = mDisplay.NumSamples();

  // Apply the visualization filter.
  GenericSignal filteredSignal( s.Properties() );
  mDisplayFilter.Process( s, filteredSignal );

  mNumChannels = filteredSignal.Channels();

  // Apply the decimation filter.
  GenericSignal decimatedSignal( 0, 0 );
  mDecimationFilter.Process( filteredSignal, decimatedSignal );

  mDisplay.WrapForward( decimatedSignal );
  mSignalElements = s.Elements();

  if( curChannels != mNumChannels || curSamples != mDisplay.NumSamples() )
    SetConfig( Visconfigs()[ mSourceID ] );
}

void
VisDisplayGraph::BuildContextMenu()
{
  mpContextMenu = new QMenu;
  mpActEnlargeSignal = mpContextMenu->addAction( tr("Enlarge Signal"), this, SLOT(EnlargeSignal()) );
  mpActReduceSignal = mpContextMenu->addAction( tr("Reduce Signal"), this, SLOT(ReduceSignal()) );
  mpContextMenu->addSeparator();

  mpActFewerSamples = mpContextMenu->addAction( tr("Fewer Samples"), this, SLOT(FewerSamples()) );
  mpActMoreSamples = mpContextMenu->addAction( tr("More Samples"), this, SLOT(MoreSamples()) );
  mpContextMenu->addSeparator();

  mpActMoreChannels = mpContextMenu->addAction( tr("More Channels"), this, SLOT(MoreChannels()) );
  mpActFewerChannels = mpContextMenu->addAction( tr("Fewer Channels"), this, SLOT(FewerChannels()) );
  mpContextMenu->addSeparator();

  mpActToggleDisplayMode = mpContextMenu->addAction( tr("Toggle Display Mode"), this, SLOT(ToggleDisplayMode()) );
  mpActToggleColor = mpContextMenu->addAction( tr("Color Display"), this, SLOT(ToggleColor()) );
  mpActToggleColor->setCheckable( true );
  mpActInvertDisplay = mpContextMenu->addAction( tr("Invert"), this, SLOT(InvertDisplay()) );
  mpActInvertDisplay->setCheckable( true );
  mpActChooseColors = mpContextMenu->addAction( tr("Choose Channel Colors..."), this, SLOT(ChooseColors()) );
  mpContextMenu->addSeparator();

  mpActToggleBaselines = mpContextMenu->addAction( tr("Show Baselines"), this, SLOT(ToggleBaselines()) );
  mpActToggleBaselines->setCheckable( true );
  mpActToggleValueUnit = mpContextMenu->addAction( tr("Show Unit"), this, SLOT(ToggleValueUnit()) );
  mpActToggleValueUnit->setCheckable( true );
  mpActToggleChannelLabels = mpContextMenu->addAction( tr("Show Legend"), this, SLOT(ToggleChannelLabels()) );
  mpActToggleChannelLabels->setCheckable( true );
  mpContextMenu->addSeparator();

  mpHPMenu = new QMenu( tr("High Pass") );
  QActionGroup* pHPGroup = new QActionGroup( mpHPMenu );
  QAction* pAct = mpHPMenu->addAction( tr("off"), this, SLOT(SetHPOff()) );
  pAct->setCheckable( true );
  pHPGroup->addAction( pAct );
  pAct->setChecked( true );
  pAct = mpHPMenu->addAction( "0.1Hz", this, SLOT(SetHP01()) );
  pAct->setCheckable( true );
  pHPGroup->addAction( pAct );
  pAct = mpHPMenu->addAction( "1Hz", this, SLOT(SetHP1()) );
  pAct->setCheckable( true );
  pHPGroup->addAction( pAct );
  pAct = mpHPMenu->addAction( "5Hz", this, SLOT(SetHP5()) );
  pAct->setCheckable( true );
  pHPGroup->addAction( pAct );
  mpContextMenu->addMenu( mpHPMenu );

  mpLPMenu = new QMenu( tr("Low Pass") );
  QActionGroup* pLPGroup = new QActionGroup( mpLPMenu );
  pAct = mpLPMenu->addAction( tr("off"), this, SLOT(SetLPOff()) );
  pAct->setCheckable( true );
  pAct->setChecked( true );
  pLPGroup->addAction( pAct );
  pAct = mpLPMenu->addAction( "30Hz", this, SLOT(SetLP30()) );
  pAct->setCheckable( true );
  pLPGroup->addAction( pAct );
  pAct = mpLPMenu->addAction( "40Hz", this, SLOT(SetLP40()) );
  pAct->setCheckable( true );
  pLPGroup->addAction( pAct );
  pAct = mpLPMenu->addAction( "70Hz", this, SLOT(SetLP70()) );
  pAct->setCheckable( true );
  pLPGroup->addAction( pAct );
  mpContextMenu->addMenu( mpLPMenu );

  mpNotchMenu = new QMenu( tr("Notch") );
  QActionGroup* pNotchGroup = new QActionGroup( mpNotchMenu );
  pAct = mpNotchMenu->addAction( tr("off"), this, SLOT(SetNotchOff()) );
  pAct->setCheckable( true );
  pAct->setChecked( true );
  pNotchGroup->addAction( pAct );
  pAct = mpNotchMenu->addAction( "50Hz", this, SLOT(SetNotch50()) );
  pAct->setCheckable( true );
  pNotchGroup->addAction( pAct );
  pAct = mpNotchMenu->addAction( "60Hz", this, SLOT(SetNotch60()) );
  pAct->setCheckable( true );
  pNotchGroup->addAction( pAct );
  mpContextMenu->addMenu( mpNotchMenu );

  this->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ContextMenu(QPoint)) );
}

void
VisDisplayGraph::ContextMenu( const QPoint& inP )
{
  mpActEnlargeSignal->setEnabled( EnlargeSignal_Enabled() );
  mpActReduceSignal->setEnabled( ReduceSignal_Enabled() );

  mpActFewerSamples->setEnabled( FewerSamples_Enabled() );
  mpActMoreSamples->setEnabled( MoreSamples_Enabled() );

  mpActFewerChannels->setEnabled( FewerChannels_Enabled() );
  mpActMoreChannels->setEnabled( MoreChannels_Enabled() );

  mpActToggleColor->setEnabled( ToggleColor_Enabled() );
  mpActToggleColor->setChecked( ToggleColor_Checked() );

  mpActInvertDisplay->setChecked( InvertDisplay_Checked() );

  mpActChooseColors->setEnabled( ChooseColors_Enabled() );

  mpActToggleBaselines->setEnabled( ToggleBaselines_Enabled() );
  mpActToggleBaselines->setChecked( ToggleBaselines_Checked() );

  mpActToggleValueUnit->setEnabled( ToggleValueUnit_Enabled() );
  mpActToggleValueUnit->setChecked( ToggleValueUnit_Checked() );

  mpActToggleChannelLabels->setEnabled( ToggleChannelLabels_Enabled() );
  mpActToggleChannelLabels->setChecked( ToggleChannelLabels_Checked() );

  mpHPMenu->setEnabled( Filter_Enabled() );
  mpLPMenu->setEnabled( Filter_Enabled() );
  mpNotchMenu->setEnabled( Filter_Enabled() );

  mpContextMenu->exec( this->mapToGlobal( inP ) );
}

void
VisDisplayGraph::ToggleDisplayMode()
{
  mDisplay.SetDisplayMode(
    ( mDisplay.DisplayMode() + 1 ) % SignalDisplay::numDisplayModes
  );
}

void
VisDisplayGraph::ToggleBaselines()
{
  mDisplay.SetBaselinesVisible( !mDisplay.BaselinesVisible() );
  Visconfigs()[ mSourceID ].Put( CfgID::ShowBaselines, mDisplay.BaselinesVisible(), UserDefined );
}

bool
VisDisplayGraph::ToggleBaselines_Enabled() const
{
  return mDisplay.DisplayMode() == SignalDisplay::polyline;
}

bool
VisDisplayGraph::ToggleBaselines_Checked() const
{
  return mDisplay.BaselinesVisible();
}

void
VisDisplayGraph::ToggleValueUnit()
{
  mDisplay.SetValueUnitVisible( !mDisplay.ValueUnitVisible() );
  Visconfigs()[ mSourceID ].Put( CfgID::ShowValueUnit, mDisplay.ValueUnitVisible(), UserDefined );
}

bool
VisDisplayGraph::ToggleValueUnit_Enabled() const
{
  return mDisplay.DisplayMode() == SignalDisplay::polyline;
}

bool
VisDisplayGraph::ToggleValueUnit_Checked() const
{
  return mDisplay.ValueUnitVisible();
}

void
VisDisplayGraph::ToggleChannelLabels()
{
  mDisplay.SetChannelLabelsVisible( !mDisplay.ChannelLabelsVisible() );
}

bool
VisDisplayGraph::ToggleChannelLabels_Enabled() const
{
  return !mDisplay.ChannelLabels().empty();
}

bool
VisDisplayGraph::ToggleChannelLabels_Checked() const
{
  return mDisplay.ChannelLabelsVisible();
}

void
VisDisplayGraph::ToggleColor()
{
  mDisplay.SetColorDisplay( !mDisplay.ColorDisplay() );
}

bool
VisDisplayGraph::ToggleColor_Enabled() const
{
  return mDisplay.DisplayMode() == SignalDisplay::polyline
         || mDisplay.DisplayMode() == SignalDisplay::field2d;
}

bool
VisDisplayGraph::ToggleColor_Checked() const
{
  return mDisplay.ColorDisplay();
}

void
VisDisplayGraph::InvertDisplay()
{
  mDisplay.SetInverted( !mDisplay.Inverted() );
  Visconfigs()[ mSourceID ].Put( CfgID::InvertedDisplay, mDisplay.Inverted(), UserDefined );
}

bool
VisDisplayGraph::InvertDisplay_Checked() const
{
  return mDisplay.Inverted();
}

void
VisDisplayGraph::ChooseColors()
{
  // The dialog's "custom colors" are used to hold channel colors.
  ColorList channelColors = mDisplay.ChannelColors();
  ColorListChooser().SetPrompt( "Choose channel colors" )
                    .Display( channelColors );
  mDisplay.SetChannelColors( channelColors );
  Visconfigs()[ mSourceID ].Put( CfgID::ChannelColors, channelColors, UserDefined );
}

bool
VisDisplayGraph::ChooseColors_Enabled() const
{
  return mDisplay.ColorDisplay()
         && mDisplay.DisplayMode() == SignalDisplay::polyline;
}

void
VisDisplayGraph::EnlargeSignal()
{
  mDisplay.SetMinValue( mDisplay.MinValue() / 2 );
  mDisplay.SetMaxValue( mDisplay.MaxValue() / 2 );
  ++mUserScaling;
}

bool
VisDisplayGraph::EnlargeSignal_Enabled() const
{
  return mUserScaling < cMaxUserScaling;
}

void
VisDisplayGraph::ReduceSignal()
{
  mDisplay.SetMinValue( mDisplay.MinValue() * 2 );
  mDisplay.SetMaxValue( mDisplay.MaxValue() * 2 );
  --mUserScaling;
}

bool
VisDisplayGraph::ReduceSignal_Enabled() const
{
  return mUserScaling > -cMaxUserScaling;
}

void
VisDisplayGraph::FewerSamples()
{
  SetNominalDisplaySamples( NominalDisplaySamples() / 2 );
  ++mUserZoom;
}

bool
VisDisplayGraph::FewerSamples_Enabled() const
{
  return ( 2 * mSignalElements <= NominalDisplaySamples() ) && ( mUserZoom < cMaxUserScaling );
}

void
VisDisplayGraph::MoreSamples()
{
  SetNominalDisplaySamples( NominalDisplaySamples() * 2 );
  --mUserZoom;
}

bool
VisDisplayGraph::MoreSamples_Enabled() const
{
  return ( 2 * mSignalElements <= NominalDisplaySamples() ) && ( mUserZoom > -cMaxUserScaling );
}

void
VisDisplayGraph::FewerChannels()
{
  // Round down to the nearest power of 2.
  int n = mDisplay.DisplayGroups(),
      roundedN = 1;
  while( n >>= 1 )
    roundedN <<= 1;

  int newGroups = roundedN;
  if( newGroups == mDisplay.DisplayGroups() )
    newGroups /= 2;
  mDisplay.SetDisplayGroups( newGroups );
}

bool
VisDisplayGraph::FewerChannels_Enabled() const
{
  return mDisplay.DisplayGroups() > 1;
}

void
VisDisplayGraph::MoreChannels()
{
  int maxDisplayGroups = mNumChannels / mDisplay.ChannelGroupSize(),
      newDisplayGroups = min( maxDisplayGroups, mDisplay.DisplayGroups() * 2 );
  mDisplay.SetDisplayGroups( newDisplayGroups );
}

bool
VisDisplayGraph::MoreChannels_Enabled() const
{
  return mDisplay.DisplayGroups() < mNumChannels / mDisplay.ChannelGroupSize();
}

double
VisDisplayGraph::FilterCaptionToValue( const char* inCaption ) const
{
  const char* p = inCaption;
  while( *p && !::isdigit( *p ) )
    ++p;
  return ::atof( p ) * NominalUnitsPerSample()
                     * FilterUnitToValue( mDisplay.SampleUnit() );
}

double
VisDisplayGraph::FilterUnitToValue( const string& inUnit ) const
{
  if( inUnit.length() < 1 || *inUnit.rbegin() != 's' )
    return 0;

  return PhysicalUnit().SetOffset( 0 ).SetGain( 1 ).SetSymbol( "s" )
                       .PhysicalToRaw( string( "1" ) + inUnit );
}

int
VisDisplayGraph::NominalDisplaySamples() const
{
  return mDisplay.NumSamples() * mDecimationFilter.Decimation();
}

void
VisDisplayGraph::SetNominalDisplaySamples( int inSamples )
{
  if( inSamples != NominalDisplaySamples() )
  {
    mDisplay.WrapForward( GenericSignal( 0, 0 ) );
    
    double nominalUnitsPerSample = NominalUnitsPerSample();
    int actualSamples = inSamples,
        decimation = 1;
    int screenWidth = QApplication::desktop()->screenGeometry().width();
    if( screenWidth < inSamples )
    {
      decimation = ::ceil( static_cast<float>( inSamples ) / screenWidth / cDisplayOversampling );
      actualSamples = inSamples / decimation;
    }
    mDisplay.SetNumSamples( actualSamples );
    mDecimationFilter.Decimation( decimation );
    SetNominalUnitsPerSample( nominalUnitsPerSample );
  }
}

double
VisDisplayGraph::NominalUnitsPerSample() const
{
  return mDisplay.UnitsPerSample() / mDecimationFilter.Decimation();
}

void
VisDisplayGraph::SetNominalUnitsPerSample( double inUps )
{
  mDisplay.SetUnitsPerSample( inUps * mDecimationFilter.Decimation() );
}

bool
VisDisplayGraph::Filter_Enabled() const
{
  return NominalUnitsPerSample() * FilterUnitToValue( mDisplay.SampleUnit() ) != 0;
}

void
VisDisplayGraph::SetHPOff()
{
  mDisplayFilter.HPCorner( 0 );
}

void
VisDisplayGraph::SetHP01()
{
  mDisplayFilter.HPCorner( FilterCaptionToValue( "0.1Hz" ) );
}

void
VisDisplayGraph::SetHP1()
{
  mDisplayFilter.HPCorner( FilterCaptionToValue( "1Hz" ) );
}

void
VisDisplayGraph::SetHP5()
{
  mDisplayFilter.HPCorner( FilterCaptionToValue( "5Hz" ) );
}

void
VisDisplayGraph::SetLPOff()
{
  mDisplayFilter.LPCorner( 0 );
}

void
VisDisplayGraph::SetLP30()
{
  mDisplayFilter.LPCorner( FilterCaptionToValue( "30Hz" ) );
}

void
VisDisplayGraph::SetLP40()
{
  mDisplayFilter.LPCorner( FilterCaptionToValue( "40Hz" ) );
}

void
VisDisplayGraph::SetLP70()
{
  mDisplayFilter.LPCorner( FilterCaptionToValue( "70Hz" ) );
}

void
VisDisplayGraph::SetNotchOff()
{
  mDisplayFilter.NotchCenter( 0 );
}

void
VisDisplayGraph::SetNotch50()
{
  mDisplayFilter.NotchCenter( FilterCaptionToValue( "50Hz" ) );
}

void
VisDisplayGraph::SetNotch60()
{
  mDisplayFilter.NotchCenter( FilterCaptionToValue( "60Hz" ) );
}

void
VisDisplayGraph::SyncDisplay()
{
  GUI::DrawContext dc =
  {
    this,
    { 0, 0, this->width(), this->height() }
  };
  mDisplay.SetContext( dc );
}

void
VisDisplayGraph::paintEvent( QPaintEvent* iopEvent )
{
  VisDisplayBase::paintEvent( iopEvent );
  mDisplay.Paint( ( void* )&iopEvent->region() );
  iopEvent->accept();
}

void
VisDisplayGraph::moveEvent( QMoveEvent* iopEvent )
{
  SyncDisplay();
  VisDisplayBase::moveEvent( iopEvent );
  iopEvent->accept();
}

void
VisDisplayGraph::resizeEvent( QResizeEvent* iopEvent )
{
  SyncDisplay();
  VisDisplayBase::resizeEvent( iopEvent );
  iopEvent->accept();
}

void
VisDisplayGraph::keyReleaseEvent( QKeyEvent* iopEvent )
{
  static int acc = 0;
  int wipe_acc = 1;
  int key = iopEvent->key(),
      modkey = iopEvent->modifiers();

  if( key == Qt::Key_Space && ( modkey & Qt::ShiftModifier ) )
    key = Qt::Key_PageUp;
  if( acc == 0 && key == Qt::Key_G && ( modkey & Qt::ShiftModifier ) )
    key = Qt::Key_End;
  if( acc != 0 && key == Qt::Key_Return )
    key = Qt::Key_G;

  switch( key )
  {
    case Qt::Key_Up:
      mDisplay.SetTopGroup( mDisplay.TopGroup() - 1 );
      break;
    case Qt::Key_Down:
      mDisplay.SetTopGroup( mDisplay.TopGroup() + 1 );
      break;
    case Qt::Key_PageUp:
    case Qt::Key_B:
      mDisplay.SetTopGroup( mDisplay.TopGroup() - mDisplay.DisplayGroups() );
      break;
    case Qt::Key_PageDown:
    case Qt::Key_Space:
      mDisplay.SetTopGroup( mDisplay.TopGroup() + mDisplay.DisplayGroups() );
      break;
    case Qt::Key_Right:
      if( FewerSamples_Enabled() )
        FewerSamples();
      break;
    case Qt::Key_Left:
      if( MoreSamples_Enabled() )
        MoreSamples();
      break;
    case Qt::Key_Comma:
      if( FewerChannels_Enabled() )
        FewerChannels();
      break;
    case Qt::Key_Period:
      if( MoreChannels_Enabled() )
        MoreChannels();
      break;
    case Qt::Key_Minus:
      if( ReduceSignal_Enabled() )
        ReduceSignal();
      break;
    case Qt::Key_Plus:
      if( EnlargeSignal_Enabled() )
        EnlargeSignal();
      break;
    case Qt::Key_Home:
      mDisplay.SetTopGroup( 0 );
      break;
    case Qt::Key_End:
      mDisplay.SetTopGroup( mNumChannels / mDisplay.ChannelGroupSize() );
      break;
    case Qt::Key_G:
      mDisplay.SetTopGroup( acc - 1 );
      break;
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
      acc = acc * 10 + ( key - Qt::Key_0 );
      wipe_acc = 0;
      break;
    default:
      VisDisplayBase::keyReleaseEvent( iopEvent );
  }
  if( wipe_acc )
    acc = 0;
}

void
VisDisplayGraph::mousePressEvent( QMouseEvent* iopEvent )
{
  this->activateWindow();
  VisDisplayBase::mousePressEvent( iopEvent );
}
