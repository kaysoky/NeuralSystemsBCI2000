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

#pragma package( smart_init )

using namespace std;

VISUAL::VISUAL_BASE::vis_container VISUAL::VISUAL_BASE::visuals;
VISUAL::VISUAL_BASE::config_container VISUAL::VISUAL_BASE::visconfigs;
const char* key_base = KEY_BCI2000 KEY_OPERATOR KEY_VISUALIZATION "\\";

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
}

void
VISUAL::VISUAL_BASE::Restore()
{
  assert( form != NULL );

  form->BorderStyle = bsSizeToolWin;

  AnsiString key = key_base + AnsiString( sourceID );
  TRegistry* reg = new TRegistry( KEY_READ );
  if( reg->OpenKeyReadOnly( key ) )
  {
    try
    {
      form->Top = reg->ReadInteger( "Top" );
      form->Left = reg->ReadInteger( "Left" );
      form->Width = reg->ReadInteger( "Width" );
      form->Height = reg->ReadInteger( "Height" );
    }
    catch( ERegistryException& )
    {
      static int top = 10,
                 left = 10;
      form->Top = top;
      top += 10;
      form->Left = left;
      left += 10;
      form->Height = 100;
      form->Width = 100;
    }
  }
  delete reg;

  if( !::Types::PtInRect( form->ClientRect, TPoint( 10, 10 ) ) )
  {
    form->Height = 100;
    form->Width = 100;
  }
  SetConfig( visconfigs[ sourceID ] );
}

void
VISUAL::VISUAL_BASE::Save() const
{
  assert( form != NULL );

  AnsiString key = key_base + AnsiString( sourceID );
  TRegistry* reg = new TRegistry( KEY_WRITE );
  if( reg->OpenKey( key, true ) )
  {
    try
    {
      reg->WriteInteger( "Top", form->Top );
      reg->WriteInteger( "Left", form->Left );
      reg->WriteInteger( "Width", form->Width );
      reg->WriteInteger( "Height", form->Height );
      // This is just for the user to recognize the registry entry for
      // manual editing.
      if( title != "" )
        reg->WriteString( "WindowTitle", title.c_str() );
    }
    catch( ERegistryException& )
    {
    }
  }
  delete reg;
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
    visconfigs[ sourceID ][ cfgID ] = value;
    if( visuals[ sourceID ] != NULL )
      visuals[ sourceID ]->SetConfig( visconfigs[ sourceID ] );
  }
  return is;
}

////////////////////////////////////////////////////////////////////////////////
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
  offscreenBitmap( new Graphics::TBitmap )
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
  if( inConfig[ CFGID::MINVALUE ] != "" )
    minValue = ::atof( inConfig[ CFGID::MINVALUE ].c_str() );
  if( inConfig[ CFGID::MAXVALUE ] != "" )
    maxValue = ::atof( inConfig[ CFGID::MAXVALUE ].c_str() );
  if( inConfig[ CFGID::NUMSAMPLES ] != "" )
    numSamples = ::atof( inConfig[ CFGID::NUMSAMPLES ].c_str() );
  if( inConfig[ CFGID::channelGroupSize ] != "" )
  {
    channelGroupSize = ::atoi( inConfig[ CFGID::channelGroupSize ].c_str() );
    if( channelGroupSize < 1 )
      channelGroupSize = numeric_limits<size_t>::max();
  }
  if( inConfig[ CFGID::graphType ] != "" )
    switch( ::atoi( inConfig[ CFGID::graphType ].c_str() ) )
    {
      case CFGID::polyline:
        displayMode = polyline;
        break;
      case CFGID::colorfield:
        displayMode = colorfield;
        break;
    }
  if( inConfig[ CFGID::showBaselines ] != "" )
    showBaselines = ::atoi( inConfig[ CFGID::showBaselines ].c_str() );

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
    form = new TVisForm;
    BuildContextMenu();
  }
  VISUAL_BASE::Restore();
  form->OnKeyUp = FormKeyUp;
  form->OnResize = FormResize;
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

  if( !( data >= newData ) )
  {
    if( newData.MaxElements() > numSamples )
      numSamples = newData.MaxElements();
    numDisplayGroups = ( newData.Channels() - 1 ) / channelGroupSize + 1;
    switch( displayMode )
    {
      case polyline:
        numDisplayGroups = min( numDisplayGroups, maxDisplayGroups );
        break;
      case colorfield:
      default:
        break;
    }
    numDisplayChannels = min( newData.Channels(), numDisplayGroups * channelGroupSize );
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
    case colorfield:
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
  return displayMode == polyline;
}

bool
VISUAL::VISUAL_GRAPH::ToggleColor_Checked() const
{
  return displayColors;
}

void
VISUAL::VISUAL_GRAPH::EnlargeSignal()
{
  float offset = ( minValue + maxValue ) / 2,
        unit = maxValue - offset;
  unit /= 2;
  minValue = offset - unit;
  maxValue = offset + unit;
  form->Invalidate();
}

bool
VISUAL::VISUAL_GRAPH::EnlargeSignal_Enabled() const
{
  return maxValue > 1 && minValue < -1;
}

void
VISUAL::VISUAL_GRAPH::ReduceSignal()
{
  float offset = ( minValue + maxValue ) / 2,
        unit = maxValue - offset;
  unit *= 2;
  minValue = offset - unit;
  maxValue = offset + unit;
  form->Invalidate();
}

bool
VISUAL::VISUAL_GRAPH::ReduceSignal_Enabled() const
{
  return maxValue < 1 << 16 && maxValue > -( 1 << 16 );
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
VISUAL::VISUAL_GRAPH::FormResize( TObject* Sender )
{
  TForm* Form = static_cast<TForm*>( Sender );
  Form->Invalidate();
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
  {
    const TColor signalColors[] =
    { clWhite, clWhite, clWhite, clWhite, clYellow };
    size_t numColors = sizeof( signalColors ) / sizeof( *signalColors );
    for( size_t i = 0; i < data.Channels(); ++i )
    {
      int colorIndex = i % numColors;
      signalPens[ i ] = ::CreatePen( PS_SOLID, 0, signalColors[ colorIndex ] );
      signalBrushes[ i ] = ::CreateSolidBrush( signalColors[ colorIndex ] );
    }
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

    case colorfield:
    {
      struct // A wrapper to locally define a function (which is not possible otherwise).
      {
        COLORREF operator()( float H, float S, float V )
        {
          // According to Foley and VanDam.
          // All input components range from 0 to 1 - EPS.
          float h = 6.0 * ::fmod( H, 1.0 );
          if( h < 0.0 )
            h += 6.0;
          int i = ::floor( h );
          float f = h - i;
          if( !( i % 2 ) )
            f = 1.0 - f;
          unsigned int m = ( V * ( 1.0 - S ) ) * 0x100,
                       n = ( V * ( 1.0 - S * f ) ) * 0x100,
                       v = V * 0x100;
          if( m > 0xff )
            m = 0xff;
          if( n > 0xff )
            n = 0xff;
          if( v > 0xff )
            v = 0xff;
          COLORREF rgb = RGB( 0, 0, 0 );
          switch( i )
          {
            case 0:
              rgb = RGB( v, n, m );
              break;
            case 1:
              rgb = RGB( n, v, m );
              break;
            case 2:
              rgb = RGB( m, v, n );
              break;
            case 3:
              rgb = RGB( m, n, v );
              break;
            case 4:
              rgb = RGB( n, m, v );
              break;
            case 5:
              rgb = RGB( v, m, n );
              break;
            default:
              assert( false );
          }
          return rgb;
        };
      } HSVColor;

      for( size_t i = 0; i < numDisplayChannels; ++i )
        for( size_t j = 0; j < numSamples; ++j )
        {
          LONG color = HSVColor( 2.0 / 3.0 * ( 1.0 - NormData( i + bottomGroup * channelGroupSize, j ) ), 1.0, 1.0 );
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
    case colorfield:
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
  Form->Canvas->Draw( 0, Form->ClientHeight - offscreenBitmap->Height, offscreenBitmap );
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
  if( inConfig[ CFGID::numLines ] != "" )
    numLines = ::atoi( inConfig[ CFGID::numLines ].c_str() );
  if( numLines < 1 )
    numLines = numeric_limits<int>::max();
}

void
VISUAL::VISUAL_MEMO::Restore()
{
  if( form == NULL )
    form = new TForm( ( TComponent* )NULL );
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

