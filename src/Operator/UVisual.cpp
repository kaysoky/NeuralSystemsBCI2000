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
//---------------------------------------------------------------------------
#include "UVisual.h"

#include "defines.h"
#include "UCoreMessage.h" // for COREMSG_DATA

#include <assert>
#include <Registry.hpp>
#include <math.h>
#include <algorithm>
#include <sstream>
#include <vcl.h>
#include <grids.hpp>

#pragma package( smart_init )

using namespace std;

const char* key_base = KEY_BCI2000 KEY_OPERATOR KEY_VISUALIZATION "\\";

VISUAL::VISUAL_BASE::vis_container VISUAL::VISUAL_BASE::visuals;
VISUAL::VISUAL_BASE::config_container VISUAL::VISUAL_BASE::visconfigs;

////////////////////////////////////////////////////////////////////////////////
const char* cfgid_prefix = "CFGID"; // const AnsiString cfgid_prefix = "CFGID"; won't work.

void
VISUAL::VISUAL_BASE::config_container::Save()
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
VISUAL::VISUAL_BASE::config_container::Restore()
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
VISUAL::VISUAL_BASE::VISUAL_BASE( id_type inSourceID )
: sourceID( inSourceID ),
  form( NULL )
{
  VISUAL_BASE* visual = visuals[ sourceID ];
  delete visual;
  visuals[ sourceID ] = this;
}

VISUAL::VISUAL_BASE::~VISUAL_BASE()
{
  delete form;
}

void
VISUAL::VISUAL_BASE::vis_container::clear()
{
  for( iterator i = begin(); i != end(); ++i )
    delete i->second;
  vis_container_base::clear();
}

void
VISUAL::VISUAL_BASE::SetConfig( config_settings& inConfig )
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
VISUAL::VISUAL_BASE::Restore()
{
  assert( form != NULL );
  form->BorderStyle = bsSizeToolWin;
  form->OnMove = FormMove;
  form->OnResize = FormResize;
  SetConfig( visconfigs[ sourceID ] );
}

void
VISUAL::VISUAL_BASE::Save() const
{
  assert( form != NULL );
}

bool
VISUAL::VISUAL_BASE::HandleMessage( istream& is )
{
  if( is.peek() != VISTYPE::VISCFG )
    return  VISUAL_GRAPH::HandleMessage( is )
            || VISUAL_MEMO::HandleMessage( is );

  is.ignore( 3 );
  int sourceID = is.get(),
      cfgID = is.get();
  string value;
  if( getline( is >> ws, value, '\0' ) )
  {
    visconfigs[ sourceID ].Put( cfgID, value, MessageDefined );
    if( visuals[ sourceID ] != NULL )
      visuals[ sourceID ]->SetConfig( visconfigs[ sourceID ] );
  }
  return is;
}

void
__fastcall
VISUAL::VISUAL_BASE::FormMove( TObject* )
{
  visconfigs[ sourceID ].Put( CFGID::Top, form->Top, UserDefined );
  visconfigs[ sourceID ].Put( CFGID::Left, form->Left, UserDefined );
}

void
__fastcall
VISUAL::VISUAL_BASE::FormResize( TObject* Sender )
{
  TForm* Form = static_cast<TForm*>( Sender );
  Form->Invalidate();
  visconfigs[ sourceID ].Put( CFGID::Width, form->Width, UserDefined );
  visconfigs[ sourceID ].Put( CFGID::Height, form->Height, UserDefined );
}

////////////////////////////////////////////////////////////////////////////////
const RGBColor VISUAL::VISUAL_GRAPH::channelColorsDefault[] =
{
  White,
  White,
  White,
  White,
  Yellow,
  Colorlist::End
};

VISUAL::VISUAL_GRAPH::VISUAL_GRAPH( id_type inSourceID )
: VISUAL_BASE( inSourceID ),
  showCursor( false ),
  wrapAround( false ),
  numSamples( numSamplesDefault ),
  sampleCursor( 0 ),
  numDisplayGroups( 0 ),
  numDisplayChannels( 0 ),
  bottomGroup( 0 ),
  showBaselines( false ),
  displayColors( true ),
  channelGroupSize( 1 ),
  minValue( minValueDefault ),
  maxValue( maxValueDefault ),
  displayMode( polyline ),
  redrawRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ),
  offscreenBitmap( new Graphics::TBitmap ),
  channelColors( channelColorsDefault ),
  mUserScaling( 0 )
{
  Restore();
}

VISUAL::VISUAL_GRAPH::~VISUAL_GRAPH()
{
  Save();
  delete offscreenBitmap;
  ::DeleteObject( redrawRgn );
}

void
VISUAL::VISUAL_GRAPH::SetConfig( config_settings& inConfig )
{
  VISUAL_BASE::SetConfig( inConfig );

  int userScaling = mUserScaling;
  mUserScaling = 0;
  inConfig.Get( CFGID::MINVALUE, minValue );
  inConfig.Get( CFGID::MAXVALUE, maxValue );
  for( int i = 0; i < userScaling; ++i )
    EnlargeSignal();
  for( int i = 0; i > userScaling; --i )
    ReduceSignal();

  inConfig.Get( CFGID::NUMSAMPLES, numSamples );
  inConfig.Get( CFGID::channelGroupSize, channelGroupSize );
  if( channelGroupSize < 1 )
    channelGroupSize = numeric_limits<size_t>::max();
  int graphType = CFGID::polyline;
  inConfig.Get( CFGID::graphType, graphType );
  switch( graphType )
  {
    case CFGID::polyline:
      displayMode = polyline;
      break;
    case CFGID::field2d:
      displayMode = field2d;
      break;
  }
  inConfig.Get( CFGID::showBaselines, showBaselines );
  inConfig.Get( CFGID::channelColors, channelColors );

  // Sanity checks.
  if( minValue == maxValue )
    maxValue = minValue + 1;
  if( numSamples < 1 )
    numSamples = 1;
  for( size_t i = 0; i < data.Channels(); ++i )
    data.SetNumElements( i, numSamples );
  form->Invalidate();
}

void
VISUAL::VISUAL_GRAPH::Restore()
{
  if( form == NULL )
  {
    form = new TVisGraphForm;
    BuildContextMenu();
  }
  VISUAL_BASE::Restore();
  form->OnKeyUp = FormKeyUp;
  form->OnPaint = FormPaint;
  form->Show();
}

void
VISUAL::VISUAL_GRAPH::Save() const
{
  VISUAL_BASE::Save();
}

bool
VISUAL::VISUAL_GRAPH::HandleMessage( istream& is )
{
  if( is.peek() != VISTYPE::GRAPH )
    return false;

  is.ignore( 3 );
  int sourceID = is.get();
  VISUAL_GRAPH* visual = dynamic_cast<VISUAL_GRAPH*>( visuals[ sourceID ] );
  if( visual == NULL )
  {
    delete visuals[ sourceID ];
    visual = new VISUAL_GRAPH( sourceID );
    visuals[ sourceID ] = visual;
  }
  return visual->InstanceHandleMessage( is );
}

bool
VISUAL::VISUAL_GRAPH::InstanceHandleMessage( istream& is )
{
  GenericSignal newData;

  if( !newData.ReadBinary( is ) )
    return false;
  if( newData.Channels() < 1 || newData.MaxElements() < 1 )
    return true;

  // Any changes in the signal size that we must react to?
  bool reconfigure = false;
  if( newData.MaxElements() > numSamples )
  {
    numSamples = newData.MaxElements();
    reconfigure = true;
  }
  if( newData.Channels() != data.Channels() )
  {
    numDisplayGroups = ( newData.Channels() - 1 ) / channelGroupSize + 1;
    switch( displayMode )
    {
      case polyline:
        numDisplayGroups = min( numDisplayGroups, maxDisplayGroups );
        break;
      case field2d:
      default:
        break;
    }
    numDisplayChannels = min( newData.Channels(), numDisplayGroups * channelGroupSize );
    reconfigure = true;
  }
  if( reconfigure )
  {
    data = GenericSignal( newData.Channels(), numSamples );
    sampleCursor = 0;
    form->Invalidate();
  }

  showCursor = ( newData.MaxElements() < numSamples );

  for( size_t i = 0; i < newData.Channels(); ++i )
    for( size_t j = 0; j < newData.GetNumElements( i ); ++j )
      data( i, ( sampleCursor + j ) % data.GetNumElements( i ) ) = newData( i, j );

  SyncGraphics();

  int firstInvalidSample = sampleCursor,
      firstValidSample = sampleCursor + newData.MaxElements();
  sampleCursor = firstValidSample % numSamples;
  wrapAround |= bool( firstValidSample / numSamples > 0  );

  long firstInvalidPixel = dataRect.left,
       firstValidPixel = dataRect.right;

  switch( displayMode )
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
  ::InvalidateRgn( form->Handle, redrawRgn, false );
  ::SetRectRgn( redrawRgn, 0, 0, 0, 0 );

  RECT invalidRect = dataRect;

  // The non-wrapped area.
  invalidRect.left = max( firstInvalidPixel, dataRect.left );
  invalidRect.right = min( firstValidPixel, dataRect.right );
  if( invalidRect.left < invalidRect.right )
    ::InvalidateRect( form->Handle, &invalidRect, false );

  // The area wrapped around the left edge.
  invalidRect.left = max( firstInvalidPixel + dataWidth, dataRect.left );
  invalidRect.right = min( firstValidPixel + dataWidth, dataRect.right );
  if( invalidRect.left < invalidRect.right )
    ::InvalidateRect( form->Handle, &invalidRect, false );

  // The area wrapped around the right edge.
  invalidRect.left = max( firstInvalidPixel - dataWidth, dataRect.left );
  invalidRect.right = min( firstValidPixel - dataWidth, dataRect.right );
  if( invalidRect.left < invalidRect.right )
    ::InvalidateRect( form->Handle, &invalidRect, false );

  return true;
}

inline
void
VISUAL::VISUAL_GRAPH::SyncGraphics()
{
  dataRect = TRect( labelWidth, 0, form->ClientWidth, form->ClientHeight );
  dataWidth = std::max<int>( 0, dataRect.right - dataRect.left );
  dataHeight = std::max<int>( 0, dataRect.bottom - dataRect.top - labelWidth );
}

struct VISUAL::VISUAL_GRAPH::MenuItemEntry VISUAL::VISUAL_GRAPH::sMenuItems[] =
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
  { ToggleBaselines, ToggleBaselines_Enabled, ToggleBaselines_Checked, "Show Baselines" },
};

void
VISUAL::VISUAL_GRAPH::BuildContextMenu()
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
VISUAL::VISUAL_GRAPH::PopupMenuPopup( TObject* inSender )
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
VISUAL::VISUAL_GRAPH::PopupMenuItemClick( TObject* inSender )
{
  TMenuItem* item = dynamic_cast<TMenuItem*>( inSender );
  assert( item != NULL );
  MenuItemEntry::MenuAction action = sMenuItems[ item->Tag ].mAction;
  assert( action != NULL );
  ( this->*action )();
}

void
VISUAL::VISUAL_GRAPH::ToggleDisplayMode()
{
  SetDisplayMode( DisplayMode( ( displayMode + 1 ) % numDisplayModes ) );
}

void
VISUAL::VISUAL_GRAPH::ToggleBaselines()
{
  showBaselines = !showBaselines;
  visconfigs[ sourceID ].Put( CFGID::showBaselines, showBaselines, UserDefined );
  form->Invalidate();
}

bool
VISUAL::VISUAL_GRAPH::ToggleBaselines_Enabled() const
{
  return displayMode == polyline;
}

bool
VISUAL::VISUAL_GRAPH::ToggleBaselines_Checked() const
{
  return showBaselines;
}

void
VISUAL::VISUAL_GRAPH::ToggleColor()
{
  displayColors = !displayColors;
  form->Invalidate();
}

bool
VISUAL::VISUAL_GRAPH::ToggleColor_Enabled() const
{
  return displayMode == polyline || displayMode == field2d;
}

bool
VISUAL::VISUAL_GRAPH::ToggleColor_Checked() const
{
  return displayColors;
}

void
VISUAL::VISUAL_GRAPH::ChooseColors()
{
  // The dialog's "custom colors" are used to hold the user colors.
  // Maybe this should be changed in the future.
  const numCustomColors = 16;
  COLORREF customColors[ numCustomColors ];
  for( int i = 0; i < ::min<int>( channelColors.size(), numCustomColors ); ++i )
    customColors[ i ] = channelColors[ i ];
  for( int i = channelColors.size(); i < numCustomColors; ++i )
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
      channelColors.resize( 1, White );
    else
    {
      channelColors.resize( numUserColors );
      for( int i = 0; i < numUserColors; ++i )
        channelColors[ i ] = customColors[ i ];
    }
    visconfigs[ sourceID ].Put( CFGID::channelColors, channelColors, UserDefined );
    form->Invalidate();
  }
}

bool
VISUAL::VISUAL_GRAPH::ChooseColors_Enabled() const
{
  return displayColors && displayMode == polyline;
}

void
VISUAL::VISUAL_GRAPH::EnlargeSignal()
{
#if 0
  float offset = ( minValue + maxValue ) / 2,
        unit = maxValue - offset;
  unit /= 2;
  minValue = offset - unit;
  maxValue = offset + unit;
#else
  minValue /= 2;
  maxValue /= 2;
#endif
  ++mUserScaling;
  form->Invalidate();
}

bool
VISUAL::VISUAL_GRAPH::EnlargeSignal_Enabled() const
{
  return mUserScaling < maxUserScaling;
}

void
VISUAL::VISUAL_GRAPH::ReduceSignal()
{
#if 0
  float offset = ( minValue + maxValue ) / 2,
        unit = maxValue - offset;
  unit *= 2;
  minValue = offset - unit;
  maxValue = offset + unit;
#else
  minValue *= 2;
  maxValue *= 2;
#endif
  --mUserScaling;
  form->Invalidate();
}

bool
VISUAL::VISUAL_GRAPH::ReduceSignal_Enabled() const
{
  return mUserScaling > -maxUserScaling;
}

void
VISUAL::VISUAL_GRAPH::LessChannels()
{
  SetDisplayGroups( numDisplayGroups / 2 );
}

bool
VISUAL::VISUAL_GRAPH::LessChannels_Enabled() const
{
  return numDisplayGroups > 1;
}

void
VISUAL::VISUAL_GRAPH::MoreChannels()
{
  SetDisplayGroups( numDisplayGroups * 2 );
}

bool
VISUAL::VISUAL_GRAPH::MoreChannels_Enabled() const
{
  return numDisplayGroups < data.Channels() / channelGroupSize;
}

void
VISUAL::VISUAL_GRAPH::SetDisplayGroups( int inDisplayGroups )
{
  numDisplayGroups = inDisplayGroups;
  numDisplayChannels = min( data.Channels(), numDisplayGroups * channelGroupSize );
  SetBottomGroup( bottomGroup );
  form->Invalidate();
}

void
VISUAL::VISUAL_GRAPH::SetBottomGroup( int inBottomGroup )
{
  int newBottomGroup = inBottomGroup,
      maxBottomGroup = ChannelToGroup( data.Channels() ) - int( numDisplayGroups );
  if( newBottomGroup > maxBottomGroup )
    newBottomGroup = maxBottomGroup;
  if( newBottomGroup < 0 )
    newBottomGroup = 0;
  if( ( size_t )newBottomGroup != bottomGroup )
  {
    bottomGroup = newBottomGroup;
    form->Invalidate();
  }
}

void
VISUAL::VISUAL_GRAPH::SetDisplayMode( DisplayMode mode )
{
  if( mode != displayMode )
  {
    form->Invalidate();
    displayMode = mode;
  }
}

void
__fastcall
VISUAL::VISUAL_GRAPH::FormKeyUp( TObject*, WORD& key, TShiftState )
{
  switch( key )
  {
    case VK_UP:
      SetBottomGroup( bottomGroup + 1 );
      break;
    case VK_DOWN:
      SetBottomGroup( bottomGroup - 1 );
      break;
    case VK_PRIOR:
      SetBottomGroup( bottomGroup + numDisplayGroups / 2 );
      break;
    case VK_NEXT:
      SetBottomGroup( bottomGroup - numDisplayGroups / 2 );
      break;
  }
}

void
__fastcall
VISUAL::VISUAL_GRAPH::FormPaint( TObject* Sender )
{
  enum
  {
    backgroundBrush = 0,
    cursorBrush,
    axisBrush,
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
    signalPens( data.Channels() ),
    signalBrushes( data.Channels() );

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
  const xDivision = 50, xStart = xDivision;
  gdi[ axisBrush ] = ::CreateSolidBrush( axisColor );
  gdi[ baselinePen ] = ::CreatePen( PS_SOLID, 0, axisColor );

  const fontHeight = labelWidth / 2;
  const labelColor = axisColor;
  gdi[ labelFont ] = ::CreateFont( -fontHeight, 0, 0, 0, FW_DONTCARE,
                      false, false, false, ANSI_CHARSET, OUT_RASTER_PRECIS,
                      CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                      VARIABLE_PITCH | FF_SWISS, NULL );

  // Signal properties
  if( displayColors )
    for( size_t i = 0; i < data.Channels(); ++i )
    {
      signalPens[ i ] = ::CreatePen( PS_SOLID, 0, ChannelColor( i ) );
      signalBrushes[ i ] = ::CreateSolidBrush( ChannelColor( i ) );
    }
  else
  {
    HGDIOBJ pen = ::CreatePen( PS_SOLID, 0, clWhite );
    for( size_t i = 0; i < data.Channels(); ++i )
      signalPens[ i ] = pen;
  }

  // Do the drawing.
  TVisForm* Form = static_cast<TVisForm*>( Sender );
  TRect formRect = Form->ClientRect;
  if( offscreenBitmap->Width != formRect.right )
    offscreenBitmap->Width = formRect.right;
  if( offscreenBitmap->Height != formRect.bottom )
    offscreenBitmap->Height = formRect.bottom;

  HDC dc = offscreenBitmap->Canvas->Handle;
  ::SelectClipRgn( dc, Form->updateRgn );

  // Clear the background.
  ::FillRect( dc, &formRect, gdi[ backgroundBrush ] );

  // Draw the signal.
  SyncGraphics();

  switch( displayMode )
  // Ideally, this distinction should be implemented as subclassing.
  // However, this wouldn't allow switching display modes for windows
  // that already exist.
  {
    case polyline:
    {
      float  baseInterval = dataHeight / numDisplayGroups;
      // Draw the baselines.
      if( showBaselines )
      {
        POINT baselinePoints[ 2 ];
        ::SelectObject( dc, gdi[ baselinePen ] );
        baselinePoints[ 0 ].x = SampleLeft( 0 );
        baselinePoints[ 1 ].x = SampleRight( numSamples );
        for( size_t i = 0; i < numDisplayGroups; ++i )
        {
          baselinePoints[ 0 ].y = ChannelBottom( i ) + ( baseInterval * minValue ) / ( maxValue - minValue );
          baselinePoints[ 1 ].y = baselinePoints[ 0 ].y;
          ::Polyline( dc, baselinePoints, 2 );
        }
      }

      signalPoints.resize( numSamples );
      for( size_t i = 0; i < numDisplayChannels; ++i )
      {
        for( size_t j = 0; j < numSamples; ++j )
        {
          signalPoints[ j ].x = SampleLeft( j );
          signalPoints[ j ].y = ChannelBottom( i ) - 1
               - baseInterval * NormData( i + bottomGroup * channelGroupSize, j );
        }

        ::SelectObject( dc, signalPens[ ( i + bottomGroup * channelGroupSize ) % signalPens.size() ] );
        ::Polyline( dc, signalPoints, sampleCursor );
        ::Polyline( dc, &signalPoints[ sampleCursor ], numSamples - sampleCursor );

        // We actually need this strange distinction of cases.
        if( showCursor && sampleCursor != 0 && numSamples > 1 )
        {
          POINT remainingPoints[ 2 ];
          remainingPoints[ 0 ] = signalPoints[ numSamples - 1 ];
          remainingPoints[ 1 ].x = SampleLeft( numSamples );
          if( wrapAround )
            remainingPoints[ 1 ].y = signalPoints[ 0 ].y;
          else
            remainingPoints[ 1 ].y = signalPoints[ numSamples - 1 ].y;
          ::Polyline( dc, remainingPoints, 2 );
        }
      }
    } break;

    case field2d:
    {
      for( size_t i = 0; i < numDisplayChannels; ++i )
        for( size_t j = 0; j < numSamples; ++j )
        {
          float dataValue = NormData( i + bottomGroup * channelGroupSize, j );
          if( dataValue < 0.0 )
            dataValue = 0.0;
          else if( dataValue > 1.0 )
            dataValue = 1.0;
            
          LONG color;
          if( displayColors )
            color = RGBColor::HSVColor( dataValue - 1.0 / 3.0, 1.0, dataValue );
          else
            color = RGBColor::HSVColor( 0.0, 0.0, dataValue );

          HBRUSH brush = ::CreateSolidBrush( color );
          RECT dotRect =
          {
            SampleLeft( j ),
            ChannelTop( i ),
            SampleRight( j ),
            ChannelBottom( i )
          };
          ::FillRect( dc, &dotRect, brush );
          ::DeleteObject( brush );
        }
    } break;

    default:
      assert( false );
  }

  // Draw the cursor.
  if( showCursor )
  {
    size_t cursorSample = sampleCursor;
    if( cursorSample == 0 )
      cursorSample = numSamples;

    RECT cursorRect =
    {
      SampleLeft( cursorSample ) - cursorWidth,
      0,
      SampleLeft( cursorSample ),
      formRect.bottom - labelWidth
    };
    ::FillRect( dc, &cursorRect, gdi[ cursorBrush ] );
    // Remember the cursor rectangle for redrawing when the next
    // data packet arrives.
    HRGN rectRgn = ::CreateRectRgnIndirect( &cursorRect );
    ::CombineRgn( redrawRgn, redrawRgn, rectRgn, RGN_OR );
    ::DeleteObject( rectRgn );
  }

  // Draw the ticks.
  ::SetTextColor( dc, labelColor );
  ::SelectObject( dc, gdi[ labelFont ] );
  ::SetBkMode( dc, TRANSPARENT );

  // Ticks on the y axis.
  switch( displayMode )
  {
    case field2d:
      break;
    case polyline:
      {
        int nextLabelPos = dataRect.bottom;
        for( size_t i = 0; i < numDisplayGroups; ++i )
        {
          int channelNumber = ( bottomGroup + i ) * channelGroupSize,
              tickY = ( GroupBottom( i ) + GroupBottom( i + 1 ) ) / 2;
          RECT tickRect =
          {
            labelWidth - axisWidth - tickLength,
            tickY - tickWidth / 2,
            labelWidth - axisWidth,
            tickY + tickWidth / 2
          };
          if( displayColors && channelGroupSize == 1 )
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
               IntToStr( channelNumber + channelBase ).c_str(),
               -1, &tickRect, DT_RIGHT | DT_SINGLELINE | DT_VCENTER | DT_NOCLIP );
          }
        }
      } break;
    default:
      assert( false );
  }
  // Ticks on the x axis.
  int nextLabelPos = dataRect.left;
  for( size_t j = xStart; j < numSamples; j += xDivision )
  {
    int tickX = SampleRight( j );
    RECT tickRect =
    {
      tickX - tickWidth / 2,
      dataHeight + axisWidth,
      tickX + tickWidth / 2,
      dataHeight + axisWidth + tickLength
    };
    ::FillRect( dc, &tickRect, gdi[ axisBrush ] );
    if( tickX > nextLabelPos )
    {
      tickRect.top += 2 * axisWidth;
      AnsiString label = IntToStr( j + sampleBase );
      ::DrawText( dc, label.c_str(), -1, &tickRect,
         DT_TOP | DT_SINGLELINE | DT_CENTER | DT_NOCLIP );
      SIZE textSize;
      ::GetTextExtentPoint32( dc, label.c_str(), label.Length(), &textSize );
      nextLabelPos = tickX + textSize.cx;
    }
  }

  // Draw the axes.
  RECT xAxis =
  {
    0,
    dataHeight,
    formRect.right,
    dataHeight + axisWidth
  };
  ::FillRect( dc, &xAxis, gdi[ axisBrush ] );
  RECT yAxis =
  {
    labelWidth - axisWidth,
    0,
    labelWidth,
    formRect.bottom
  };
  ::FillRect( dc, &yAxis, gdi[ axisBrush ] );

  // Copy the data from the buffer onto the screen.
#if TEST_UPDATE_REGIONS
  Form->Canvas->Brush->Color = random( 0x1000000 );
  Form->Canvas->FillRect( Form->ClientRect );
#else
  Form->Canvas->Draw( 0, Form->ClientHeight - offscreenBitmap->Height, offscreenBitmap );
#endif
}

////////////////////////////////////////////////////////////////////////////////
VISUAL::VISUAL_MEMO::VISUAL_MEMO( id_type inSourceID )
: VISUAL_BASE( inSourceID ),
  memo( new TMemo( ( TComponent* )NULL ) ),
  numLines( 0 )
{
  Restore();
}

VISUAL::VISUAL_MEMO::~VISUAL_MEMO()
{
  Save();
  delete memo;
}

void
VISUAL::VISUAL_MEMO::SetConfig( config_settings& inConfig )
{
  VISUAL_BASE::SetConfig( inConfig );
  inConfig.Get( CFGID::numLines, numLines );
  if( numLines < 1 )
    numLines = numeric_limits<int>::max();
}

void
VISUAL::VISUAL_MEMO::Restore()
{
  if( form == NULL )
    form = new TVisForm();
  VISUAL_BASE::Restore();
  form->Show();
  memo->Visible = false;
  memo->Parent = form;
  memo->BoundsRect = form->ClientRect;
  memo->Anchors << akLeft << akTop << akRight << akBottom;
  memo->ScrollBars = ssVertical;
  memo->ReadOnly = true;
  memo->Visible = true;
}

void
VISUAL::VISUAL_MEMO::Save() const
{
  VISUAL_BASE::Save();
  visconfigs[ sourceID ].Put( CFGID::numLines, numLines, MessageDefined );
}

bool
VISUAL::VISUAL_MEMO::HandleMessage( istream& is )
{
  if( is.peek() != VISTYPE::MEMO )
    return false;

  is.ignore( 3 );
  int sourceID = is.get();
  VISUAL_MEMO* visual = dynamic_cast<VISUAL_MEMO*>( visuals[ sourceID ] );
  if( visual == NULL )
  {
    delete visuals[ sourceID ];
    visual = new VISUAL_MEMO( sourceID );
    visuals[ sourceID ] = visual;
  }
  return visual->InstanceHandleMessage( is );
}

bool
VISUAL::VISUAL_MEMO::InstanceHandleMessage( istream& is )
{
  string s;
  getline( is, s, '\0' );
  while( memo->Lines->Count >= numLines )
    memo->Lines->Delete( 0 );
  memo->Lines->Add( s.c_str() );
  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool
VISUAL::HandleMessage( istream& is )
{
  if( is.peek() != COREMSG_DATA )
    return false;
  is.get();
  return VISUAL_BASE::HandleMessage( is );
}

