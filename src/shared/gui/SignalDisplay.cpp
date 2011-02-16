////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A SignalDisplay class that renders GenericSignal data into a
//   given DisplayContext.
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

#include "SignalDisplay.h"

#include <cassert>
#include <cmath>
#include <sstream>
#include <iomanip>

#ifndef __BORLANDC__
# include <QWidget>
# include <QPainter>
#endif // __BORLANDC__

#undef TEST_UPDATE_RGN

using namespace std;

////////////////////////////////////////////////////////////////////////////////
const RGBColor SignalDisplay::cAxisColorDefault = RGBColor::Aqua;
const RGBColor SignalDisplay::cChannelColorsDefault[] =
{
  RGBColor::White,
  RGBColor::White,
  RGBColor::White,
  RGBColor::White,
  RGBColor::Yellow,
  ColorList::End
};

SignalDisplay::SignalDisplay()
: mDataWidth( 0 ),
  mDataHeight( 0 ),
  mLabelWidth( cLabelWidth ),
  mMarkerHeight( 0 ),
  mDisplayMode( polyline ),
  mShowCursor( false ),
  mWrapAround( false ),
  mTimeLabels( false ),
  mShowBaselines( false ),
  mShowChannelLabels( false ),
  mShowValueUnit( false ),
  mDisplayColors( true ),
  mInverted( false ),
  mNumSamples( cNumSamplesDefault ),
  mSampleCursor( 0 ),
  mNumDisplayGroups( 0 ),
  mMaxDisplayGroups( 0 ),
  mNumDisplayChannels( 0 ),
  mTopGroup( 0 ),
  mChannelGroupSize( 1 ),
  mMarkerChannels( 0 ),
  mMinValue( cMinValueDefault ),
  mMaxValue( cMaxValueDefault ),
  mSampleOffset( 0 ),
  mUnitsPerSample( 1 ),
  mUnitsPerValue( 1 ),
  mUnitsPerChannel( 1 ),
  mAxisColor( cAxisColorDefault ),
  mChannelColors( cChannelColorsDefault ),
  mTargetDC( NULL ),
#ifdef __BORLANDC__
  mRedrawRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ),
  mDisplayRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ),
#endif // __BORLANDC__
  mpSignalPoints( NULL )
{
#ifdef __BORLANDC__
  RECT r = { 0, 0, 0, 0 };
  mDisplayRect = r;
#endif // __BORLANDC__
}

SignalDisplay::~SignalDisplay()
{
#ifdef __BORLANDC__
  ::DeleteObject( mRedrawRgn );
  ::DeleteObject( mDisplayRgn );
#endif // __BORLANDC__
  delete[] mpSignalPoints;
}

SignalDisplay&
SignalDisplay::SetContext( const GUI::DrawContext& dc )
{
#ifdef __BORLANDC__
  RECT r =
  {
    dc.rect.left,
    dc.rect.top,
    dc.rect.right,
    dc.rect.bottom
  };
  mDisplayRect = r;
  ::SetRectRgn( mDisplayRgn, r.left, r.top, r.right, r.bottom );
  mTargetDC = reinterpret_cast<HDC>( dc.handle );
#else
  mDisplayRect = QRect(
    static_cast<int>( dc.rect.left ),
    static_cast<int>( dc.rect.top ),
    static_cast<int>( dc.rect.right - dc.rect.left ),
    static_cast<int>( dc.rect.bottom - dc.rect.top )
  );
  mDisplayRgn = mDisplayRect;
  mTargetDC = dc.handle;
#endif // __BORLANDC__
  return Invalidate();
}

SignalDisplay&
SignalDisplay::Invalidate()
{
#ifdef __BORLANDC__
  if( mTargetDC != NULL )
    ::InvalidateRect( ::WindowFromDC( mTargetDC ), &mDisplayRect, false );
#else
  QWidget* pWindow = dynamic_cast< QWidget* >( mTargetDC );
  if( pWindow )
    pWindow->update( mDisplayRect );
#endif // __BORLANDC__
  return *this;
}

void
SignalDisplay::AdaptTo( const GenericSignal& inSignal )
{
  // Any changes in the signal size that we must react to?
  bool reconfigure = false;
  if( inSignal.Elements() > mNumSamples )
  {
    OSMutex::Lock lock( mDataLock );
    SetNumSamples( inSignal.Elements() );
    reconfigure = true;
  }
  if( inSignal.Channels() != mData.Channels() )
  {
    OSMutex::Lock lock( mDataLock );
    int newNumDisplayGroups = ( inSignal.Channels() - mMarkerChannels - 1 ) / mChannelGroupSize + 1;
    if( mNumDisplayGroups == 0 )
      mNumDisplayGroups = min<int>( newNumDisplayGroups, cInitialMaxDisplayGroups );
    else if( newNumDisplayGroups < mNumDisplayGroups )
      mNumDisplayGroups = newNumDisplayGroups;
    reconfigure = true;
  }
  if( reconfigure )
  {
    OSMutex::Lock lock( mDataLock );
    mData = GenericSignal( inSignal.Channels(), mNumSamples );
    SetDisplayGroups( DisplayGroups() );
    SyncLabelWidth();
    mSampleCursor = 0;
    Invalidate();
  }
}

SignalDisplay&
SignalDisplay::WrapForward( const GenericSignal& inSignal )
{
  if( mTargetDC == NULL )
    return *this;

  AdaptTo( inSignal );
  OSMutex::Lock lock( mDataLock );
  mShowCursor = ( inSignal.Elements() < mNumSamples );

  for( int i = 0; i < inSignal.Channels(); ++i )
    for( int j = 0; j < inSignal.Elements(); ++j )
      mData( i, ( mSampleCursor + j ) % mData.Elements() ) = inSignal( i, j );

  SyncGraphics();

  int firstInvalidSample = mSampleCursor,
      firstValidSample = mSampleCursor + inSignal.Elements();
  bool wrappingAround = false;
  if( mNumSamples > 0 )
  {
    wrappingAround = ( firstValidSample / mNumSamples > 0 );
    mWrapAround |= wrappingAround;
    wrappingAround |= bool( mSampleCursor == 0 );
    mSampleCursor = firstValidSample % mNumSamples;
  }

#ifdef __BORLANDC__
  long firstInvalidPixel = mDataRect.left,
       firstValidPixel = mDataRect.right;
#else
  long firstInvalidPixel = mDataRect.left(),
       firstValidPixel = mDataRect.right();
#endif // __BORLANDC__

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

#ifdef __BORLANDC__
  // We maintain a redraw region to make sure the
  // cursor is deleted from its old position.
  HWND targetWindow = NULL;
  if( mTargetDC )
    targetWindow = ::WindowFromDC( mTargetDC );
  if( targetWindow != NULL )
  {
    ::InvalidateRgn( targetWindow, mRedrawRgn, false );
    ::SetRectRgn( mRedrawRgn, 0, 0, 0, 0 );

    RECT invalidRect = mDataRect;

    // The non-wrapped area.
    invalidRect.left = max( firstInvalidPixel, mDataRect.left );
    invalidRect.right = min( firstValidPixel, mDataRect.right );
    if( invalidRect.left < invalidRect.right )
      ::InvalidateRect( targetWindow, &invalidRect, false );

    // The area wrapped around the left edge.
    invalidRect.left = max( firstInvalidPixel + mDataWidth, mDataRect.left );
    invalidRect.right = min( firstValidPixel + mDataWidth, mDataRect.right );
    if( invalidRect.left < invalidRect.right )
      ::InvalidateRect( targetWindow, &invalidRect, false );

    // The area wrapped around the right edge.
    invalidRect.left = max( firstInvalidPixel - mDataWidth, mDataRect.left );
    invalidRect.right = min( firstValidPixel - mDataWidth, mDataRect.right );
    if( invalidRect.left < invalidRect.right )
      ::InvalidateRect( targetWindow, &invalidRect, false );
  }
#else // __BORLANDC__
  QWidget* pWindow = dynamic_cast< QWidget* >( mTargetDC );
  if( pWindow )
  { // Our Paint() implementation is fast for rectangular update regions but
    // quite slow if the update region is wrapped around the right edge.
    // Thus, we use repaint() rather than update() in this case.
    if( wrappingAround )
      pWindow->repaint( mCursorRect );
    else
      pWindow->update( mCursorRect );

    QRect invalidRect = mDataRect;

    // The non-wrapped area.
    invalidRect.setLeft( max<int>( firstInvalidPixel, mDataRect.left() ) );
    invalidRect.setRight( min<int>( firstValidPixel, mDataRect.right() ) );
    if( invalidRect.left() < invalidRect.right() )
    {
      if( wrappingAround )
        pWindow->repaint( invalidRect );
      else
        pWindow->update( invalidRect );
    }

    // The area wrapped around the left edge.
    invalidRect.setLeft( max<int>( firstInvalidPixel + mDataWidth, mDataRect.left() ) );
    invalidRect.setRight( min<int>( firstValidPixel + mDataWidth, mDataRect.right() ) );
    if( invalidRect.left() < invalidRect.right() )
      pWindow->repaint( invalidRect );

    // The area wrapped around the right edge.
    invalidRect.setLeft( max<int>( firstInvalidPixel - mDataWidth, mDataRect.left() ) );
    invalidRect.setRight( min<int>( firstValidPixel - mDataWidth, mDataRect.right() ) );
    if( invalidRect.left() < invalidRect.right() )
      pWindow->repaint( invalidRect );
  }
#endif // __BORLANDC__
  return *this;
}

SignalDisplay&
SignalDisplay::ScrollForward( const GenericSignal& inSignal )
{
  if( mTargetDC == NULL )
    return *this;

  if( inSignal.Elements() < 1 )
    return *this;

  AdaptTo( inSignal );
  OSMutex::Lock lock( mDataLock );
  mShowCursor = false;
  // Shift data to the left, and then append the input signal.
  for( int i = 0; i < inSignal.Channels(); ++i )
  {
    int j = 0,
        k = inSignal.Elements();
    while( j < mData.Elements() - inSignal.Elements() )
      mData( i, j++ ) = mData( i, k++ );
    k = 0;
    while( j < mData.Elements() )
      mData( i, j++ ) = inSignal( i, k++ );
  }

  SyncGraphics();
  Invalidate();

  return *this;
}

SignalDisplay&
SignalDisplay::ScrollBack( const GenericSignal& inSignal )
{
  if( mTargetDC == NULL )
    return *this;

  if( inSignal.Elements() < 1 )
    return *this;

  AdaptTo( inSignal );
  OSMutex::Lock lock( mDataLock );
  mShowCursor = false;
  // Shift data to the right, and then prepend the input signal.
  for( int i = 0; i < inSignal.Channels(); ++i )
  {
    int j = mData.Elements(),
        k = mData.Elements() - inSignal.Elements();
    while( k > 0 )
      mData( i, --j ) = mData( i, --k );
    k = inSignal.Elements();
    while( j > 0 )
      mData( i, --j ) = inSignal( i, --k );
  }

  SyncGraphics();
  Invalidate();

  return *this;
}

#ifdef __BORLANDC__
HFONT
SignalDisplay::AxisFont()
{
  const fontHeight = cLabelWidth / 2;
  return ::CreateFont( -fontHeight, 0, 0, 0, FW_DONTCARE,
             false, false, false, ANSI_CHARSET, OUT_RASTER_PRECIS,
             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
             VARIABLE_PITCH | FF_SWISS, NULL );
}
#else // __BORLANDC__
QFont
SignalDisplay::AxisFont()
{
  return QFont( "Helvetica", 9 );
}
#endif // __BORLANDC__

void
SignalDisplay::SyncLabelWidth()
{
  mChannelNameCache.clear();
  if( mTargetDC == NULL )
  {
    mLabelWidth = cLabelWidth;
    mMarkerHeight = mLabelWidth;
  }
  else
  {
#ifdef __BORLANDC__
    HDC dc = mTargetDC;
    HGDIOBJ font = AxisFont();
    ::SelectObject( dc, font );
    SIZE size = { cLabelWidth, 0 };
#else // __BORLANDC__
    QFontMetrics fontMetrics( AxisFont() );
#endif // __BORLANDC__
    ostringstream oss;
    oss << mData.Channels();
#ifdef __BORLANDC__
    if( ::GetTextExtentPoint32( dc, oss.str().c_str(), oss.str().length(), &size ) )
    {
      mLabelWidth = size.cx;
      mMarkerHeight = size.cy;
    }
#else // __BORLANDC__
    QSize size = fontMetrics.size( 0, oss.str().c_str() );
    mLabelWidth = size.width();
    mMarkerHeight = size.height();
#endif // __BORLANDC__
    if( mShowChannelLabels )
    {
      for( size_t i = 0; i < mChannelLabels.size(); ++i )
      {
        int address = mChannelLabels[ i ].Address();
        if( address >= int( mChannelNameCache.size() ) )
          mChannelNameCache.resize( address + 1, "" );
        const string& label = mChannelLabels[ i ].Text();
        mChannelNameCache[ address ] = label;
        if( address < mData.Channels() - mMarkerChannels )
        {
#ifdef __BORLANDC__
          if( ::GetTextExtentPoint32( dc, label.c_str(), label.length(), &size )
              && size.cx > mLabelWidth )
            mLabelWidth = size.cx;
#else // __BORLANDC__
          size = fontMetrics.size( 0, label.c_str() );
          if( size.width() > mLabelWidth )
            mLabelWidth = size.width();
#endif // __BORLANDC__
        }
        else
          mChannelNameCache[ address ] += " ";
      }
    }
    for( int i = 0; i < mData.Channels(); ++i )
    {
      if( int( mChannelNameCache.size() ) <= i )
        mChannelNameCache.resize( i + 1 );
      if( mChannelNameCache[ i ].empty() )
      {
        ostringstream oss;
        oss << i + cChannelBase;
        mChannelNameCache[ i ] = oss.str();
      }
    }
#ifdef __BORLANDC__
    ::DeleteObject( font );
#endif // __BORLANDC__
    mLabelWidth += 3 * cAxisWidth + cTickWidth;
  }
}

inline
void
SignalDisplay::SyncGraphics()
{
#ifdef __BORLANDC__
  mDataRect.left = mLabelWidth;
  mDataRect.top = 0;
  mDataRect.right = mDisplayRect.right - mDisplayRect.left;
  mDataRect.bottom = mDisplayRect.bottom - mDisplayRect.top;
  mDataWidth = std::max<int>( 0, mDataRect.right - mDataRect.left );
  mDataHeight = std::max<int>( 0, mDataRect.bottom - mDataRect.top
                                  - mMarkerHeight * ( mMarkerChannels + 2 )
                                  + cTickLength / 2 );
#else // __BORLANDC__
  mDataRect.setLeft( mLabelWidth );
  mDataRect.setTop( 0 );
  mDataRect.setRight( mDisplayRect.right() - mDisplayRect.left() );
  mDataRect.setBottom( mDisplayRect.bottom() - mDisplayRect.top() );
  mDataWidth = max<int>( 0, mDataRect.right() - mDataRect.left() );
  mDataHeight = max<int>( 0, mDataRect.bottom() - mDataRect.top()
                             - mMarkerHeight * ( mMarkerChannels + 2 )
                             + cTickLength / 2 );
#endif // __BORLANDC__
}

SignalDisplay&
SignalDisplay::SetDisplayGroups( int inDisplayGroups )
{
  mNumDisplayGroups = inDisplayGroups;
  int numSignalChannels = mData.Channels() - mMarkerChannels;
  if( mNumDisplayGroups * mChannelGroupSize > numSignalChannels )
    mNumDisplayGroups = ( numSignalChannels - 1 ) / mChannelGroupSize + 1;
  mNumDisplayChannels = min( numSignalChannels, mNumDisplayGroups * mChannelGroupSize );
  SetTopGroup( mTopGroup );
  Invalidate();
  return *this;
}

SignalDisplay&
SignalDisplay::SetTopGroup( int inTopGroup )
{
  int newTopGroup = inTopGroup,
         maxTopGroup = ChannelToGroup( mData.Channels() - mMarkerChannels ) - int( mNumDisplayGroups );
  if( newTopGroup > maxTopGroup )
    newTopGroup = maxTopGroup;
  if( newTopGroup < 0 )
    newTopGroup = 0;
  if( newTopGroup != mTopGroup )
  {
    mTopGroup = newTopGroup;
    Invalidate();
  }
  return *this;
}

SignalDisplay&
SignalDisplay::SetDisplayMode( int mode )
{
  if( mode != mDisplayMode )
  {
    mDisplayMode = eDisplayMode( mode );
    Invalidate();
  }
  return *this;
}

SignalDisplay&
SignalDisplay::SetNumSamples( int inNumSamples )
{
  int newNumSamples = inNumSamples;
  if( newNumSamples < 0 )
    newNumSamples = 0;
  if( newNumSamples != mNumSamples )
  {
    OSMutex::Lock lock( mDataLock );
    mData = GenericSignal( mData.Channels(), newNumSamples );
    mSampleCursor = 0;
    Invalidate();
    mNumSamples = newNumSamples;
  }
  return *this;
}

SignalDisplay&
SignalDisplay::Paint( void* inUpdateRgn )
{
  if( mTargetDC == NULL )
    return *this;

  OSMutex::Lock lock( mDataLock );
  PaintInfo p;
  SetupPainting( p, inUpdateRgn );

#ifdef __BORLANDC__
  HRGN updateRgn
   = inUpdateRgn ? static_cast<HRGN>( inUpdateRgn ) : mDisplayRgn;
  int width = mDisplayRect.right - mDisplayRect.left,
      height = mDisplayRect.bottom - mDisplayRect.top;
  HBITMAP offscreenBmp = NULL;
  switch( ::GetObjectType( p.dc ) )
  {
    case OBJ_METADC:
    case OBJ_ENHMETADC:
      break;

    default:
      p.dc = ::CreateCompatibleDC( mTargetDC );
      offscreenBmp = ::CreateCompatibleBitmap( mTargetDC, width, height );
      ::DeleteObject( ::SelectObject( p.dc, offscreenBmp ) );
      ::SelectClipRgn( p.dc, updateRgn );
      ::OffsetClipRgn( p.dc, -mDisplayRect.left, -mDisplayRect.top );
  }
  ::SelectClipRgn( mTargetDC, updateRgn );
#endif // __BORLANDC__

  ClearBackground( p );
  DrawXTicks( p );
  DrawAxes( p );
  switch( mDisplayMode )
  {
    case polyline:
      DrawSignalPolyline( p );
      DrawYTicks( p );
      DrawValueUnit( p );
      break;
    case field2d:
      DrawSignalField2d( p );
      break;
    default:
      assert( false );
  }
  DrawMarkerChannels( p );
  DrawMarkers( p );
  DrawChannelLabels( p );
  DrawCursor( p );

#ifdef TEST_UPDATE_RGN
# ifdef __BORLANDC__
  HBRUSH brush = ::CreateSolidBrush( ::rand() & 0x0000ffff );
  ::FillRect( p.dc, &mDisplayRect, brush );
  ::DeleteObject( brush );
# else
  p.painter->fillRect( mDisplayRect, QColor( ::rand() << 8 & 0x00ffff00 ) );
# endif // __BORLANDC__
#endif // TEST_UPDATE_RGN

#ifdef __BORLANDC__
  // Copy data from the offscreen buffer into the target device context (usually a window).
  if( offscreenBmp )
  {
    ::BitBlt( mTargetDC,
              mDisplayRect.left,
              mDisplayRect.top,
              width,
              height,
              p.dc,
              0,
              0,
              SRCCOPY
    );
    ::DeleteObject( p.dc );
    ::DeleteObject( offscreenBmp );
  }
#endif // __BORLANDC__

  CleanupPainting( p );
  return *this;
}

void
SignalDisplay::SetupPainting( PaintInfo& p, void* inUpdateRgn )
{
  SyncGraphics();

#ifdef __BORLANDC__
  p.dc = mTargetDC;
#else
  p.updateRgn = reinterpret_cast<const QRegion*>( inUpdateRgn );
  p.painter = new QPainter( mTargetDC );
  p.painter->setRenderHint( QPainter::Antialiasing, false );
  p.painter->setRenderHint( QPainter::TextAntialiasing, false );
#endif
  p.signalPens.resize( mData.Channels() );
  p.signalBrushes.resize( mData.Channels() );

  // Background properties
#ifdef __BORLANDC__
  p.backgroundColor = RGBColor( mInverted ? RGBColor::White : RGBColor::Black ).ToWinColor();
  p.backgroundBrush = ::CreateSolidBrush( p.backgroundColor );
#else
  RGBColor c = mInverted ? RGBColor::White : RGBColor::Black;
  p.backgroundColor = QColor( c.R(), c.G(), c.B() );
  p.backgroundBrush = QBrush( p.backgroundColor );
#endif // __BORLANDC__

  // Cursor properties
  p.cursorWidth = 3;
#ifdef __BORLANDC__
  p.cursorColor = RGBColor( RGBColor::Yellow ).ToWinColor();
  p.cursorBrush = ::CreateSolidBrush( p.cursorColor );
#else
  c = RGBColor::Yellow;
  p.cursorColor = QColor( c.R(), c.G(), c.B() );
  p.cursorBrush = QBrush( p.cursorColor );
#endif // __BORLANDC__

  // Axis properties
  RGBColor axisColor = mAxisColor;
  if( mInverted && ( axisColor == RGBColor::White ) )
    axisColor = RGBColor::Black;
#ifdef __BORLANDC__
  COLORREF winAxisColor = axisColor.ToWinColor();
  p.axisBrush = ::CreateSolidBrush( winAxisColor );
#else
  QColor qAxisColor( axisColor.R(), axisColor.G(), axisColor.B() );
  p.axisBrush = QBrush( qAxisColor );
#endif // __BORLANDC__
  p.axisY = GroupBottom( mNumDisplayGroups - 1 );

  p.markerWidth = 1;
#ifdef __BORLANDC__
  p.baselinePen = ::CreatePen( PS_SOLID, 0, winAxisColor );
  p.markerColor = RGBColor( mInverted ? RGBColor::Black : RGBColor::White ).ToWinColor();
  p.markerBrush = ::CreateSolidBrush( p.markerColor );

  p.labelColor = winAxisColor;
#else
  p.baselinePen = QPen( qAxisColor );
  c = RGBColor( mInverted ? RGBColor::Black : RGBColor::White );
  p.markerColor = QColor( c.R(), c.G(), c.B() );
  p.markerBrush = QBrush( p.markerColor );
  p.labelColor = qAxisColor;
#endif // __BORLANDC__
  p.labelFont = AxisFont();


  // Signal properties
  if( mDisplayColors )
    for( int i = 0; i < mData.Channels(); ++i )
    {
      RGBColor channelColor = ChannelColor( i );
      if( mInverted && channelColor == RGBColor::White )
        channelColor = RGBColor::Black;
#ifdef __BORLANDC__
      p.signalPens[ i ] = ::CreatePen( PS_SOLID, 0, channelColor.ToWinColor() );
      p.signalBrushes[ i ] = ::CreateSolidBrush( channelColor.ToWinColor() );
#else
      QColor c( channelColor.R(), channelColor.G(), channelColor.B() );
      p.signalPens[ i ] = QPen( c );
      p.signalBrushes[ i ] = QBrush( c );
#endif // __BORLANDC__
    }
  else
  {
    RGBColor channelColor = mInverted? RGBColor::Black : RGBColor::White;
#ifdef __BORLANDC__
    HPEN pen = ::CreatePen( PS_SOLID, 0, RGBColor( channelColor ).ToWinColor() );
#else
    QColor c( channelColor.R(), channelColor.G(), channelColor.B() );
    QPen pen( c );
#endif // __BORLANDC__
    for( int i = 0; i < mData.Channels(); ++i )
      p.signalPens[ i ] = pen;
  }
}

void
SignalDisplay::ClearBackground( const PaintInfo& p )
{
#ifdef __BORLANDC__
  int width = mDisplayRect.right - mDisplayRect.left,
      height = mDisplayRect.bottom - mDisplayRect.top;
  RECT all = { 0, 0, width, height };
  ::FillRect( p.dc, &all, p.backgroundBrush );
#else
  p.painter->fillRect( mDisplayRect, p.backgroundColor );
#endif // __BORLANDC__
}

void
SignalDisplay::DrawSignalPolyline( const PaintInfo& p )
{
  float baseInterval = mNumDisplayGroups > 0
                       ? mDataHeight / mNumDisplayGroups
                       : mDataHeight;
  // Draw the baselines.
  if( mShowBaselines )
  {
#ifdef __BORLANDC__
    POINT baselinePoints[ 2 ];
    ::SelectObject( p.dc, p.baselinePen );
    baselinePoints[ 0 ].x = SampleLeft( 0 );
    baselinePoints[ 1 ].x = SampleRight( mNumSamples );
    for( int i = 0; i < mNumDisplayGroups; ++i )
    {
      baselinePoints[ 0 ].y = ChannelBottom( i ) + ( baseInterval * mMinValue ) / ( mMaxValue - mMinValue );
      baselinePoints[ 1 ].y = baselinePoints[ 0 ].y;
      ::Polyline( p.dc, baselinePoints, 2 );
    }
#else
    QPoint baselinePoints[ 2 ];
    p.painter->setPen( p.baselinePen );
    baselinePoints[ 0 ].setX( SampleLeft( 0 ) );
    baselinePoints[ 1 ].setX( SampleRight( mNumSamples ) );
    for( int i = 0; i < mNumDisplayGroups; ++i )
    {
      baselinePoints[ 0 ].setY(
        static_cast<int>( ChannelBottom( i ) + ( baseInterval * mMinValue ) / ( mMaxValue - mMinValue ) )
      );
      baselinePoints[ 1 ].setY(
        static_cast<int>( baselinePoints[ 0 ].y() )
      );
      p.painter->drawPolyline( baselinePoints, 2 );
    }
#endif // __BORLANDC__
  }

#ifdef __BORLANDC__
  delete[] mpSignalPoints;
  mpSignalPoints = new POINT[ mNumSamples ];
  for( int j = 0; j < mNumSamples; ++j )
    mpSignalPoints[ j ].x = SampleLeft( j );
  for( int i = 0; i < mNumDisplayChannels; ++i )
  {
    int channelBottom = ChannelBottom( i ),
        channel = i + mTopGroup * mChannelGroupSize;
    for( int j = 0; j < mNumSamples; ++j )
      mpSignalPoints[ j ].y = channelBottom - 1
           - baseInterval * NormData( channel, j );

    ::SelectObject( p.dc, p.signalPens[ channel % p.signalPens.size() ] );
    ::Polyline( p.dc, mpSignalPoints, mSampleCursor );
    ::Polyline( p.dc, &mpSignalPoints[ mSampleCursor ], mNumSamples - mSampleCursor );

    // We actually need this strange distinction of cases.
    if( mShowCursor && mSampleCursor != 0 && mNumSamples > 1 )
    {
      POINT remainingPoints[ 2 ];
      remainingPoints[ 0 ] = mpSignalPoints[ mNumSamples - 1 ];
      remainingPoints[ 1 ].x = SampleLeft( mNumSamples );
      if( mWrapAround )
        remainingPoints[ 1 ].y = mpSignalPoints[ 0 ].y;
      else
        remainingPoints[ 1 ].y = mpSignalPoints[ mNumSamples - 1 ].y;
      ::Polyline( p.dc, remainingPoints, 2 );
    }
    ::Sleep( 0 );
  }
#else // __BORLANDC__
  int sampleBegin = 0,
      sampleEnd = mNumSamples;
  if( p.updateRgn )
  { // We restrict drawing to the actually requested update region.
    QRect clipRect = p.updateRgn->boundingRect();
    sampleBegin = PosToSample( clipRect.left() );
    sampleBegin = max( sampleBegin - 1, 0 );
    sampleEnd = PosToSample( clipRect.right() + 1 );
    sampleEnd = min( sampleEnd + 1, mNumSamples );
  }
  delete[] mpSignalPoints;
  mpSignalPoints = new QPoint[ mNumSamples ];
  int numPens = p.signalPens.size();

  for( int j = sampleBegin; j < sampleEnd; ++j )
    mpSignalPoints[ j ].setX( SampleLeft( j ) );
  for( int i = 0; i < mNumDisplayChannels; ++i )
  {
    int channelBottom = ChannelBottom( i ),
        channel = i + mTopGroup * mChannelGroupSize;
    for( int j = sampleBegin; j < sampleEnd; ++j )
      mpSignalPoints[ j ].setY(
        static_cast<int>( channelBottom - 1 - baseInterval * NormData( channel, j ) )
      );
    p.painter->setPen( p.signalPens[ channel % numPens ] );
    if( sampleBegin <= mSampleCursor && mSampleCursor < sampleEnd )
    {
      p.painter->drawPolyline( mpSignalPoints + sampleBegin, mSampleCursor - sampleBegin );
      p.painter->drawPolyline( mpSignalPoints + mSampleCursor, sampleEnd - mSampleCursor );
    }
    else
    {
      p.painter->drawPolyline( mpSignalPoints + sampleBegin, sampleEnd - sampleBegin );
    }

#if 0
    // We actually need this strange distinction of cases.
    if( mShowCursor && mSampleCursor != 0 && mNumSamples > 1 )
    {
      QPoint remainingPoints[ 2 ];
      remainingPoints[ 0 ] = mpSignalPoints[ mNumSamples - 1 ];
      remainingPoints[ 1 ].setX( SampleLeft( mNumSamples ) );
      if( mWrapAround )
        remainingPoints[ 1 ].setY( mpSignalPoints[ 0 ].y() );
      else
        remainingPoints[ 1 ].setY( mpSignalPoints[ mNumSamples - 1 ].y() );
      p.painter->drawPolyline( remainingPoints, 2 );
    }
#endif
# ifdef _WIN32
    ::Sleep( 0 );
# endif // _WIN32
  }
#endif // __BORLANDC__
}

void
SignalDisplay::DrawSignalField2d( const PaintInfo& p )
{
#ifdef __BORLANDC__
  for( int i = 0; i < mNumDisplayChannels; ++i )
  {
    for( int j = 0; j < mNumSamples; ++j )
    {
      RECT dotRect =
      {
        SampleLeft( j ),
        ChannelTop( i ),
        SampleRight( j ),
        ChannelBottom( i )
      };
      if( ::RectVisible( p.dc, &dotRect ) )
      {
        float dataValue = NormData( i + mTopGroup * mChannelGroupSize, j );
        if( dataValue < 0.0 )
          dataValue = 0.0;
        else if( dataValue > 1.0 )
          dataValue = 1.0;

        LONG color;
        if( mDisplayColors )
          color = RGBColor::FromHSV( dataValue - 1.0 / 3.0, 1.0, dataValue ).ToWinColor();
        else
          color = RGBColor::FromHSV( 0.0, 0.0, dataValue ).ToWinColor();
        // SetBkColor/ExtTextOut is faster than CreateSolidBrush/FillRect/DeleteObject.
        ::SetBkColor( p.dc, color );
        ::ExtTextOut( p.dc, dotRect.left, dotRect.top, ETO_OPAQUE, &dotRect, "", 0, NULL );
      }
    }
    ::Sleep( 0 );
  }
#else
  int sampleBegin = 0,
      sampleEnd = mNumSamples;
  if( p.updateRgn )
  { // We restrict drawing to the actually requested update region.
    QRect clipRect = p.updateRgn->boundingRect();
    sampleBegin = PosToSample( clipRect.left() );
    sampleBegin = max( sampleBegin - 1, 0 );
    sampleEnd = PosToSample( clipRect.right() + 1 );
    sampleEnd = min( sampleEnd + 1, mNumSamples );
  }
  for( int i = 0; i < mNumDisplayChannels; ++i )
  {
    for( int j = sampleBegin; j < sampleEnd; ++j )
    {
      QRect dotRect(
          SampleLeft( j ),
          ChannelTop( i ),
          SampleRight( j ) - SampleLeft( j ),
          ChannelBottom( i ) - ChannelTop( i )
      );
      float dataValue = NormData( i + mTopGroup * mChannelGroupSize, j );
      if( dataValue < 0.0 )
        dataValue = 0.0;
      else if( dataValue > 1.0 )
        dataValue = 1.0;

      RGBColor rgb;
      if( mDisplayColors )
        rgb = RGBColor::FromHSV( dataValue - 1.0 / 3.0, 1.0, dataValue );
      else
        rgb = RGBColor::FromHSV( 0.0, 0.0, dataValue );
      p.painter->fillRect( dotRect, QColor( rgb.R(), rgb.G(), rgb.B() ) );
    }
# ifdef _WIN32
    ::Sleep( 0 );
# endif // _WIN32
  }
#endif // __BORLANDC__
}

void
SignalDisplay::DrawMarkerChannels( const PaintInfo& p )
{
#ifdef __BORLANDC__
  for( int markerCh = 0; markerCh < mMarkerChannels; ++markerCh )
  {
    ::SetTextColor( p.dc, p.labelColor );
    ::SelectObject( p.dc, p.labelFont );
    ::SetBkMode( p.dc, TRANSPARENT );
    int baseline = MarkerChannelBottom( markerCh );
    size_t channelNumber = mData.Channels() - mMarkerChannels + markerCh;
    if( mData.Elements() > 0 )
    {
      int prevVal = mData( channelNumber, 0 );
      for( int sample = 0; sample < mData.Elements(); ++sample )
      {
        int curVal = mData( channelNumber, sample );
        if( curVal != prevVal )
        {
          RECT posRect =
          {
            SampleLeft( sample ) - p.markerWidth,
            baseline,
            SampleLeft( sample ),
            baseline
          };
          ostringstream oss;
          oss << curVal;
          ::DrawText( p.dc,
              oss.str().c_str(),
              -1, &posRect, DT_CENTER | DT_SINGLELINE | DT_BOTTOM | DT_NOCLIP );
          RECT markerRect =
          {
            SampleLeft( sample ) - p.markerWidth,
            GroupTop( 0 ),
            SampleLeft( sample ),
            p.axisY
          };
          ::FillRect( p.dc, &markerRect, p.axisBrush );
          prevVal = curVal;
        }
      }
    }
    ::SetBkColor( p.dc, p.backgroundColor );
    ::SetBkMode( p.dc, OPAQUE );
    RECT labelRect =
    {
      cTickWidth,
      baseline,
      cTickWidth,
      baseline
    };
    const char* labelText = "";
    if( mChannelNameCache.size() > channelNumber )
      labelText = mChannelNameCache[ channelNumber ].c_str();
    ::DrawText( p.dc, labelText,
       -1, &labelRect, DT_LEFT | DT_SINGLELINE | DT_BOTTOM | DT_NOCLIP );
  }
#else
  for( int markerCh = 0; markerCh < mMarkerChannels; ++markerCh )
  {
    p.painter->setPen( p.labelColor );
    p.painter->setFont( p.labelFont );
    p.painter->setBackgroundMode( Qt::TransparentMode );
    int baseline = MarkerChannelBottom( markerCh );
    size_t channelNumber = mData.Channels() - mMarkerChannels + markerCh;
    if( mData.Elements() > 0 )
    {
      int prevVal = static_cast<int>( mData( channelNumber, 0 ) );
      for( int sample = 0; sample < mData.Elements(); ++sample )
      {
        int curVal = static_cast<int>( mData( channelNumber, sample ) );
        if( curVal != prevVal )
        {
          QRect posRect;
          posRect.setLeft( SampleLeft( sample ) - p.markerWidth );
          posRect.setTop( baseline );
          posRect.setRight( SampleLeft( sample ) );
          posRect.setBottom( baseline );
          ostringstream oss;
          oss << curVal;
          p.painter->drawText( posRect,
            Qt::AlignHCenter | Qt::TextSingleLine | Qt::AlignBottom | Qt::TextDontClip,
            oss.str().c_str() );
          QRect markerRect;
          markerRect.setLeft( SampleLeft( sample ) - p.markerWidth );
          markerRect.setTop( GroupTop( 0 ) );
          markerRect.setRight( SampleLeft( sample ) );
          markerRect.setBottom( p.axisY );
          p.painter->fillRect( markerRect, p.axisBrush );
          prevVal = curVal;
        }
      }
    }
    p.painter->setBackground( p.backgroundBrush );
    p.painter->setBackgroundMode( Qt::OpaqueMode );
    QRect labelRect( cTickWidth, baseline, 0, 0 );
    const char* labelText = "";
    if( mChannelNameCache.size() > channelNumber )
      labelText = mChannelNameCache[ channelNumber ].c_str();
    p.painter->drawText( labelRect,
      Qt::AlignLeft | Qt::TextSingleLine | Qt::AlignBottom | Qt::TextDontClip,
      labelText );
  }
#endif
}

void
SignalDisplay::DrawCursor( const PaintInfo& p )
{
#ifdef __BORLANDC__
  if( mShowCursor )
  {
    size_t cursorSample = mSampleCursor;
    if( cursorSample == 0 )
      cursorSample = mNumSamples;

    RECT cursorRect =
    {
      SampleLeft( cursorSample ) - p.cursorWidth,
      0,
      SampleLeft( cursorSample ),
      GroupBottom( mNumDisplayGroups - 1 )
    };
    ::FillRect( p.dc, &cursorRect, p.cursorBrush );
    // Remember the cursor rectangle for redrawing when the next
    // data packet arrives.
    HRGN rectRgn = ::CreateRectRgnIndirect( &cursorRect );
    ::CombineRgn( mRedrawRgn, mRedrawRgn, rectRgn, RGN_OR );
    ::DeleteObject( rectRgn );
  }
#else // __BORLANDC__
  if( mShowCursor )
  {
    size_t cursorSample = mSampleCursor;
#if 0
    if( cursorSample == 0 )
      cursorSample = mNumSamples;
#endif

    QRect cursorRect(
      SampleLeft( cursorSample ) - p.cursorWidth,
      0,
      p.cursorWidth,
      GroupBottom( mNumDisplayGroups - 1 )
    );
    p.painter->fillRect( cursorRect, p.cursorBrush );
    // Remember the cursor rectangle for redrawing when the next
    // data packet arrives.
    mCursorRect = cursorRect;
  }
#endif // __BORLANDC__
}

void
SignalDisplay::DrawXTicks( const PaintInfo& p )
{
#ifdef __BORLANDC__
  ::SetTextColor( p.dc, p.labelColor );
  ::SelectObject( p.dc, p.labelFont );
  ::SetBkMode( p.dc, TRANSPARENT );
#else
  p.painter->setPen( p.labelColor );
  p.painter->setFont( p.labelFont );
  p.painter->setBackgroundMode( Qt::TransparentMode );
#endif // __BORLANDC__
  float xStart = 0,
        xDivision = 1;
  if( mNumSamples > 0 )
  { // Are samples spaced too dense to allow for individual labels?
    float pixelsPerSample = ( SampleLeft( mNumSamples - 1 ) - SampleLeft( 0 ) ) / mNumSamples;
    if( pixelsPerSample < 10 )
    { // Samples are dense
      float displayLength = ::fabs( mNumSamples * mUnitsPerSample ),
            scale = ::pow( 10.0, ::floor( ::log10( displayLength ) + 0.5 ) );
      xDivision = scale / mUnitsPerSample / 5,
      xStart = xDivision - ::fmod( mSampleOffset, xDivision );
    }
  }
  if( xDivision < 1 )
    xDivision = 1;

#ifdef __BORLANDC__
  int nextLabelPos = mDataRect.left;
#else
  int nextLabelPos = mDataRect.left();
#endif // __BORLANDC__
  for( float j = xStart; j < float( mNumSamples ); j += xDivision )
  {
    int tickX = 0;
    switch( mDisplayMode )
    {
      case field2d:
        tickX = ( SampleRight( static_cast<int>( j ) ) + SampleLeft( static_cast<int>( j ) ) ) / 2;
        break;
      case polyline:
      default:
        tickX = SampleLeft( static_cast<int>( j ) );
    }
#ifdef __BORLANDC__
    RECT tickRect =
    {
      tickX - cTickWidth / 2,
      p.axisY + cAxisWidth,
      tickX + cTickWidth / 2,
      p.axisY + cAxisWidth + cTickLength
    };
    ::FillRect( p.dc, &tickRect, p.axisBrush );
#else
    QRect tickRect(
        tickX - cTickWidth / 2,
        p.axisY + cAxisWidth,
        cTickWidth,
        cTickLength
    );
    p.painter->fillRect( tickRect, p.axisBrush );
#endif // __BORLANDC__
    if( tickX > nextLabelPos )
    {
#ifdef __BORLANDC__
      tickRect.top += 2 * cAxisWidth;
#else
      tickRect.setTop( tickRect.top() + 2 * cAxisWidth );
#endif // __BORLANDC__
      ostringstream label;
      float val  = ( j + mSampleOffset ) * mUnitsPerSample;
      if( mTimeLabels )
      {
        label << ' ' << setfill( '0' )
              << setw( 2 ) << int( val ) / 60 << ':'
              << setw( 2 ) << setprecision( 2 ) << int( val ) % 60;
        if( ::fmod( xDivision * mUnitsPerSample, 1.0f ) != 0.0 )
          label << '.' << setw( 2 ) << int( 100 * val + 0.5 ) % 100;
        label << ' ';
      }
      else
        label << setprecision( 6 ) << ' ' << val << mSampleUnit << ' ';

#ifdef __BORLANDC__
      ::DrawText( p.dc, label.str().c_str(), -1, &tickRect,
         DT_TOP | DT_SINGLELINE | DT_CENTER | DT_NOCLIP );
      SIZE textSize;
      ::GetTextExtentPoint32( p.dc, label.str().c_str(), label.str().length(), &textSize );
      nextLabelPos = tickX + textSize.cx;
#else
      p.painter->drawText( tickRect,
        Qt::AlignHCenter | Qt::TextSingleLine | Qt::AlignTop | Qt::TextDontClip,
        label.str().c_str() );
      nextLabelPos = tickX + p.painter->fontMetrics().width( label.str().c_str() );
#endif // __BORLANDC__
    }
  }
}

void
SignalDisplay::DrawYTicks( const PaintInfo& p )
{
#ifdef __BORLANDC__
  ::SetTextColor( p.dc, p.labelColor );
  ::SelectObject( p.dc, p.labelFont );
  ::SetBkMode( p.dc, TRANSPARENT );
  int nextLabelPos = mDataRect.top;
#else
  p.painter->setPen( p.labelColor );
  p.painter->setFont( p.labelFont );
  p.painter->setBackgroundMode( Qt::TransparentMode );
  int nextLabelPos = mDataRect.top();
#endif // __BORLANDC__
  for( int i = 0; i < mNumDisplayGroups; ++i )
  {
    size_t channelNumber = ( mTopGroup + i ) * mChannelGroupSize;
    int    tickY = ( GroupTop( i ) + GroupBottom( i ) ) / 2;
#ifdef __BORLANDC__
    RECT tickRect =
    {
      mLabelWidth - cAxisWidth - cTickLength,
      tickY - cTickWidth / 2,
      mLabelWidth - cAxisWidth,
      tickY + cTickWidth / 2
    };
#else
    QRect tickRect(
        mLabelWidth - cAxisWidth - cTickLength,
        tickY - cTickWidth / 2,
        cTickLength,
        cTickWidth
    );
#endif
    if( mDisplayColors && mChannelGroupSize == 1 )
    {
#ifdef __BORLANDC__
      tickRect.top -= 1;
      tickRect.bottom += 1;
      ::FillRect( p.dc, &tickRect, p.signalBrushes[ channelNumber ] );
#else
      tickRect.setTop( tickRect.top() - 1 );
      tickRect.setBottom( tickRect.bottom() + 1 );
      p.painter->fillRect( tickRect, p.signalBrushes[ channelNumber ] );
#endif // __BORLANDC__
    }
    else
    {
#ifdef __BORLANDC__
      ::FillRect( p.dc, &tickRect, p.axisBrush );
#else
      p.painter->fillRect( tickRect, p.axisBrush );
#endif // __BORLANDC__
    }
    if( tickY >= nextLabelPos )
    {
      const char* labelText = "";
      if( mChannelGroupSize == 1 && mChannelNameCache.size() > channelNumber )
        labelText = mChannelNameCache[ channelNumber ].c_str();
#ifdef __BORLANDC__
      tickRect.right -= 2 * cAxisWidth;
      nextLabelPos = tickY + ::DrawText( p.dc, labelText,
         -1, &tickRect, DT_RIGHT | DT_SINGLELINE | DT_VCENTER | DT_NOCLIP );
#else
      tickRect.setRight( tickRect.right() - 2 * cAxisWidth );
      if( mChannelGroupSize == 1 )
        p.painter->drawText( tickRect,
          Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignRight | Qt::TextDontClip,
          labelText );
      nextLabelPos = tickY + p.painter->fontMetrics().height();
#endif // __BORLANDC__
    }
  }
}

void
SignalDisplay::DrawAxes( const PaintInfo& p )
{
#ifdef __BORLANDC__
  RECT xAxis =
  {
    0,
    p.axisY,
    mDisplayRect.right,
    p.axisY + cAxisWidth
  };
  ::FillRect( p.dc, &xAxis, p.axisBrush );
  RECT yAxis =
  {
    mLabelWidth - cAxisWidth,
    mMarkerChannels > 0 ? GroupTop( 0 ) : 0,
    mLabelWidth,
    mDisplayRect.bottom
  };
  ::FillRect( p.dc, &yAxis, p.axisBrush );
  if( mMarkerChannels > 0 )
  {
    RECT divider =
    {
      0,
      GroupTop( 0 ) - cAxisWidth,
      mDisplayRect.right,
      GroupTop( 0 )
    };
    ::FillRect( p.dc, &divider, p.axisBrush );
  }
#else
  QRect xAxis(
      0,
      p.axisY,
      mDisplayRect.right(),
      cAxisWidth
  );
  p.painter->fillRect( xAxis, p.axisBrush );
  QRect yAxis;
  yAxis.setLeft( mLabelWidth - cAxisWidth );
  yAxis.setTop( mMarkerChannels > 0 ? GroupTop( 0 ) : 0 );
  yAxis.setWidth( cAxisWidth );
  yAxis.setBottom( mDisplayRect.bottom() );
  p.painter->fillRect( yAxis, p.axisBrush );
  if( mMarkerChannels > 0 )
  {
    QRect divider;
    divider.setLeft( 0 );
    divider.setRight( mDisplayRect.right() );
    divider.setTop( GroupTop( 0 ) - cAxisWidth );
    divider.setBottom( GroupTop( 0 ) );
    p.painter->fillRect( divider, p.axisBrush );
  }
#endif // __BORLANDC__
}

void
SignalDisplay::DrawMarkers( const PaintInfo& p )
{
  // Draw markers.
  for( size_t i = 0; i < mXAxisMarkers.size(); ++i )
  {
    int markerX = SampleRight( static_cast<int>( mXAxisMarkers[ i ].Address() - mSampleOffset ) );
#ifdef __BORLANDC__
    RECT markerBar =
    {
      markerX - cAxisWidth / 2,
      p.axisY - 4 * cAxisWidth,
      markerX + cAxisWidth / 2,
      p.axisY - 1
    };
    ::FillRect( p.dc, &markerBar, p.markerBrush );
#else
    QRect markerBar(
        markerX - cAxisWidth / 2,
        p.axisY - 4 * cAxisWidth,
        cAxisWidth,
        4 * cAxisWidth - 1
    );
    p.painter->fillRect( markerBar, p.markerBrush );
#endif // __BORLANDC__
  }
}

void
SignalDisplay::DrawChannelLabels( const PaintInfo& p )
{
  // Draw channel labels if channels don't coincide with groups.
  if( mShowChannelLabels && mChannelGroupSize > 1 )
  {
#ifdef __BORLANDC__
    ::SetBkColor( p.dc, p.backgroundColor );
    ::SetBkMode( p.dc, OPAQUE );
    RECT legendRect =
    {
      0, 0, 0, 0
    };
#else
    p.painter->setBackground( p.backgroundColor );
    p.painter->setBackgroundMode( Qt::OpaqueMode );
    QRect legendRect;
#endif // __BORLANDC__
    for( size_t i = 0; i < mChannelLabels.size(); ++i )
    {
      RGBColor textColor = ChannelColor( mChannelLabels[ i ].Address() );
      if( mInverted && textColor == RGBColor::White )
        textColor = RGBColor::Black;
#ifdef __BORLANDC__
      ::SetTextColor( p.dc, textColor.ToWinColor() );
      legendRect.top += ::DrawText( p.dc, mChannelLabels[ i ].Text().c_str(),
        mChannelLabels[ i ].Text().length(), &legendRect,
        DT_SINGLELINE | DT_LEFT | DT_NOCLIP | DT_EXTERNALLEADING );
#else
      p.painter->setPen( QColor( textColor.R(), textColor.G(), textColor.B() ) );
      p.painter->drawText( legendRect,
        Qt::TextSingleLine | Qt::AlignLeft | Qt::TextDontClip,
        mChannelLabels[ i ].Text().c_str() );
      legendRect.setTop( legendRect.top() + p.painter->fontMetrics().height() );
#endif
    }
  }
}

void
SignalDisplay::DrawValueUnit( const PaintInfo& p )
{
  float baseInterval = mDataHeight;
  if( mNumDisplayGroups > 0 )
    baseInterval /= mNumDisplayGroups;

  if( mShowValueUnit && mNumDisplayGroups > 0 && baseInterval > 0 )
  {
#ifdef __BORLANDC__
    ::SelectObject( p.dc, p.labelFont );
    ::SetTextColor( p.dc, p.backgroundColor );
    ::SetBkColor( p.dc, p.markerColor );
    ::SetBkMode( p.dc, OPAQUE );
#else
    p.painter->setFont( p.labelFont );
    p.painter->setPen( p.backgroundColor );
    p.painter->setBackground( p.markerColor );
    p.painter->setBackgroundMode( Qt::OpaqueMode );
#endif // __BORLANDC__
    // Find a round value that is near the display range.
    float unitsPerPixel = ::fabs( ( mMaxValue - mMinValue ) * mUnitsPerValue / baseInterval ),
          scale = ::pow( 10.0, ::ceil( ::log10( unitsPerPixel * 0.95 * baseInterval ) ) ),
          rulerLength = scale;
    while( rulerLength / unitsPerPixel >= 0.95 * baseInterval
           && rulerLength / unitsPerPixel > mMarkerHeight )
      rulerLength -= scale / 10;
    int pixelLength = static_cast<int>( rulerLength / unitsPerPixel );

    ostringstream label;
    label << rulerLength << mValueUnit;
    if( mMinValue == 0 )
    {
      int left = SampleLeft( 0 ) + cTickLength,
          top = ChannelBottom( 0 ) - pixelLength;
#ifdef __BORLANDC__
      RECT labelRect =
      {
        left,
        top,
        left,
        top
      };
      ::DrawText( p.dc, label.str().c_str(), -1, &labelRect,
          DT_TOP | DT_SINGLELINE | DT_LEFT | DT_NOCLIP );
      ::DrawText( p.dc, label.str().c_str(), -1, &labelRect,
          DT_TOP | DT_SINGLELINE | DT_LEFT | DT_NOCLIP | DT_CALCRECT );
      RECT lineRect =
      {
        SampleLeft( 0 ),
        labelRect.top,
        left,
        labelRect.top + 1
      };
      ::FillRect( p.dc, &lineRect, p.markerBrush );
#else
      QRect labelRect( left, top, 0, 0 );
      p.painter->drawText( labelRect,
        Qt::AlignTop | Qt::TextSingleLine | Qt::AlignLeft | Qt::TextDontClip,
        label.str().c_str(), &labelRect );
      QRect lineRect;
      lineRect.setLeft( SampleLeft( 0 ) );
      lineRect.setTop( labelRect.top() );
      lineRect.setRight( left );
      lineRect.setHeight( 1 );
      p.painter->fillRect( lineRect, p.markerBrush );
#endif // __BORLANDC__
    }
    else
    {
      int left = SampleLeft( 0 ),
          center = ( GroupBottom( mNumDisplayGroups - 1 ) + GroupTop( mNumDisplayGroups - 1 ) ) / 2;
#ifdef __BORLANDC__
      RECT labelRect =
      {
        left,
        center,
        left,
        center
      };
      ::DrawText( p.dc, label.str().c_str(), -1, &labelRect,
          DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP | DT_CALCRECT );
      RECT lineRect =
      {
        labelRect.left,
        center - ( pixelLength + p.markerWidth ) / 2,
        labelRect.right,
        center - ( pixelLength - p.markerWidth ) / 2
      };
      ::FillRect( p.dc, &lineRect, p.markerBrush );
      lineRect.top += pixelLength;
      lineRect.bottom += pixelLength;
      ::FillRect( p.dc, &lineRect, p.markerBrush );

      labelRect.top = center;
      labelRect.bottom = labelRect.top;
      ::DrawText( p.dc, label.str().c_str(), -1, &labelRect,
         DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP );
#else
      QRect labelRect( left, center, 0, 0 );
      p.painter->drawText( labelRect,
        Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignLeft,
        label.str().c_str(), &labelRect );
      QRect lineRect;
      lineRect.setLeft( labelRect.left() );
      lineRect.setTop( center - ( pixelLength + p.markerWidth ) / 2 + 1 );
      lineRect.setRight( labelRect.right() );
      lineRect.setBottom( center - ( pixelLength - p.markerWidth ) / 2 );
      p.painter->fillRect( lineRect, p.markerBrush );
      lineRect.setTop( lineRect.top() + pixelLength );
      lineRect.setBottom( lineRect.bottom() + pixelLength );
      p.painter->fillRect( lineRect, p.markerBrush );

      labelRect.setTop( center );
      labelRect.setBottom( labelRect.top() );
      p.painter->drawText( labelRect,
        Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignLeft | Qt::TextDontClip,
        label.str().c_str() );
#endif // __BORLANDC__
    }
  }
}

void
SignalDisplay::CleanupPainting( PaintInfo& p )
{
#ifdef __BORLANDC__
  ::DeleteObject( p.backgroundBrush );
  ::DeleteObject( p.cursorBrush );
  ::DeleteObject( p.axisBrush );
  ::DeleteObject( p.markerBrush );
  ::DeleteObject( p.labelFont );
  ::DeleteObject( p.baselinePen );
  for( size_t i = 0; i < p.signalPens.size(); ++i )
    ::DeleteObject( p.signalPens[ i ] );
  for( size_t i = 0; i < p.signalBrushes.size(); ++i )
    ::DeleteObject( p.signalBrushes[ i ] );
#else
  delete p.painter;
#endif // __BORLANDC__
}

