////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "VisDisplay.h"
#include "defines.h"
#include "GenericVisualization.h"
#include "PhysicalUnit.h"
#include "HierarchicalLabel.h"
#include "GUI.h"

#include <cassert>
#include <Registry.hpp>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <limits>
#include <vcl.h>

using namespace std;

static const char* key_base = KEY_BCI2000 KEY_OPERATOR KEY_VISUALIZATION "\\";

void
VisDisplay::HandleMessage( const VisSignal& v )
{
  Graph::HandleMessage( v );
}

void
VisDisplay::HandleMessage( const VisSignalProperties& v )
{
  VisDisplayBase::HandleMessage( v );
}

void
VisDisplay::HandleMessage( const VisMemo& v )
{
  Memo::HandleMessage( v );
}

void
VisDisplay::HandleMessage( const VisBitmap& v )
{
  Bitmap::HandleMessage( v );
}

void
VisDisplay::HandleMessage( const VisCfg& v )
{
  VisDisplayBase::HandleMessage( v );
}

////////////////////////////////////////////////////////////////////////////////

VisDisplay::VisDisplayBase::VisContainer&
VisDisplay::VisDisplayBase::Visuals()
{
  static VisDisplay::VisDisplayBase::VisContainer visuals;
  return visuals;
}

VisDisplay::VisDisplayBase::ConfigContainer&
VisDisplay::VisDisplayBase::Visconfigs()
{
  static VisDisplay::VisDisplayBase::ConfigContainer visconfigs;
  return visconfigs;
}

////////////////////////////////////////////////////////////////////////////////
static const char* cfgid_prefix = "CFGID"; // const AnsiString cfgid_prefix = "CFGID"; won't work.

void
VisDisplay::VisDisplayBase::ConfigContainer::Save()
{
  AnsiString as_cfgid_prefix = cfgid_prefix;

  for( iterator i = begin(); i != end(); ++i )
  {
    AnsiString key = key_base + AnsiString( i->first.c_str() );
    TRegistry* reg = new TRegistry( KEY_WRITE );
    if( reg->OpenKey( key, true ) )
    {
      TStringList* valueNames = new TStringList;
      reg->GetValueNames( valueNames );
      for( int j = 0; j < valueNames->Count; ++j )
        reg->DeleteValue( valueNames->Strings[ j ] );

      reg->WriteString( "Title", i->second[ CfgID::WindowTitle ].c_str() );
      for( ConfigSettings::iterator j = i->second.begin(); j != i->second.end(); ++j )
        if( i->second.State( j->first ) == UserDefined )
          try
          {
            reg->WriteString( as_cfgid_prefix + AnsiString( j->first ), j->second.c_str() );
          }
          catch( ERegistryException& ) {}
      delete valueNames;
    }
    delete reg;
  }
}

void
VisDisplay::VisDisplayBase::ConfigContainer::Restore()
{
  AnsiString as_cfgid_prefix = cfgid_prefix;

  TRegistry* reg = new TRegistry( KEY_READ );
  if( reg->OpenKeyReadOnly( key_base ) )
  {
     TStringList* keys = new TStringList;
     reg->GetKeyNames( keys );
     for( int i = 0; i < keys->Count; ++i )
     {
       AnsiString key = key_base + keys->Strings[ i ];
       std::string visID = keys->Strings[ i ].c_str();
       if( reg->OpenKeyReadOnly( key ) )
       {
         TStringList* valueNames = new TStringList;
         reg->GetValueNames( valueNames );
         for( int j = 0; j < valueNames->Count; ++j )
         {

           if( valueNames->Strings[ j ].SubString( 0, as_cfgid_prefix.Length() ) == as_cfgid_prefix )
           {
             IDType cfgID = ::atoi( valueNames->Strings[ j ].c_str() + as_cfgid_prefix.Length() );
             try
             {
               ( *this )[ visID ].Put( cfgID,
                                       reg->ReadString( valueNames->Strings[ j ] ).c_str(),
                                       OnceUserDefined );
             }
             catch( ERegistryException& ) {}
           }
         }
         delete valueNames;
       }
     }
     delete keys;
  }
  delete reg;
}

////////////////////////////////////////////////////////////////////////////////
VisDisplay::VisDisplayBase::VisDisplayBase( const std::string& inSourceID )
: mSourceID( inSourceID ),
  mpForm( NULL )
{
  VisDisplayBase* visual = Visuals()[ mSourceID ];
  delete visual;
  Visuals()[ mSourceID ] = this;
}

VisDisplay::VisDisplayBase::~VisDisplayBase()
{
  delete mpForm;
}

void
VisDisplay::VisDisplayBase::VisContainer::Clear()
{
  for( iterator i = begin(); i != end(); ++i )
    delete i->second;
  VisContainerBase::clear();
}

void
VisDisplay::VisDisplayBase::SetConfig( ConfigSettings& inConfig )
{
  mTitle = inConfig[ CfgID::WindowTitle ];
  if( !mTitle.empty() )
    mpForm->Caption = mTitle.c_str();
  else
    mpForm->Caption = mSourceID.c_str();

  // The static variables make each new window appear a little down right
  // to the previous one.
  static int newTop = 10,
             newLeft = 10;
  int formTop = 10,
      formLeft = 10,
      formHeight = 100,
      formWidth = 100;
  bool posDefault  = !inConfig.Get( CfgID::Top, formTop ) ||
                     !inConfig.Get( CfgID::Left, formLeft ),
       sizeDefault = !inConfig.Get( CfgID::Height, formHeight ) ||
                     !inConfig.Get( CfgID::Width, formWidth );
  if( posDefault )
  {
    formTop = newTop;
    newTop += 10;
    formLeft = newLeft;
    newLeft += 10;
  }
  mpForm->Top = formTop;
  mpForm->Left = formLeft;
  mpForm->Height = formHeight;
  mpForm->Width = formWidth;
  if( !PtInRect( mpForm->ClientRect, TPoint( 10, 10 ) ) )
  {
    sizeDefault = true;
    mpForm->Height = 100;
    mpForm->Width = 100;
  }
  if( !PtInRect( Screen->DesktopRect, mpForm->ClientOrigin ) )
  {
    posDefault = true;
    mpForm->Top = newTop;
    newTop += 10;
    mpForm->Left = newLeft;
    newLeft += 10;
  }
  if( posDefault )
  {
    Visconfigs()[ mSourceID ].Put( CfgID::Top, mpForm->Top, Default );
    Visconfigs()[ mSourceID ].Put( CfgID::Left, mpForm->Left, Default );
  }
  if( sizeDefault )
  {
    Visconfigs()[ mSourceID ].Put( CfgID::Width, mpForm->Width, Default );
    Visconfigs()[ mSourceID ].Put( CfgID::Height, mpForm->Height, Default );
  }
  
  bool visible = true;
  inConfig.Get( CfgID::Visible, visible );
  SetVisible( visible );
}

void
VisDisplay::VisDisplayBase::Restore()
{
  assert( mpForm != NULL );
  mpForm->BorderStyle = bsSizeToolWin;
  mpForm->OnSizeMove = FormSizeMove;
  SetConfig( Visconfigs()[ mSourceID ] );
}

void
VisDisplay::VisDisplayBase::Save() const
{
  assert( mpForm != NULL );
}

void
VisDisplay::VisDisplayBase::HandleMessage( const VisCfg& v )
{
  Visconfigs()[ v.SourceID() ].Put( v.CfgID(), v.CfgValue(), MessageDefined );
  if( Visuals()[ v.SourceID() ] != NULL )
    Visuals()[ v.SourceID() ]->SetConfig( Visconfigs()[ v.SourceID() ] );
}

// For now, we treat a VisSignalProperties message as a collection of VisCfg
// messages. In the future, CfgIDs represented in SignalProperties will become
// obsolete to avoid redundancy.
void
VisDisplay::VisDisplayBase::HandleMessage( const VisSignalProperties& v )
{
  const SignalProperties& s = v.SignalProperties();
  if( !s.Name().empty() )
    Visconfigs()[ v.SourceID() ].Put( CfgID::WindowTitle, s.Name(), MessageDefined );

  string channelUnit = s.ChannelUnit().RawToPhysical( s.ChannelUnit().Offset() + 1 );
  Visconfigs()[ v.SourceID() ].Put( CfgID::ChannelUnit, channelUnit, MessageDefined );

  int numSamples = s.ElementUnit().RawMax() - s.ElementUnit().RawMin() + 1;
  Visconfigs()[ v.SourceID() ].Put( CfgID::NumSamples, numSamples, MessageDefined );

  if( numSamples > 0 )
  {
    string symbol;
    double value = s.ElementUnit().Gain() * numSamples;
    int magnitude = ( ::log10( ::fabs( value ) ) + 1.0 ) / 3.0;
    if( magnitude < -3 )
      magnitude = -3;
    if( magnitude > 3 )
      magnitude = 3;
    switch( magnitude )
    {
      case -3:
        symbol = "n";
        value /= 1e-9;
        break;
      case -2:
        symbol = "mu";
        value /= 1e-6;
        break;
      case -1:
        symbol = "m";
        value /= 1e-3;
        break;
      case 0:
        break;
      case 1:
        symbol = "k";
        value /= 1e3;
        break;
      case 2:
        symbol = "M";
        value /= 1e6;
        break;
      case 3:
        symbol = "G";
        value /= 1e9;
        break;
    }
    ostringstream oss;
    oss << setprecision( 10 ) << value / numSamples << symbol << s.ElementUnit().Symbol();
    Visconfigs()[ v.SourceID() ].Put( CfgID::SampleUnit, oss.str(), MessageDefined );
    Visconfigs()[ v.SourceID() ].Put( CfgID::SampleOffset, s.ElementUnit().Offset(), MessageDefined );
  }
  // Although the SignalProperties class allows for individual units for
  // individual channels, the SignalDisplay class is restricted a single
  // unit and range.
  string valueUnit = s.ValueUnit().RawToPhysical( s.ValueUnit().Offset() + 1 );
  Visconfigs()[ v.SourceID() ].Put( CfgID::ValueUnit, valueUnit, MessageDefined );

  float rangeMin = s.ValueUnit().RawMin(),
        rangeMax = s.ValueUnit().RawMax();
  if( rangeMin == rangeMax )
  {
    Visconfigs()[ v.SourceID() ].erase( CfgID::MinValue );
    Visconfigs()[ v.SourceID() ].erase( CfgID::MaxValue );
  }
  else
  {
    Visconfigs()[ v.SourceID() ].Put( CfgID::MinValue, rangeMin, MessageDefined );
    Visconfigs()[ v.SourceID() ].Put( CfgID::MaxValue, rangeMax, MessageDefined );
  }

  LabelList groupLabels,
            channelLabels;
  int channelGroupSize = 1;
  if( !s.ChannelLabels().IsTrivial() )
  {
    for( int i = 0; i < s.ChannelLabels().Size(); ++i )
    {
      istringstream iss( s.ChannelLabels()[ i ] );
      HierarchicalLabel label;
      iss >> label;
      if( label.size() == 2 )
      {
        if( groupLabels.empty() )
        {
          groupLabels.push_back( Label( 0, label[ 0 ] ) );
        }
        else
        {
          if( label[ 0 ] == groupLabels.begin()->Text() )
            ++channelGroupSize;
          if( label[ 0 ] != groupLabels.rbegin()->Text() )
            groupLabels.push_back( Label( groupLabels.size(), label[ 0 ] ) );
        }
        channelLabels.push_back( Label( channelLabels.size(), label[ 1 ] ) );
      }
      else
      {
        channelLabels.push_back( Label( i, s.ChannelLabels()[ i ] ) );
      }
    }
  }
  Visconfigs()[ v.SourceID() ].Put( CfgID::ChannelGroupSize, channelGroupSize, MessageDefined );
  Visconfigs()[ v.SourceID() ].Put( CfgID::ChannelLabels, channelLabels, MessageDefined );
  Visconfigs()[ v.SourceID() ].Put( CfgID::GroupLabels, groupLabels, MessageDefined );
  if( Visuals()[ v.SourceID() ] == NULL )
    VisDisplay::HandleMessage( VisSignal( v.SourceID(), GenericSignal( v.SignalProperties() ) ) );
  Visuals()[ v.SourceID() ]->SetConfig( Visconfigs()[ v.SourceID() ] );
}

void
__fastcall
VisDisplay::VisDisplayBase::FormSizeMove( TObject* Sender )
{
  TForm* Form = static_cast<TForm*>( Sender );
  Form->Invalidate();
  Visconfigs()[ mSourceID ].Put( CfgID::Top, mpForm->Top, UserDefined );
  Visconfigs()[ mSourceID ].Put( CfgID::Left, mpForm->Left, UserDefined );
  Visconfigs()[ mSourceID ].Put( CfgID::Width, mpForm->Width, UserDefined );
  Visconfigs()[ mSourceID ].Put( CfgID::Height, mpForm->Height, UserDefined );
}

////////////////////////////////////////////////////////////////////////////////
VisDisplay::Graph::Graph( const std::string& inSourceID )
: VisDisplayBase( inSourceID ),
  mNumChannels( 0 ),
  mUserScaling( 0 ),
  mUserZoom( 0 ),
  mDisplaySamples( 0 ),
  mSignalElements( 0 )
{
  Restore();
}

VisDisplay::Graph::~Graph()
{
  Save();
}

void
VisDisplay::Graph::SetConfig( ConfigSettings& inConfig )
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
    EnlargeSignal( NULL );
  for( int i = 0; i > userScaling; --i )
    ReduceSignal( NULL );

  size_t numSamples = mDisplay.NumSamples();
  if( inConfig.Get( CfgID::NumSamples, numSamples ) )
  {
    mDisplay.SetNumSamples( numSamples );
    mDisplaySamples = numSamples;
  }
  int userZoom = mUserZoom;
  mUserZoom = 0;
  for( int i = 0; i < userZoom; ++i )
    FewerSamples( NULL );
  for( int i = 0; i > userZoom; --i )
    MoreSamples( NULL );

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
     = mDisplay.UnitsPerSample() * FilterUnitToValue( mDisplay.SampleUnit() );
    iss.clear();
    iss.str( unit );
    float unitsPerSample = 1;
    string sampleUnit = "";
    iss >> unitsPerSample >> sampleUnit;
    mDisplay.SetUnitsPerSample( unitsPerSample ).SetSampleUnit( sampleUnit );
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
VisDisplay::Graph::Restore()
{
  if( mpForm == NULL )
  {
    mpForm = new TVisGraphForm;
    BuildContextMenu();
  }
  VisDisplayBase::Restore();
  SyncDisplay();
  mDisplaySamples = mDisplay.NumSamples();
  mpForm->OnSizeMove = FormSizeMove;
  mpForm->OnKeyUp = FormKeyUp;
  mpForm->OnPaint = FormPaint;
  mpForm->Show();
}

void
VisDisplay::Graph::Save() const
{
  VisDisplayBase::Save();
}

void
VisDisplay::Graph::HandleMessage( const VisSignal& v )
{
  Graph* visual = dynamic_cast<Graph*>( Visuals()[ v.SourceID() ] );
  if( visual == NULL )
  {
    delete Visuals()[ v.SourceID() ];
    visual = new Graph( v.SourceID() );
    Visuals()[ v.SourceID() ] = visual;
  }
  visual->InstanceHandleMessage( v );
}

void
VisDisplay::Graph::InstanceHandleMessage( const VisSignal& v )
{
  if( v.Signal().Channels() < 1 || v.Signal().Elements() < 1 )
    return;

  int curChannels = mNumChannels,
      curSamples = mDisplay.NumSamples();

  // Apply the visualization filter.
  GenericSignal filteredSignal( v.Signal().Properties() );
  mDisplayFilter.Process( v.Signal(), filteredSignal );

  mNumChannels = filteredSignal.Channels();
  mDisplay.WrapForward( filteredSignal );
  mSignalElements = v.Signal().Elements();

  if( curChannels != mNumChannels || curSamples != mDisplay.NumSamples() )
    SetConfig( Visconfigs()[ mSourceID ] );
}

const char VisDisplay::Graph::cSubmenuSeparator = ':';
struct VisDisplay::Graph::MenuItemEntry VisDisplay::Graph::sMenuItems[] =
{
  { EnlargeSignal, EnlargeSignal_Enabled, NULL, "Enlarge Signal" },
  { ReduceSignal, ReduceSignal_Enabled, NULL, "Reduce Signal" },
  { NULL, NULL, NULL, "-" },
  { FewerSamples, FewerSamples_Enabled, NULL, "Fewer Samples" },
  { MoreSamples, MoreSamples_Enabled, NULL, "More Samples" },
  { NULL, NULL, NULL, "-" },
  { FewerChannels, FewerChannels_Enabled, NULL, "Fewer Channels" },
  { MoreChannels, MoreChannels_Enabled, NULL, "More Channels" },
  { NULL, NULL, NULL, "-" },
  { ToggleDisplayMode, NULL, NULL, "Toggle Display Mode" },
  { ToggleColor, ToggleColor_Enabled, ToggleColor_Checked, "Color Display" },
  { InvertDisplay, NULL, InvertDisplay_Checked, "Invert" },
  { ChooseColors, ChooseColors_Enabled, NULL, "Choose Channel Colors..." },
  { NULL, NULL, NULL, "-" },
  { ToggleBaselines, ToggleBaselines_Enabled, ToggleBaselines_Checked, "Show Baselines" },
  { ToggleValueUnit, ToggleValueUnit_Enabled, ToggleValueUnit_Checked, "Show Unit" },
  { ToggleChannelLabels, ToggleChannelLabels_Enabled, ToggleChannelLabels_Checked, "Show Legend" },
  { NULL, NULL, NULL, "-" },
  { NULL, Filter_Enabled, NULL, "High Pass:" },
  { SetHP, Filter_Enabled, SetHP_Checked, "High Pass:off" },
  { SetHP, Filter_Enabled, SetHP_Checked, "High Pass:0.1Hz" },
  { SetHP, Filter_Enabled, SetHP_Checked, "High Pass:1Hz" },
  { SetHP, Filter_Enabled, SetHP_Checked, "High Pass:5Hz" },
  { NULL, Filter_Enabled, NULL, "Low Pass:" },
  { SetLP, Filter_Enabled, SetLP_Checked, "Low Pass:off" },
  { SetLP, Filter_Enabled, SetLP_Checked, "Low Pass:30Hz" },
  { SetLP, Filter_Enabled, SetLP_Checked, "Low Pass:40Hz" },
  { SetLP, Filter_Enabled, SetLP_Checked, "Low Pass:70Hz" },
  { NULL, Filter_Enabled, NULL, "Notch:" },
  { SetNotch, Filter_Enabled, SetNotch_Checked, "Notch:off" },
  { SetNotch, Filter_Enabled, SetNotch_Checked, "Notch:50Hz" },
  { SetNotch, Filter_Enabled, SetNotch_Checked, "Notch:60Hz" },
};

void
VisDisplay::Graph::BuildContextMenu()
{
  assert( mpForm != NULL );
  mMenuItems.clear();
  TPopupMenu* menu = new TPopupMenu( mpForm );
  for( size_t i = 0; i < sizeof( sMenuItems ) / sizeof( *sMenuItems ); ++i )
  {
    TMenuItem* newItem = new TMenuItem( menu );
    mMenuItems.push_back( newItem );
    newItem->Tag = i;

    string caption = sMenuItems[ i ].mCaption;
    size_t pos = caption.find( cSubmenuSeparator );
    if( pos != string::npos )
    {
      string topLevelCaption = caption.substr( 0, pos );
      caption = caption.substr( pos + 1 );
      if( caption == "" )
      {
        newItem->Caption = topLevelCaption.c_str();
        menu->Items->Add( newItem );
      }
      else
      {
        TMenuItem* topLevelItem = menu->Items->Find( topLevelCaption.c_str() );
        if( topLevelItem != NULL )
        {
          newItem->Caption = caption.c_str();
          newItem->OnClick = PopupMenuItemClick;
          newItem->RadioItem = true;
          topLevelItem->Add( newItem );
        }
      }
    }
    else
    {
      newItem->Caption = caption.c_str();
      newItem->OnClick = PopupMenuItemClick;
      menu->Items->Add( newItem );
    }
  }
  menu->OnPopup = PopupMenuPopup;
  mpForm->PopupMenu = menu;
}

void
__fastcall
VisDisplay::Graph::PopupMenuPopup( TObject* inSender )
{
  TPopupMenu* menu = dynamic_cast<TPopupMenu*>( inSender );
  assert( menu != NULL );
  for( size_t i = 0; i < mMenuItems.size() && i < sizeof( sMenuItems ) / sizeof( *sMenuItems ); ++i )
  {
    TMenuItem* item = mMenuItems[ i ];
    if( sMenuItems[ i ].mGetChecked )
      item->Checked = ( this->*sMenuItems[ i ].mGetChecked )( i );
    if( sMenuItems[ i ].mGetEnabled )
      item->Enabled = ( this->*sMenuItems[ i ].mGetEnabled )( i );
  }
}

void
__fastcall
VisDisplay::Graph::PopupMenuItemClick( TObject* inSender )
{
  TMenuItem* item = dynamic_cast<TMenuItem*>( inSender );
  assert( item != NULL );
  MenuAction action = sMenuItems[ item->Tag ].mAction;
  assert( action != NULL );
  ( this->*action )( item->Tag );
}

void
VisDisplay::Graph::ToggleDisplayMode( size_t )
{
  mDisplay.SetDisplayMode(
    ( mDisplay.DisplayMode() + 1 ) % SignalDisplay::numDisplayModes
  );
}

void
VisDisplay::Graph::ToggleBaselines( size_t )
{
  mDisplay.SetBaselinesVisible( !mDisplay.BaselinesVisible() );
  Visconfigs()[ mSourceID ].Put( CfgID::ShowBaselines, mDisplay.BaselinesVisible(), UserDefined );
}

bool
VisDisplay::Graph::ToggleBaselines_Enabled( size_t ) const
{
  return mDisplay.DisplayMode() == SignalDisplay::polyline;
}

bool
VisDisplay::Graph::ToggleBaselines_Checked( size_t ) const
{
  return mDisplay.BaselinesVisible();
}

void
VisDisplay::Graph::ToggleValueUnit( size_t )
{
  mDisplay.SetValueUnitVisible( !mDisplay.ValueUnitVisible() );
  Visconfigs()[ mSourceID ].Put( CfgID::ShowValueUnit, mDisplay.ValueUnitVisible(), UserDefined );
}

bool
VisDisplay::Graph::ToggleValueUnit_Enabled( size_t ) const
{
  return mDisplay.DisplayMode() == SignalDisplay::polyline;
}

bool
VisDisplay::Graph::ToggleValueUnit_Checked( size_t ) const
{
  return mDisplay.ValueUnitVisible();
}

void
VisDisplay::Graph::ToggleChannelLabels( size_t )
{
  mDisplay.SetChannelLabelsVisible( !mDisplay.ChannelLabelsVisible() );
}

bool
VisDisplay::Graph::ToggleChannelLabels_Enabled( size_t ) const
{
  return !mDisplay.ChannelLabels().empty();
}

bool
VisDisplay::Graph::ToggleChannelLabels_Checked( size_t ) const
{
  return mDisplay.ChannelLabelsVisible();
}

void
VisDisplay::Graph::ToggleColor( size_t )
{
  mDisplay.SetColorDisplay( !mDisplay.ColorDisplay() );
}

bool
VisDisplay::Graph::ToggleColor_Enabled( size_t ) const
{
  return mDisplay.DisplayMode() == SignalDisplay::polyline
         || mDisplay.DisplayMode() == SignalDisplay::field2d;
}

bool
VisDisplay::Graph::ToggleColor_Checked( size_t ) const
{
  return mDisplay.ColorDisplay();
}

void
VisDisplay::Graph::InvertDisplay( size_t )
{
  mDisplay.SetInverted( !mDisplay.Inverted() );
  Visconfigs()[ mSourceID ].Put( CfgID::InvertedDisplay, mDisplay.Inverted(), UserDefined );
}

bool
VisDisplay::Graph::InvertDisplay_Checked( size_t ) const
{
  return mDisplay.Inverted();
}

void
VisDisplay::Graph::ChooseColors( size_t )
{
  // The dialog's "custom colors" are used to hold the user colors.
  // Maybe this should be changed in the future.
  const numCustomColors = 16;
  COLORREF customColors[ numCustomColors ];
  ColorList channelColors = mDisplay.ChannelColors();
  for( int i = 0; i < ::min<int>( channelColors.size(), numCustomColors ); ++i )
    customColors[ i ] = channelColors[ i ].ToWinColor();
  for( int i = channelColors.size(); i < numCustomColors; ++i )
    customColors[ i ] = RGBColor( RGBColor::Black ).ToWinColor();
  CHOOSECOLOR chooserParams =
  {
    sizeof( CHOOSECOLOR ),
    mpForm->Handle,
    NULL,
    0x0,
    customColors,
    CC_FULLOPEN,
    NULL,
    NULL,
    NULL
  };
  if( ::ChooseColor( &chooserParams ) )
  {
    int numUserColors = 0;
    while( numUserColors < numCustomColors && customColors[ numUserColors ] != RGBColor::Black )
      ++numUserColors;
    if( numUserColors == 0 )
      channelColors.resize( 1, RGBColor::White );
    else
    {
      channelColors.resize( numUserColors );
      for( int i = 0; i < numUserColors; ++i )
        channelColors[ i ] = RGBColor::FromWinColor( customColors[ i ] );
    }
    mDisplay.SetChannelColors( channelColors );
    Visconfigs()[ mSourceID ].Put( CfgID::ChannelColors, channelColors, UserDefined );
  }
}

bool
VisDisplay::Graph::ChooseColors_Enabled( size_t ) const
{
  return mDisplay.ColorDisplay()
         && mDisplay.DisplayMode() == SignalDisplay::polyline;
}

void
VisDisplay::Graph::EnlargeSignal( size_t )
{
  mDisplay.SetMinValue( mDisplay.MinValue() / 2 );
  mDisplay.SetMaxValue( mDisplay.MaxValue() / 2 );
  ++mUserScaling;
}

bool
VisDisplay::Graph::EnlargeSignal_Enabled( size_t ) const
{
  return mUserScaling < maxUserScaling;
}

void
VisDisplay::Graph::ReduceSignal( size_t )
{
  mDisplay.SetMinValue( mDisplay.MinValue() * 2 );
  mDisplay.SetMaxValue( mDisplay.MaxValue() * 2 );
  --mUserScaling;
}

bool
VisDisplay::Graph::ReduceSignal_Enabled( size_t ) const
{
  return mUserScaling > -maxUserScaling;
}

void
VisDisplay::Graph::FewerSamples( size_t )
{
  mDisplay.SetNumSamples( mDisplaySamples /= 2 );
  ++mUserZoom;
}

bool
VisDisplay::Graph::FewerSamples_Enabled( size_t ) const
{
  return ( 2 * mSignalElements <= mDisplaySamples ) && ( mUserZoom < maxUserScaling );
}

void
VisDisplay::Graph::MoreSamples( size_t )
{
  mDisplay.SetNumSamples( mDisplaySamples *= 2 );
  --mUserZoom;
}

bool
VisDisplay::Graph::MoreSamples_Enabled( size_t ) const
{
  return ( 2 * mSignalElements <= mDisplaySamples ) && ( mUserZoom > -maxUserScaling );
}

void
VisDisplay::Graph::FewerChannels( size_t )
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
VisDisplay::Graph::FewerChannels_Enabled( size_t ) const
{
  return mDisplay.DisplayGroups() > 1;
}

void
VisDisplay::Graph::MoreChannels( size_t )
{
  int maxDisplayGroups = mNumChannels / mDisplay.ChannelGroupSize(),
      newDisplayGroups = min( maxDisplayGroups, mDisplay.DisplayGroups() * 2 );
  mDisplay.SetDisplayGroups( newDisplayGroups );
}

bool
VisDisplay::Graph::MoreChannels_Enabled( size_t ) const
{
  return mDisplay.DisplayGroups() < mNumChannels / mDisplay.ChannelGroupSize();
}

double
VisDisplay::Graph::FilterItemToValue( size_t inMenuItem ) const
{
  const char* p = sMenuItems[ inMenuItem ].mCaption;
  while( *p && !::isdigit( *p ) )
    ++p;
  return ::atof( p ) * mDisplay.UnitsPerSample()
                     * FilterUnitToValue( mDisplay.SampleUnit() );
}

double
VisDisplay::Graph::FilterUnitToValue( const string& inUnit ) const
{
  if( inUnit.length() < 1 || *inUnit.rbegin() != 's' )
    return 0;

  return PhysicalUnit().SetOffset( 0 ).SetGain( 1 ).SetSymbol( "s" )
                       .PhysicalToRaw( string( "1" ) + inUnit );
}

bool
VisDisplay::Graph::Filter_Enabled( size_t ) const
{
  return mDisplay.UnitsPerSample() * FilterUnitToValue( mDisplay.SampleUnit() ) != 0;
}

void
VisDisplay::Graph::SetHP( size_t inMenuItem )
{
  mDisplayFilter.HPCorner( FilterItemToValue( inMenuItem ) );
}

bool
VisDisplay::Graph::SetHP_Checked( size_t inMenuItem ) const
{
  return ::fabs( mDisplayFilter.HPCorner() - FilterItemToValue( inMenuItem ) ) < 1e-6;
}

void
VisDisplay::Graph::SetLP( size_t inMenuItem )
{
  mDisplayFilter.LPCorner( FilterItemToValue( inMenuItem ) );
}

bool
VisDisplay::Graph::SetLP_Checked( size_t inMenuItem ) const
{
  return mDisplayFilter.LPCorner() == FilterItemToValue( inMenuItem );
}

void
VisDisplay::Graph::SetNotch( size_t inMenuItem )
{
  mDisplayFilter.NotchCenter( FilterItemToValue( inMenuItem ) );
}

bool
VisDisplay::Graph::SetNotch_Checked( size_t inMenuItem ) const
{
  return mDisplayFilter.NotchCenter() == FilterItemToValue( inMenuItem );
}

void
VisDisplay::Graph::SyncDisplay()
{
  TRect rect = mpForm->ClientRect;
  GUI::DrawContext dc =
  {
    ::GetDC( mpForm->Handle ),
    {
      rect.left,
      rect.top,
      rect.right,
      rect.bottom
    },
  };
  mDisplay.SetContext( dc );
}

void
__fastcall
VisDisplay::Graph::FormPaint( TObject* )
{
  mDisplay.Paint( mpForm->mUpdateRgn );
}

void
__fastcall
VisDisplay::Graph::FormSizeMove( TObject* Sender )
{
  SyncDisplay();
  VisDisplay::VisDisplayBase::FormSizeMove( Sender );
}

void
__fastcall
VisDisplay::Graph::FormKeyUp( TObject*, WORD& key, TShiftState )
{
  switch( key )
  {
    case VK_UP:
      mDisplay.SetTopGroup( mDisplay.TopGroup() - 1 );
      break;
    case VK_DOWN:
      mDisplay.SetTopGroup( mDisplay.TopGroup() + 1 );
      break;
    case VK_PRIOR:
      mDisplay.SetTopGroup( mDisplay.TopGroup() - mDisplay.DisplayGroups() );
      break;
    case VK_NEXT:
      mDisplay.SetTopGroup( mDisplay.TopGroup() + mDisplay.DisplayGroups() );
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////
VisDisplay::Memo::Memo( const std::string& inSourceID )
: VisDisplayBase( inSourceID ),
  mpMemo( new TMemo( ( TComponent* )NULL ) ),
  mNumLines( 0 )
{
  Restore();
}

VisDisplay::Memo::~Memo()
{
  Save();
  delete mpMemo;
}

void
VisDisplay::Memo::SetConfig( ConfigSettings& inConfig )
{
  VisDisplayBase::SetConfig( inConfig );
  inConfig.Get( CfgID::NumLines, mNumLines );
  if( mNumLines < 1 )
    mNumLines = numeric_limits<int>::max();
}

void
VisDisplay::Memo::Restore()
{
  if( mpForm == NULL )
    mpForm = new TVisForm();
  VisDisplayBase::Restore();
  mpForm->Show();
  mpMemo->Visible = false;
  mpMemo->Parent = mpForm;
  mpMemo->BoundsRect = mpForm->ClientRect;
  mpMemo->Anchors << akLeft << akTop << akRight << akBottom;
  mpMemo->ScrollBars = ssVertical;
  mpMemo->ReadOnly = true;
  mpMemo->Visible = true;
}

void
VisDisplay::Memo::Save() const
{
  VisDisplayBase::Save();
  Visconfigs()[ mSourceID ].Put( CfgID::NumLines, mNumLines, MessageDefined );
}

void
VisDisplay::Memo::HandleMessage( const VisMemo& v )
{
  Memo* visual = dynamic_cast<Memo*>( Visuals()[ v.SourceID() ] );
  if( visual == NULL )
  {
    delete Visuals()[ v.SourceID() ];
    visual = new Memo( v.SourceID() );
    Visuals()[ v.SourceID() ] = visual;
  }
  visual->InstanceHandleMessage( v );
}

void
VisDisplay::Memo::InstanceHandleMessage( const VisMemo& v )
{
  while( mpMemo->Lines->Count >= mNumLines )
    mpMemo->Lines->Delete( 0 );
  string s = v.MemoText();
  size_t pos = 0;
  while( ( pos = s.find_first_of( "\n\r" ) ) != s.npos )
  {
    mpMemo->Lines->Add( s.substr( 0, pos ).c_str() );
    s.erase( 0, pos + 1 );
  }
  if( !s.empty() )
    mpMemo->Lines->Add( s.c_str() );
}

////////////////////////////////////////////////////////////////////////////////
VisDisplay::Bitmap::Bitmap( const std::string& inSourceID )
: VisDisplayBase( inSourceID )
{
  Restore();
}

VisDisplay::Bitmap::~Bitmap()
{
  Save();
}

void
VisDisplay::Bitmap::SetConfig( ConfigSettings& inConfig )
{
  VisDisplayBase::SetConfig( inConfig );
}

void
VisDisplay::Bitmap::Restore()
{
  if( mpForm == NULL )
  {
    mpForm = new TVisBitmapForm();
    VisDisplayBase::Restore();
    mpForm->OnSizeMove = FormSizeMove;
    mpForm->OnPaint = FormPaint;
  }
  mpForm->Show();
}

void
VisDisplay::Bitmap::Save() const
{
  VisDisplayBase::Save();
}

void
VisDisplay::Bitmap::HandleMessage( const VisBitmap& b )
{
  Bitmap* visual = dynamic_cast<Bitmap*>( Visuals()[ b.SourceID() ] );
  if( visual == NULL )
  {
    delete Visuals()[ b.SourceID() ];
    visual = new Bitmap( b.SourceID() );
    Visuals()[ b.SourceID() ] = visual;
  }
  visual->InstanceHandleMessage( b );
}

void
VisDisplay::Bitmap::InstanceHandleMessage( const VisBitmap& b )
{
  const BitmapImage& image = b.BitmapImage();
  if( image.Empty() )
  { // An empty image precedes a reference frame (rather than a difference frame).
    mImageBuffer.SetBlack();
  }
  else
  {
    if( ( mImageBuffer.Width() != image.Width() ) || ( mImageBuffer.Height() != image.Height() ) )
    {
      mImageBuffer = image;
      // Adapt the window's aspect ratio without changing its width.
      if( image.Width() > 0 )
      {
        mpForm->ClientHeight = ( image.Height() * mpForm->ClientWidth ) / image.Width();
        Visconfigs()[ mSourceID ].Put( CfgID::Height, mpForm->Height, UserDefined );
      }
    }
    else
    {
      mImageBuffer += image;
    }
    mpForm->Invalidate();
  }
}

void
__fastcall
VisDisplay::Bitmap::FormPaint( TObject* )
{
  int formHeight = mpForm->ClientHeight,
      formWidth = mpForm->ClientWidth;
  HDC formDC = mpForm->Canvas->Handle,
      tmpDC = ::CreateCompatibleDC( formDC );
  HBITMAP offscreenBmp = ::CreateCompatibleBitmap( formDC, formWidth, formHeight );
  ::DeleteObject( ::SelectObject( tmpDC, offscreenBmp ) );
  ::SelectClipRgn( tmpDC, mpForm->mUpdateRgn );
  ::SelectClipRgn( formDC, mpForm->mUpdateRgn );

  if( mImageBuffer.Empty() )
  {
    RECT formRect = { 0, 0, formWidth, formHeight };
    ::FillRect( tmpDC, &formRect, ::GetStockObject( BLACK_BRUSH ) );
  }
  else
  {
    for( int x = 0; x < mImageBuffer.Width(); ++x )
      for( int y = 0; y < mImageBuffer.Height(); ++y )
      {
        RECT pixelRect =
        {
          ( x * formWidth ) / mImageBuffer.Width(),
          ( y * formHeight ) / mImageBuffer.Height(),
          ( ( x + 1 ) * formWidth ) / mImageBuffer.Width(),
          ( ( y + 1 ) * formHeight ) / mImageBuffer.Height()
        };
        COLORREF pixelColor = RGBColor( mImageBuffer( x, y ) ).ToWinColor();
        ::SetBkColor( tmpDC, pixelColor );
        ::ExtTextOut( tmpDC, pixelRect.left, pixelRect.top, ETO_OPAQUE, &pixelRect, " ", 0, NULL );
      }
    ::Sleep( 0 );
  }
  ::BitBlt( formDC, 0, 0, formWidth, formHeight, tmpDC, 0, 0, SRCCOPY );
  ::DeleteObject( tmpDC );
  ::DeleteObject( offscreenBmp );
}

