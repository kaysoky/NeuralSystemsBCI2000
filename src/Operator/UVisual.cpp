////////////////////////////////////////////////////////////////////////////////
//
// File:    UVisual.cpp
//
// Authors: Gerwin Schalk, Juergen Mellinger
//
// Changes: Apr 15, 2003, juergen.mellinger@uni-tuebingen.de:
//          Reworked graph display double buffering scheme.
//          Untangled window painting from content changes.
//          Introduced clipping to reduce the amount of time spent blitting
//          graphics data.
//
//          May 27, 2003, jm:
//          Created Operator/UVisual to maintain VISUAL and VISCFGLIST
//          as part of the operator module.
//
//          June 1, 2003, jm:
//          Rewrote VISUAL as a class hierarchy.
//
//          June 10, 2003, jm:
//          Added the polyline3d/colorfield display types for a graph to support
//          FFT data.
//
//          Aug 8, 2003, jm:
//          Cleared up use of registry.
//
//          Nov 20, 2003, jm:
//          Added context menu to ease interactive configuration of graph
//          display properties.
//          Introduced colorized y axis ticks.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UVisual.h"
#include "defines.h"
#include "UGenericVisualization.h"

#include <assert>
#include <Registry.hpp>
#include <math.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <vcl.h>
#include <grids.hpp>

using namespace std;

static const char* key_base = KEY_BCI2000 KEY_OPERATOR KEY_VISUALIZATION "\\";

void
VISUAL::HandleMessage( const VisCfg& v )
{
  VisualBase::HandleMessage( v );
}

void
VISUAL::HandleMessage( const VisSignal& v )
{
  Graph::HandleMessage( v );
}

void
VISUAL::HandleMessage( const VisMemo& v )
{
  Memo::HandleMessage( v );
}

////////////////////////////////////////////////////////////////////////////////

VISUAL::VisualBase::vis_container&
VISUAL::VisualBase::Visuals()
{
  static VISUAL::VisualBase::vis_container visuals;
  return visuals;
}

VISUAL::VisualBase::config_container&
VISUAL::VisualBase::Visconfigs()
{
  static VISUAL::VisualBase::config_container visconfigs;
  return visconfigs;
}

////////////////////////////////////////////////////////////////////////////////
static const char* cfgid_prefix = "CFGID"; // const AnsiString cfgid_prefix = "CFGID"; won't work.

void
VISUAL::VisualBase::config_container::Save()
{
  AnsiString as_cfgid_prefix = cfgid_prefix;

  for( iterator i = begin(); i != end(); ++i )
  {
    AnsiString key = key_base + AnsiString( i->first );
    TRegistry* reg = new TRegistry( KEY_WRITE );
    if( reg->OpenKey( key, true ) )
    {
      TStringList* valueNames = new TStringList;
      reg->GetValueNames( valueNames );
      for( int j = 0; j < valueNames->Count; ++j )
        reg->DeleteValue( valueNames->Strings[ j ] );

      reg->WriteString( "Title", i->second[ CFGID::WINDOWTITLE ].c_str() );
      for( config_settings::iterator j = i->second.begin(); j != i->second.end(); ++j )
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
VISUAL::VisualBase::config_container::Restore()
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
       id_type visID = ::atoi( keys->Strings[ i ].c_str() );
       if( reg->OpenKeyReadOnly( key ) && visID != 0 )
       {
         TStringList* valueNames = new TStringList;
         reg->GetValueNames( valueNames );
         for( int j = 0; j < valueNames->Count; ++j )
         {

           if( valueNames->Strings[ j ].SubString( 0, as_cfgid_prefix.Length() ) == as_cfgid_prefix )
           {
             id_type cfgID = ::atoi( valueNames->Strings[ j ].c_str() + as_cfgid_prefix.Length() );
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
VISUAL::VisualBase::VisualBase( id_type inSourceID )
: sourceID( inSourceID ),
  form( NULL )
{
  VisualBase* visual = Visuals()[ sourceID ];
  delete visual;
  Visuals()[ sourceID ] = this;
}

VISUAL::VisualBase::~VisualBase()
{
  delete form;
}

void
VISUAL::VisualBase::vis_container::clear()
{
  for( iterator i = begin(); i != end(); ++i )
    delete i->second;
  vis_container_base::clear();
}

void
VISUAL::VisualBase::SetConfig( config_settings& inConfig )
{
  title = inConfig[ CFGID::WINDOWTITLE ];
  if( title != "" )
    form->Caption = AnsiString( title.c_str() ) + " (" + AnsiString( sourceID ) + ")";
  else
    form->Caption = AnsiString( sourceID );

  // The static variables make each new window appear a little down right
  // to the previous one.
  static int newTop = 10,
             newLeft = 10;
  int formTop = 10,
      formLeft = 10,
      formHeight = 100,
      formWidth = 100;
  bool gotPosition = inConfig.Get( CFGID::Top, formTop )
                  && inConfig.Get( CFGID::Left, formLeft )
                  && inConfig.Get( CFGID::Height, formHeight )
                  && inConfig.Get( CFGID::Width, formWidth );
  if( !gotPosition )
  {
      formTop = newTop;
      newTop += 10;
      formLeft = newLeft;
      newLeft += 10;
  }
  form->Top = formTop;
  form->Left = formLeft;
  form->Height = formHeight;
  form->Width = formWidth;
  if( !PtInRect( form->ClientRect, TPoint( 10, 10 ) ) )
  {
    form->Height = 100;
    form->Width = 100;
  }
  if( !PtInRect( Screen->DesktopRect, form->ClientOrigin ) )
  {
    form->Top = newTop;
    newTop += 10;
    form->Left = newLeft;
    newLeft += 10;
  }
}

void
VISUAL::VisualBase::Restore()
{
  assert( form != NULL );
  form->BorderStyle = bsSizeToolWin;
  form->OnMove = FormMove;
  form->OnResize = FormResize;
  SetConfig( Visconfigs()[ sourceID ] );
}

void
VISUAL::VisualBase::Save() const
{
  assert( form != NULL );
}

void
VISUAL::VisualBase::HandleMessage( const VisCfg& v )
{
  Visconfigs()[ v.GetSourceID() ].Put( v.GetCfgID(), v.GetCfgValue(), MessageDefined );
  if( Visuals()[ v.GetSourceID() ] != NULL )
    Visuals()[ v.GetSourceID() ]->SetConfig( Visconfigs()[ v.GetSourceID() ] );
}

void
__fastcall
VISUAL::VisualBase::FormMove( TObject* )
{
  Visconfigs()[ sourceID ].Put( CFGID::Top, form->Top, UserDefined );
  Visconfigs()[ sourceID ].Put( CFGID::Left, form->Left, UserDefined );
}

void
__fastcall
VISUAL::VisualBase::FormResize( TObject* Sender )
{
  TForm* Form = static_cast<TForm*>( Sender );
  Form->Invalidate();
  Visconfigs()[ sourceID ].Put( CFGID::Width, form->Width, UserDefined );
  Visconfigs()[ sourceID ].Put( CFGID::Height, form->Height, UserDefined );
}

////////////////////////////////////////////////////////////////////////////////
const RGBColor VISUAL::Graph::cChannelColorsDefault[] =
{
  White,
  White,
  White,
  White,
  Yellow,
  Colorlist::End
};

VISUAL::Graph::Graph( id_type inSourceID )
: VisualBase( inSourceID ),
  mShowCursor( false ),
  mWrapAround( false ),
  mNumSamples( cNumSamplesDefault ),
  mSampleCursor( 0 ),
  mNumDisplayGroups( 0 ),
  mNumDisplayChannels( 0 ),
  mBottomGroup( 0 ),
  mShowBaselines( false ),
  mShowChannelLabels( false ),
  mShowValueUnit( false ),
  mDisplayColors( true ),
  mChannelGroupSize( 1 ),
  mMinValue( cMinValueDefault ),
  mMaxValue( cMaxValueDefault ),
  mUnitsPerSample( 1 ),
  mUnitsPerChannel( 1 ),
  mUnitsPerValue( 1 ),
  mDisplayMode( polyline ),
  mRedrawRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ),
  mpOffscreenBitmap( new Graphics::TBitmap ),
  mChannelColors( cChannelColorsDefault ),
  mUserScaling( 0 )
{
  Restore();
}

VISUAL::Graph::~Graph()
{
  Save();
  delete mpOffscreenBitmap;
  ::DeleteObject( mRedrawRgn );
}

void
VISUAL::Graph::SetConfig( config_settings& inConfig )
{
  VisualBase::SetConfig( inConfig );

  int userScaling = mUserScaling;
  mUserScaling = 0;
  inConfig.Get( CFGID::MINVALUE, mMinValue );
  inConfig.Get( CFGID::MAXVALUE, mMaxValue );
  for( int i = 0; i < userScaling; ++i )
    EnlargeSignal();
  for( int i = 0; i > userScaling; --i )
    ReduceSignal();

  size_t newNumSamples = mNumSamples;
  inConfig.Get( CFGID::NUMSAMPLES, newNumSamples );
  SetNumSamples( newNumSamples );
  inConfig.Get( CFGID::channelGroupSize, mChannelGroupSize );
  if( mChannelGroupSize < 1 )
    mChannelGroupSize = numeric_limits<size_t>::max();
  int graphType = CFGID::polyline;
  inConfig.Get( CFGID::graphType, graphType );
  switch( graphType )
  {
    case CFGID::polyline:
      mDisplayMode = polyline;
      break;
    case CFGID::field2d:
      mDisplayMode = field2d;
      break;
  }
  inConfig.Get( CFGID::showBaselines, mShowBaselines );
  inConfig.Get( CFGID::channelColors, mChannelColors );

  string unit;
  istringstream iss;
  if( inConfig.Get( CFGID::sampleUnit, unit ) )
  {
    iss.clear();
    iss.str( unit );
    mUnitsPerSample = 1;
    mSampleUnit = "";
    iss >> mUnitsPerSample >> mSampleUnit;
  }

  if( inConfig.Get( CFGID::channelUnit, unit ) )
  {
    iss.clear();
    iss.str( unit );
    mUnitsPerChannel = 1;
    mChannelUnit = "";
    iss >> mUnitsPerChannel >> mChannelUnit;
  }

  if( inConfig.Get( CFGID::valueUnit, unit ) )
  {
    iss.clear();
    iss.str( unit );
    mUnitsPerValue = 1;
    mValueUnit = "";
    iss >> mUnitsPerValue >> mValueUnit;
  }
  inConfig.Get( CFGID::showValueUnit, mShowValueUnit );
  
  inConfig.Get( CFGID::xAxisMarkers, mXAxisMarkers );
  inConfig.Get( CFGID::channelLabels, mChannelLabels );
  mShowChannelLabels = !mChannelLabels.empty();

  // Sanity checks.
  if( mMinValue == mMaxValue )
    mMaxValue = mMinValue + 1;
  form->Invalidate();
}

void
VISUAL::Graph::Restore()
{
  if( form == NULL )
  {
    form = new TVisGraphForm;
    BuildContextMenu();
  }
  VisualBase::Restore();
  form->OnKeyUp = FormKeyUp;
  form->OnPaint = FormPaint;
  form->Show();
}

void
VISUAL::Graph::Save() const
{
  VisualBase::Save();
}

void
VISUAL::Graph::HandleMessage( const VisSignal& v )
{
  Graph* visual = dynamic_cast<Graph*>( Visuals()[ v.GetSourceID() ] );
  if( visual == NULL )
  {
    delete Visuals()[ v.GetSourceID() ];
    visual = new Graph( v.GetSourceID() );
    Visuals()[ v.GetSourceID() ] = visual;
  }
  visual->InstanceHandleMessage( v );
}

void
VISUAL::Graph::InstanceHandleMessage( const VisSignal& v )
{
  const GenericSignal& newData = v.GetSignal();
  if( newData.Channels() < 1 || newData.Elements() < 1 )
    return;

  // Any changes in the signal size that we must react to?
  bool reconfigure = false;
  if( newData.Elements() > mNumSamples )
  {
    SetNumSamples( newData.Elements() );
    reconfigure = true;
  }
  if( newData.Channels() != mData.Channels() )
  {
    mNumDisplayGroups = ( newData.Channels() - 1 ) / mChannelGroupSize + 1;
    switch( mDisplayMode )
    {
      case polyline:
        mNumDisplayGroups = min( mNumDisplayGroups, cMaxDisplayGroups );
        break;
      case field2d:
      default:
        break;
    }
    mNumDisplayChannels = min( newData.Channels(), mNumDisplayGroups * mChannelGroupSize );
    reconfigure = true;
  }
  if( reconfigure )
  {
    mData = GenericSignal( newData.Channels(), mNumSamples );
    mSampleCursor = 0;
    form->Invalidate();
  }

  mShowCursor = ( newData.Elements() < mNumSamples );

  for( size_t i = 0; i < newData.Channels(); ++i )
    for( size_t j = 0; j < newData.Elements(); ++j )
      mData( i, ( mSampleCursor + j ) % mData.Elements() ) = newData( i, j );

  SyncGraphics();

  int firstInvalidSample = mSampleCursor,
      firstValidSample = mSampleCursor + newData.Elements();
  mSampleCursor = firstValidSample % mNumSamples;
  mWrapAround |= bool( firstValidSample / mNumSamples > 0  );

  long firstInvalidPixel = mDataRect.left,
       firstValidPixel = mDataRect.right;

  switch( mDisplayMode )
  {
    case polyline:
      firstInvalidPixel = SampleLeft( firstInvalidSample - 1 );
      firstValidPixel = SampleLeft( firstValidSample + 1 );
      break;
    case field2d:
      firstInvalidPixel = SampleLeft( firstInvalidSample );
      firstValidPixel = SampleLeft( firstValidSample );
      break;
    default:
      assert( false );
  }

  // We maintain a redraw region to make sure the
  // cursor is deleted from its old position.
  ::InvalidateRgn( form->Handle, mRedrawRgn, false );
  ::SetRectRgn( mRedrawRgn, 0, 0, 0, 0 );

  RECT invalidRect = mDataRect;

  // The non-wrapped area.
  invalidRect.left = max( firstInvalidPixel, mDataRect.left );
  invalidRect.right = min( firstValidPixel, mDataRect.right );
  if( invalidRect.left < invalidRect.right )
    ::InvalidateRect( form->Handle, &invalidRect, false );

  // The area wrapped around the left edge.
  invalidRect.left = max( firstInvalidPixel + mDataWidth, mDataRect.left );
  invalidRect.right = min( firstValidPixel + mDataWidth, mDataRect.right );
  if( invalidRect.left < invalidRect.right )
    ::InvalidateRect( form->Handle, &invalidRect, false );

  // The area wrapped around the right edge.
  invalidRect.left = max( firstInvalidPixel - mDataWidth, mDataRect.left );
  invalidRect.right = min( firstValidPixel - mDataWidth, mDataRect.right );
  if( invalidRect.left < invalidRect.right )
    ::InvalidateRect( form->Handle, &invalidRect, false );
}

inline
void
VISUAL::Graph::SyncGraphics()
{
  mDataRect = TRect( cLabelWidth, 0, form->ClientWidth, form->ClientHeight );
  mDataWidth = std::max<int>( 0, mDataRect.right - mDataRect.left );
  mDataHeight = std::max<int>( 0, mDataRect.bottom - mDataRect.top - cLabelWidth );
}

struct VISUAL::Graph::MenuItemEntry VISUAL::Graph::sMenuItems[] =
{
  { EnlargeSignal, EnlargeSignal_Enabled, NULL, "Enlarge Signal" },
  { ReduceSignal, ReduceSignal_Enabled, NULL, "Reduce Signal" },
  { NULL, NULL, NULL, "-" },
  { LessChannels, LessChannels_Enabled, NULL, "Less Channels" },
  { MoreChannels, MoreChannels_Enabled, NULL, "More Channels" },
  { NULL, NULL, NULL, "-" },
  { ToggleDisplayMode, NULL, NULL, "Toggle Display Mode" },
  { ToggleColor, ToggleColor_Enabled, ToggleColor_Checked, "Color Display" },
  { ChooseColors, ChooseColors_Enabled, NULL, "Choose Channel Colors..." },
  { NULL, NULL, NULL, "-" },
  { ToggleBaselines, ToggleBaselines_Enabled, ToggleBaselines_Checked, "Show Baselines" },
  { ToggleValueUnit, ToggleValueUnit_Enabled, ToggleValueUnit_Checked, "Show Unit" },
  { ToggleChannelLabels, ToggleChannelLabels_Enabled, ToggleChannelLabels_Checked, "Show Legend" },
};

void
VISUAL::Graph::BuildContextMenu()
{
  assert( form != NULL );
  TPopupMenu* menu = new TPopupMenu( form );
  for( int i = 0; i < sizeof( sMenuItems ) / sizeof( *sMenuItems ); ++i )
  {
    TMenuItem* newItem = new TMenuItem( menu );
    menu->Items->Add( newItem );
    newItem->Caption = sMenuItems[ i ].mCaption;
    newItem->Tag = i;
    newItem->OnClick = PopupMenuItemClick;
  }
  menu->OnPopup = PopupMenuPopup;
  form->PopupMenu = menu;
}

void
__fastcall
VISUAL::Graph::PopupMenuPopup( TObject* inSender )
{
  TPopupMenu* menu = dynamic_cast<TPopupMenu*>( inSender );
  assert( menu != NULL );
  for( int i = 0; i < menu->Items->Count && i < sizeof( sMenuItems ) / sizeof( *sMenuItems ); ++i )
  {
    if( sMenuItems[ i ].mGetChecked )
      menu->Items->Items[ i ]->Checked = ( this->*sMenuItems[ i ].mGetChecked )();
    if( sMenuItems[ i ].mGetEnabled )
      menu->Items->Items[ i ]->Enabled = ( this->*sMenuItems[ i ].mGetEnabled )();
  }
}

void
__fastcall
VISUAL::Graph::PopupMenuItemClick( TObject* inSender )
{
  TMenuItem* item = dynamic_cast<TMenuItem*>( inSender );
  assert( item != NULL );
  MenuItemEntry::MenuAction action = sMenuItems[ item->Tag ].mAction;
  assert( action != NULL );
  ( this->*action )();
}

void
VISUAL::Graph::ToggleDisplayMode()
{
  SetDisplayMode( DisplayMode( ( mDisplayMode + 1 ) % cNumDisplayModes ) );
}

void
VISUAL::Graph::ToggleBaselines()
{
  mShowBaselines = !mShowBaselines;
  Visconfigs()[ sourceID ].Put( CFGID::showBaselines, mShowBaselines, UserDefined );
  form->Invalidate();
}

bool
VISUAL::Graph::ToggleBaselines_Enabled() const
{
  return mDisplayMode == polyline;
}

bool
VISUAL::Graph::ToggleBaselines_Checked() const
{
  return mShowBaselines;
}

void
VISUAL::Graph::ToggleValueUnit()
{
  mShowValueUnit = !mShowValueUnit;
  Visconfigs()[ sourceID ].Put( CFGID::showValueUnit, mShowValueUnit, UserDefined );
  form->Invalidate();
}

bool
VISUAL::Graph::ToggleValueUnit_Enabled() const
{
  return mDisplayMode == polyline;
}

bool
VISUAL::Graph::ToggleValueUnit_Checked() const
{
  return mShowValueUnit;
}

void
VISUAL::Graph::ToggleChannelLabels()
{
  mShowChannelLabels = !mShowChannelLabels;
  form->Invalidate();
}

bool
VISUAL::Graph::ToggleChannelLabels_Enabled() const
{
  return !mChannelLabels.empty();
}

bool
VISUAL::Graph::ToggleChannelLabels_Checked() const
{
  return mShowChannelLabels;
}

void
VISUAL::Graph::ToggleColor()
{
  mDisplayColors = !mDisplayColors;
  form->Invalidate();
}

bool
VISUAL::Graph::ToggleColor_Enabled() const
{
  return mDisplayMode == polyline || mDisplayMode == field2d;
}

bool
VISUAL::Graph::ToggleColor_Checked() const
{
  return mDisplayColors;
}

void
VISUAL::Graph::ChooseColors()
{
  // The dialog's "custom colors" are used to hold the user colors.
  // Maybe this should be changed in the future.
  const numCustomColors = 16;
  COLORREF customColors[ numCustomColors ];
  for( int i = 0; i < ::min<int>( mChannelColors.size(), numCustomColors ); ++i )
    customColors[ i ] = mChannelColors[ i ];
  for( int i = mChannelColors.size(); i < numCustomColors; ++i )
    customColors[ i ] = Black;
  CHOOSECOLOR chooserParams =
  {
    sizeof( CHOOSECOLOR ),
    form->Handle,
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
    while( numUserColors < numCustomColors && customColors[ numUserColors ] != Black )
      ++numUserColors;
    if( numUserColors == 0 )
      mChannelColors.resize( 1, White );
    else
    {
      mChannelColors.resize( numUserColors );
      for( int i = 0; i < numUserColors; ++i )
        mChannelColors[ i ] = customColors[ i ];
    }
    Visconfigs()[ sourceID ].Put( CFGID::channelColors, mChannelColors, UserDefined );
    form->Invalidate();
  }
}

bool
VISUAL::Graph::ChooseColors_Enabled() const
{
  return mDisplayColors && mDisplayMode == polyline;
}

void
VISUAL::Graph::EnlargeSignal()
{
#if 0
  float offset = ( mMinValue + mMaxValue ) / 2,
        unit = mMaxValue - offset;
  unit /= 2;
  mMinValue = offset - unit;
  mMaxValue = offset + unit;
#else
  mMinValue /= 2;
  mMaxValue /= 2;
#endif
  ++mUserScaling;
  form->Invalidate();
}

bool
VISUAL::Graph::EnlargeSignal_Enabled() const
{
  return mUserScaling < maxUserScaling;
}

void
VISUAL::Graph::ReduceSignal()
{
#if 0
  float offset = ( mMinValue + mMaxValue ) / 2,
        unit = mMaxValue - offset;
  unit *= 2;
  mMinValue = offset - unit;
  mMaxValue = offset + unit;
#else
  mMinValue *= 2;
  mMaxValue *= 2;
#endif
  --mUserScaling;
  form->Invalidate();
}

bool
VISUAL::Graph::ReduceSignal_Enabled() const
{
  return mUserScaling > -maxUserScaling;
}

void
VISUAL::Graph::LessChannels()
{
  SetDisplayGroups( mNumDisplayGroups / 2 );
}

bool
VISUAL::Graph::LessChannels_Enabled() const
{
  return mNumDisplayGroups > 1;
}

void
VISUAL::Graph::MoreChannels()
{
  SetDisplayGroups( mNumDisplayGroups * 2 );
}

bool
VISUAL::Graph::MoreChannels_Enabled() const
{
  return mNumDisplayGroups < mData.Channels() / mChannelGroupSize;
}

void
VISUAL::Graph::SetDisplayGroups( int inDisplayGroups )
{
  mNumDisplayGroups = inDisplayGroups;
  mNumDisplayChannels = min( mData.Channels(), mNumDisplayGroups * mChannelGroupSize );
  SetBottomGroup( mBottomGroup );
  form->Invalidate();
}

void
VISUAL::Graph::SetBottomGroup( int inBottomGroup )
{
  int newBottomGroup = inBottomGroup,
      maxBottomGroup = ChannelToGroup( mData.Channels() ) - int( mNumDisplayGroups );
  if( newBottomGroup > maxBottomGroup )
    newBottomGroup = maxBottomGroup;
  if( newBottomGroup < 0 )
    newBottomGroup = 0;
  if( ( size_t )newBottomGroup != mBottomGroup )
  {
    mBottomGroup = newBottomGroup;
    form->Invalidate();
  }
}

void
VISUAL::Graph::SetDisplayMode( DisplayMode mode )
{
  if( mode != mDisplayMode )
  {
    form->Invalidate();
    mDisplayMode = mode;
  }
}

void
VISUAL::Graph::SetNumSamples( int inNumSamples )
{
  size_t newNumSamples = inNumSamples;
  if( newNumSamples < 1 )
    newNumSamples = 1;
  if( newNumSamples != mNumSamples )
  {
    mData = GenericSignal( mData.Channels(), newNumSamples );
    mSampleCursor = 0;
  }
  mNumSamples = newNumSamples;
}

void
__fastcall
VISUAL::Graph::FormKeyUp( TObject*, WORD& key, TShiftState )
{
  switch( key )
  {
    case VK_UP:
      SetBottomGroup( mBottomGroup + 1 );
      break;
    case VK_DOWN:
      SetBottomGroup( mBottomGroup - 1 );
      break;
    case VK_PRIOR:
      SetBottomGroup( mBottomGroup + mNumDisplayGroups / 2 );
      break;
    case VK_NEXT:
      SetBottomGroup( mBottomGroup - mNumDisplayGroups / 2 );
      break;
  }
}

void
__fastcall
VISUAL::Graph::FormPaint( TObject* Sender )
{
  enum
  {
    backgroundBrush = 0,
    cursorBrush,
    axisBrush,
    markerBrush,
    labelFont,
    baselinePen,
    numGdiObj
  };
  struct GdiObjContainer : public vector<HGDIOBJ>
  {
    GdiObjContainer( size_t s )
    : vector<HGDIOBJ>( s, NULL ) {}
    ~GdiObjContainer()
    {
      for( iterator i = begin(); i != end(); ++i )
        ::DeleteObject( *i );
    }
  } gdi( numGdiObj ),
    signalPens( mData.Channels() ),
    signalBrushes( mData.Channels() );

  // Background properties
  const TColor backgroundColor = clBlack;
  gdi[ backgroundBrush ] = ::CreateSolidBrush( backgroundColor );

  // Cursor properties
  const TColor cursorColor = clYellow;
  const cursorWidth = 3;
  gdi[ cursorBrush ] = ::CreateSolidBrush( cursorColor );

  // Axis properties
  const TColor axisColor = clAqua;
  const axisWidth = 2;
  const tickWidth = axisWidth, tickLength = 4;
  gdi[ axisBrush ] = ::CreateSolidBrush( axisColor );
  gdi[ baselinePen ] = ::CreatePen( PS_SOLID, 0, axisColor );
  const TColor markerColor = clWhite;
  const markerWidth = 1;
  gdi[ markerBrush ] = ::CreateSolidBrush( markerColor );

  const fontHeight = cLabelWidth / 2;
  const labelColor = axisColor;
  gdi[ labelFont ] = ::CreateFont( -fontHeight, 0, 0, 0, FW_DONTCARE,
                      false, false, false, ANSI_CHARSET, OUT_RASTER_PRECIS,
                      CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                      VARIABLE_PITCH | FF_SWISS, NULL );

  // Signal properties
  if( mDisplayColors )
    for( size_t i = 0; i < mData.Channels(); ++i )
    {
      signalPens[ i ] = ::CreatePen( PS_SOLID, 0, ChannelColor( i ) );
      signalBrushes[ i ] = ::CreateSolidBrush( ChannelColor( i ) );
    }
  else
  {
    HGDIOBJ pen = ::CreatePen( PS_SOLID, 0, clWhite );
    for( size_t i = 0; i < mData.Channels(); ++i )
      signalPens[ i ] = pen;
  }

  // Do the drawing.
  TVisForm* Form = static_cast<TVisForm*>( Sender );
  TRect formRect = Form->ClientRect;
  if( mpOffscreenBitmap->Width != formRect.right )
    mpOffscreenBitmap->Width = formRect.right;
  if( mpOffscreenBitmap->Height != formRect.bottom )
    mpOffscreenBitmap->Height = formRect.bottom;

  HDC dc = mpOffscreenBitmap->Canvas->Handle;
  ::SelectClipRgn( dc, Form->updateRgn );

  // Clear the background.
  ::FillRect( dc, &formRect, gdi[ backgroundBrush ] );

  // Draw the signal.
  SyncGraphics();
  switch( mDisplayMode )
  // Ideally, this distinction should be implemented as subclassing.
  // However, this wouldn't allow switching display modes for windows
  // that already exist.
  {
    case polyline:
    {
      float  baseInterval = mDataHeight / mNumDisplayGroups;
      // Draw the baselines.
      if( mShowBaselines )
      {
        POINT baselinePoints[ 2 ];
        ::SelectObject( dc, gdi[ baselinePen ] );
        baselinePoints[ 0 ].x = SampleLeft( 0 );
        baselinePoints[ 1 ].x = SampleRight( mNumSamples );
        for( size_t i = 0; i < mNumDisplayGroups; ++i )
        {
          baselinePoints[ 0 ].y = ChannelBottom( i ) + ( baseInterval * mMinValue ) / ( mMaxValue - mMinValue );
          baselinePoints[ 1 ].y = baselinePoints[ 0 ].y;
          ::Polyline( dc, baselinePoints, 2 );
        }
      }

      mSignalPoints.resize( mNumSamples );
      for( size_t i = 0; i < mNumDisplayChannels; ++i )
      {
        for( size_t j = 0; j < mNumSamples; ++j )
        {
          mSignalPoints[ j ].x = SampleLeft( j );
          mSignalPoints[ j ].y = ChannelBottom( i ) - 1
               - baseInterval * NormData( i + mBottomGroup * mChannelGroupSize, j );
        }

        ::SelectObject( dc, signalPens[ ( i + mBottomGroup * mChannelGroupSize ) % signalPens.size() ] );
        ::Polyline( dc, mSignalPoints, mSampleCursor );
        ::Polyline( dc, &mSignalPoints[ mSampleCursor ], mNumSamples - mSampleCursor );

        // We actually need this strange distinction of cases.
        if( mShowCursor && mSampleCursor != 0 && mNumSamples > 1 )
        {
          POINT remainingPoints[ 2 ];
          remainingPoints[ 0 ] = mSignalPoints[ mNumSamples - 1 ];
          remainingPoints[ 1 ].x = SampleLeft( mNumSamples );
          if( mWrapAround )
            remainingPoints[ 1 ].y = mSignalPoints[ 0 ].y;
          else
            remainingPoints[ 1 ].y = mSignalPoints[ mNumSamples - 1 ].y;
          ::Polyline( dc, remainingPoints, 2 );
        }
        ::Sleep( 0 );
      }

      if( mShowValueUnit )
      {
        // Find a round value that is near the display range.
        float unitsPerPixel = ::fabs( ( mMaxValue - mMinValue ) * mUnitsPerValue / baseInterval ),
              scale = ::pow( 10, ::ceil( ::log10( unitsPerPixel * 0.95 * baseInterval ) ) ),
              rulerLength = scale;
        while( rulerLength / unitsPerPixel >= 0.95 * baseInterval )
          rulerLength -= scale / 10;
        int pixelLength = rulerLength / unitsPerPixel;

        ostringstream label;
        label << rulerLength << mValueUnit;
        if( mMinValue == 0 )
        {
          int left = SampleLeft( 0 ) + tickLength,
              top = ChannelBottom( 0 ) - pixelLength;
          RECT labelRect =
          {
            left,
            top,
            left,
            top
          };
          ::DrawText( dc, label.str().c_str(), -1, &labelRect,
              DT_TOP | DT_SINGLELINE | DT_LEFT | DT_NOCLIP );
          ::DrawText( dc, label.str().c_str(), -1, &labelRect,
              DT_TOP | DT_SINGLELINE | DT_LEFT | DT_NOCLIP | DT_CALCRECT );
          RECT lineRect =
          {
            SampleLeft( 0 ),
            labelRect.top,
            left,
            labelRect.top + 1
          };
          ::FillRect( dc, &lineRect, gdi[ markerBrush ] );
        }
        else
        {
          int left = SampleLeft( 0 ),
              center = ChannelBottom( 0 ) - baseInterval / 2;
          RECT labelRect =
          {
            left,
            center,
            left,
            center
          };
          ::DrawText( dc, label.str().c_str(), -1, &labelRect,
              DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP );
          ::DrawText( dc, label.str().c_str(), -1, &labelRect,
              DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP | DT_CALCRECT );
          RECT lineRect =
          {
            labelRect.left,
            center - ( pixelLength + markerWidth ) / 2,
            labelRect.right,
            center - ( pixelLength - markerWidth ) / 2
          };
          ::FillRect( dc, &lineRect, gdi[ markerBrush ] );
          lineRect.top += pixelLength;
          lineRect.bottom += pixelLength;
          ::FillRect( dc, &lineRect, gdi[ markerBrush ] );
        }
      }
    } break;

    case field2d:
    {
      for( size_t i = 0; i < mNumDisplayChannels; ++i )
      {
        for( size_t j = 0; j < mNumSamples; ++j )
        {
          RECT dotRect =
          {
            SampleLeft( j ),
            ChannelTop( i ),
            SampleRight( j ),
            ChannelBottom( i )
          };
          if( ::RectVisible( dc, &dotRect ) )
          {
            float dataValue = NormData( i + mBottomGroup * mChannelGroupSize, j );
            if( dataValue < 0.0 )
              dataValue = 0.0;
            else if( dataValue > 1.0 )
              dataValue = 1.0;

            LONG color;
            if( mDisplayColors )
              color = RGBColor::HSVColor( dataValue - 1.0 / 3.0, 1.0, dataValue );
            else
              color = RGBColor::HSVColor( 0.0, 0.0, dataValue );
            // SetBkColor/ExtTextOut is faster than CreateSolidBrush/FillRect/DeleteObject.
            ::SetBkColor( dc, color );
            ::ExtTextOut( dc, dotRect.left, dotRect.top, ETO_OPAQUE, &dotRect, "", 0, NULL );
          }
        }
        ::Sleep( 0 );
      }
    } break;

    default:
      assert( false );
  }

  // Draw the cursor.
  if( mShowCursor )
  {
    size_t cursorSample = mSampleCursor;
    if( cursorSample == 0 )
      cursorSample = mNumSamples;

    RECT cursorRect =
    {
      SampleLeft( cursorSample ) - cursorWidth,
      0,
      SampleLeft( cursorSample ),
      formRect.bottom - cLabelWidth
    };
    ::FillRect( dc, &cursorRect, gdi[ cursorBrush ] );
    // Remember the cursor rectangle for redrawing when the next
    // data packet arrives.
    HRGN rectRgn = ::CreateRectRgnIndirect( &cursorRect );
    ::CombineRgn( mRedrawRgn, mRedrawRgn, rectRgn, RGN_OR );
    ::DeleteObject( rectRgn );
  }

  // Draw the ticks.
  ::SetTextColor( dc, labelColor );
  ::SelectObject( dc, gdi[ labelFont ] );
  ::SetBkMode( dc, TRANSPARENT );
  // Ticks on the y axis.
  switch( mDisplayMode )
  {
    case field2d:
      break;
    case polyline:
      {
        int nextLabelPos = mDataRect.bottom;
        for( size_t i = 0; i < mNumDisplayGroups; ++i )
        {
          int channelNumber = ( mBottomGroup + i ) * mChannelGroupSize,
              tickY = ( GroupBottom( i ) + GroupBottom( i + 1 ) ) / 2;
          RECT tickRect =
          {
            cLabelWidth - axisWidth - tickLength,
            tickY - tickWidth / 2,
            cLabelWidth - axisWidth,
            tickY + tickWidth / 2
          };
          if( mDisplayColors && mChannelGroupSize == 1 )
          {
            tickRect.top -= 1;
            tickRect.bottom += 1;
            ::FillRect( dc, &tickRect, signalBrushes[ channelNumber ] );
          }
          else
            ::FillRect( dc, &tickRect, gdi[ axisBrush ] );
          if( tickY < nextLabelPos )
          {
            tickRect.right -= 2 * axisWidth;
            nextLabelPos = tickY - ::DrawText( dc,
               IntToStr( channelNumber + cChannelBase ).c_str(),
               -1, &tickRect, DT_RIGHT | DT_SINGLELINE | DT_VCENTER | DT_NOCLIP );
          }
        }
      } break;
    default:
      assert( false );
  }
  // Ticks on the x axis.
  float displayLength = ::fabs( mNumSamples * mUnitsPerSample ),
        scale = ::pow( 10, ::floor( ::log10( displayLength ) + 0.5 ) ),
        xDivision = scale / mUnitsPerSample / 5,
        xStart = xDivision;
  if( xDivision < 1 )
    xDivision = 1;
  int nextLabelPos = mDataRect.left;
  for( float j = xStart; j < float( mNumSamples ); j += xDivision )
  {
    int tickX = SampleRight( j );
    RECT tickRect =
    {
      tickX - tickWidth / 2,
      mDataHeight + axisWidth,
      tickX + tickWidth / 2,
      mDataHeight + axisWidth + tickLength
    };
    ::FillRect( dc, &tickRect, gdi[ axisBrush ] );
    if( tickX > nextLabelPos )
    {
      tickRect.top += 2 * axisWidth;
      ostringstream label;
      label << setprecision( 6 ) << ' ' << j * mUnitsPerSample << mSampleUnit << ' ';
      ::DrawText( dc, label.str().c_str(), -1, &tickRect,
         DT_TOP | DT_SINGLELINE | DT_CENTER | DT_NOCLIP );
      SIZE textSize;
      ::GetTextExtentPoint32( dc, label.str().c_str(), label.str().length(), &textSize );
      nextLabelPos = tickX + textSize.cx;
    }
  }

  // Draw the axes.
  RECT xAxis =
  {
    0,
    mDataHeight,
    formRect.right,
    mDataHeight + axisWidth
  };
  ::FillRect( dc, &xAxis, gdi[ axisBrush ] );
  RECT yAxis =
  {
    cLabelWidth - axisWidth,
    0,
    cLabelWidth,
    formRect.bottom
  };
  ::FillRect( dc, &yAxis, gdi[ axisBrush ] );

  // Draw markers.
  for( size_t i = 0; i < mXAxisMarkers.size(); ++i )
  {
    int markerX = SampleRight( mXAxisMarkers[ i ].Address() );
    RECT markerBar =
    {
      markerX - axisWidth / 2,
      mDataHeight - 4 * axisWidth,
      markerX + axisWidth / 2,
      mDataHeight - 1
    };
    ::FillRect( dc, &markerBar, gdi[ markerBrush ] );
  }

  // Draw channel labels.
  if( mShowChannelLabels )
  {
    ::SetBkColor( dc, clBlack );
    ::SetBkMode( dc, OPAQUE );
    RECT legendRect =
    {
      0, 0, 0, 0
    };
    for( size_t i = 0; i < mChannelLabels.size(); ++i )
    {
      ::SetTextColor( dc, ChannelColor( mChannelLabels[ i ].Address() ) );
      legendRect.top += ::DrawText( dc, mChannelLabels[ i ].Text().c_str(),
        mChannelLabels[ i ].Text().length(), &legendRect,
        DT_SINGLELINE | DT_LEFT | DT_NOCLIP | DT_EXTERNALLEADING );
    }
  }

  // Copy the data from the buffer onto the screen.
#if TEST_UPDATE_REGIONS
  Form->Canvas->Brush->Color = static_cast<TColor>( ::random( 0x1000000 ) );
  Form->Canvas->FillRect( Form->ClientRect );
#else
  Form->Canvas->Draw( 0, Form->ClientHeight - mpOffscreenBitmap->Height, mpOffscreenBitmap );
#endif
}

////////////////////////////////////////////////////////////////////////////////
VISUAL::Memo::Memo( id_type inSourceID )
: VisualBase( inSourceID ),
  mpMemo( new TMemo( ( TComponent* )NULL ) ),
  mNumLines( 0 )
{
  Restore();
}

VISUAL::Memo::~Memo()
{
  Save();
  delete mpMemo;
}

void
VISUAL::Memo::SetConfig( config_settings& inConfig )
{
  VisualBase::SetConfig( inConfig );
  inConfig.Get( CFGID::numLines, mNumLines );
  if( mNumLines < 1 )
    mNumLines = numeric_limits<int>::max();
}

void
VISUAL::Memo::Restore()
{
  if( form == NULL )
    form = new TVisForm();
  VisualBase::Restore();
  form->Show();
  mpMemo->Visible = false;
  mpMemo->Parent = form;
  mpMemo->BoundsRect = form->ClientRect;
  mpMemo->Anchors << akLeft << akTop << akRight << akBottom;
  mpMemo->ScrollBars = ssVertical;
  mpMemo->ReadOnly = true;
  mpMemo->Visible = true;
}

void
VISUAL::Memo::Save() const
{
  VisualBase::Save();
  Visconfigs()[ sourceID ].Put( CFGID::numLines, mNumLines, MessageDefined );
}

void
VISUAL::Memo::HandleMessage( const VisMemo& v )
{
  Memo* visual = dynamic_cast<Memo*>( Visuals()[ v.GetSourceID() ] );
  if( visual == NULL )
  {
    delete Visuals()[ v.GetSourceID() ];
    visual = new Memo( v.GetSourceID() );
    Visuals()[ v.GetSourceID() ] = visual;
  }
  visual->InstanceHandleMessage( v );
}

void
VISUAL::Memo::InstanceHandleMessage( const VisMemo& v )
{
  while( mpMemo->Lines->Count >= mNumLines )
    mpMemo->Lines->Delete( 0 );
  string s = v.GetMemoText();
  size_t pos = 0;
  while( ( pos = s.find_first_of( "\n\r" ) ) != s.npos )
  {
    mpMemo->Lines->Add( s.substr( 0, pos ).c_str() );
    s.erase( 0, pos + 1 );
  }
  if( !s.empty() )
    mpMemo->Lines->Add( s.c_str() );
}

