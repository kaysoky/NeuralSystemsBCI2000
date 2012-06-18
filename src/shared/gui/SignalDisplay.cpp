////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A SignalDisplay class that renders GenericSignal data into a
//   given DisplayContext.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
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
#include "BCIAssert.h"
#include "BCIException.h"

#include <cmath>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include <QWidget>
#include <QPainter>

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
  mShowNumericValues( false ),
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
  mNumericValueWidth( 0 ),
  mMinValue( cMinValueDefault ),
  mMaxValue( cMaxValueDefault ),
  mSampleOffset( 0 ),
  mUnitsPerSample( 1 ),
  mUnitsPerValue( 1 ),
  mUnitsPerChannel( 1 ),
  mAxisColor( cAxisColorDefault ),
  mChannelColors( cChannelColorsDefault ),
  mTargetDC( NULL ),
  mpSignalPoints( NULL )
{
}

SignalDisplay::~SignalDisplay()
{
  delete[] mpSignalPoints;
}

SignalDisplay&
SignalDisplay::SetContext( const GUI::DrawContext& dc )
{
  mDisplayRect = QRect(
    static_cast<int>( dc.rect.left ),
    static_cast<int>( dc.rect.top ),
    static_cast<int>( dc.rect.right - dc.rect.left ),
    static_cast<int>( dc.rect.bottom - dc.rect.top )
  );
  mDisplayRgn = mDisplayRect;
  mTargetDC = dc.handle.device;
  return Invalidate();
}

SignalDisplay&
SignalDisplay::Invalidate()
{
  QWidget* pWindow = dynamic_cast< QWidget* >( mTargetDC );
  if( pWindow )
    pWindow->update( mDisplayRect );
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

  long firstInvalidPixel = mDataRect.left(),
       firstValidPixel = mDataRect.right();

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
      bciassert( false );
  }

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
    
    if( mShowNumericValues )
    {
      invalidRect.setLeft( mDataRect.right() - mNumericValueWidth );
      invalidRect.setRight( mDataRect.right() );
      if( invalidRect.left() < invalidRect.right() )
        pWindow->repaint( invalidRect );
    }
  }
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

QFont
SignalDisplay::AxisFont()
{
  return QFont( "Helvetica", 9 );
}
QFont
SignalDisplay::MonoFont()
{
  return QFont( "Courier", 9 );
}

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
    QFontMetrics fontMetrics( AxisFont() );
    ostringstream oss;
    oss << mData.Channels();
    QSize size = fontMetrics.size( 0, oss.str().c_str() );
    mLabelWidth = size.width();
    mMarkerHeight = size.height();
    if( mShowChannelLabels )
    {
      for( size_t i = 0; i < mChannelLabels.size(); ++i )
      {
        int address = mChannelLabels[ i ].Address();
        if( address >= int( mChannelNameCache.size() ) )
          mChannelNameCache.resize( address + 1, "" );
        const string& label = mChannelLabels[ i ].Text();
        mChannelNameCache[ address ] = label;
        if( address >= mData.Channels() - mMarkerChannels )
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
        if( mChannelUnit.empty() )
          oss << i + cChannelBase;
        else
          oss << fixed << setprecision( 1 ) 
              << i * mUnitsPerChannel << mChannelUnit;
        mChannelNameCache[ i ] = oss.str();
      }
      const string& label = mChannelNameCache[i];
      size = fontMetrics.size( 0, label.c_str() );
      if( size.width() > mLabelWidth )
        mLabelWidth = size.width();
    }
    mLabelWidth += 3 * cAxisWidth + cTickWidth;
  }
}

inline
void
SignalDisplay::SyncGraphics()
{
  mDataRect.setLeft( mLabelWidth );
  mDataRect.setTop( 0 );
  mDataRect.setRight( mDisplayRect.right() - mDisplayRect.left() );
  mDataRect.setBottom( mDisplayRect.bottom() - mDisplayRect.top() );
  mDataWidth = max<int>( 0, mDataRect.right() - mDataRect.left() );
  mDataHeight = max<int>( 0, mDataRect.bottom() - mDataRect.top()
                             - mMarkerHeight * ( mMarkerChannels + 2 )
                             + cTickLength / 2 );
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
SignalDisplay::Paint( const void* inUpdateRgn )
{
  if( mTargetDC == NULL )
    return *this;

  OSMutex::Lock lock( mDataLock );
  PaintInfo p;
  SetupPainting( p, inUpdateRgn );
  ClearBackground( p );
  DrawXTicks( p );
  DrawAxes( p );
  switch( mDisplayMode )
  {
    case polyline:
      DrawSignalPolyline( p );
      DrawYLabels( p, true );
      DrawValueUnit( p );
      if( mShowNumericValues )
        DrawNumericValues( p );
      break;
    case field2d:
      DrawSignalField2d( p );
      DrawYLabels( p, false );
      break;
    default:
      bciassert( false );
  }
  DrawMarkerChannels( p );
  DrawMarkers( p );
  DrawChannelLabels( p );
  DrawCursor( p );

#ifdef TEST_UPDATE_RGN
  p.painter->fillRect( mDisplayRect, QColor( ::rand() << 8 & 0x00ffff00 ) );
#endif // TEST_UPDATE_RGN

  CleanupPainting( p );
  return *this;
}

void
SignalDisplay::SetupPainting( PaintInfo& p, const void* inUpdateRgn )
{
  SyncGraphics();

  p.updateRgn = reinterpret_cast<const QRegion*>( inUpdateRgn );
  p.painter = new QPainter( mTargetDC );
  p.painter->setRenderHint( QPainter::Antialiasing, false );
  p.painter->setRenderHint( QPainter::TextAntialiasing, false );
  p.signalPens.resize( mData.Channels() );
  p.signalBrushes.resize( mData.Channels() );

  // Background properties
  RGBColor c = mInverted ? RGBColor::White : RGBColor::Black;
  p.backgroundColor = QColor( c.R(), c.G(), c.B() );
  p.backgroundBrush = QBrush( p.backgroundColor );

  // Cursor properties
  p.cursorWidth = 3;
  c = RGBColor::Yellow;
  p.cursorColor = QColor( c.R(), c.G(), c.B() );
  p.cursorBrush = QBrush( p.cursorColor );

  // Axis properties
  RGBColor axisColor = mAxisColor;
  if( mInverted && ( axisColor == RGBColor::White ) )
    axisColor = RGBColor::Black;
  QColor qAxisColor( axisColor.R(), axisColor.G(), axisColor.B() );
  p.axisBrush = QBrush( qAxisColor );
  p.axisY = GroupBottom( mNumDisplayGroups - 1 );

  p.markerWidth = 1;
  p.baselinePen = QPen( qAxisColor );
  c = RGBColor( mInverted ? RGBColor::Black : RGBColor::White );
  p.markerColor = QColor( c.R(), c.G(), c.B() );
  p.markerBrush = QBrush( p.markerColor );
  p.labelColor = qAxisColor;
  p.labelFont = AxisFont();
  p.monoFont = MonoFont();


  // Signal properties
  if( mDisplayColors )
    for( int i = 0; i < mData.Channels(); ++i )
    {
      RGBColor channelColor = ChannelColor( i );
      if( mInverted && channelColor == RGBColor::White )
        channelColor = RGBColor::Black;
      QColor c( channelColor.R(), channelColor.G(), channelColor.B() );
      p.signalPens[ i ] = QPen( c );
      p.signalBrushes[ i ] = QBrush( c );
    }
  else
  {
    RGBColor channelColor = mInverted? RGBColor::Black : RGBColor::White;
    QColor c( channelColor.R(), channelColor.G(), channelColor.B() );
    QPen pen( c );
    for( int i = 0; i < mData.Channels(); ++i )
      p.signalPens[ i ] = pen;
  }
}

void
SignalDisplay::ClearBackground( const PaintInfo& p )
{
  p.painter->fillRect( mDisplayRect, p.backgroundColor );
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
  }

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
  try
  {
    mpSignalPoints = new QPoint[ mNumSamples ];
  }
  catch( const bad_alloc& )
  {
    throw bciexception( "Could not allocate memory for " << mNumSamples << " points" );
  }
  int numPens = static_cast<int>( p.signalPens.size() );

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
}

void
SignalDisplay::DrawSignalField2d( const PaintInfo& p )
{
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
}

void
SignalDisplay::DrawMarkerChannels( const PaintInfo& p )
{
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
}

void
SignalDisplay::DrawCursor( const PaintInfo& p )
{
  if( mShowCursor )
  {
    int cursorSample = mSampleCursor;

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
}

void
SignalDisplay::DrawXTicks( const PaintInfo& p )
{
  p.painter->setPen( p.labelColor );
  p.painter->setFont( p.labelFont );
  p.painter->setBackgroundMode( Qt::TransparentMode );
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

  int nextLabelPos = mDataRect.left();
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
    QRect tickRect(
        tickX - cTickWidth / 2,
        p.axisY + cAxisWidth,
        cTickWidth,
        cTickLength
    );
    p.painter->fillRect( tickRect, p.axisBrush );
    if( tickX > nextLabelPos )
    {
      tickRect.setTop( tickRect.top() + 2 * cAxisWidth );
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

      p.painter->drawText( tickRect,
        Qt::AlignHCenter | Qt::TextSingleLine | Qt::AlignTop | Qt::TextDontClip,
        label.str().c_str() );
      nextLabelPos = tickX + p.painter->fontMetrics().width( label.str().c_str() );
    }
  }
}

void
SignalDisplay::DrawYLabels( const PaintInfo& p, bool inDrawTicks )
{
  p.painter->setPen( p.labelColor );
  p.painter->setFont( p.labelFont );
  p.painter->setBackgroundMode( Qt::TransparentMode );
  int nextLabelPos = mDataRect.top();
  for( int i = 0; i < mNumDisplayGroups; ++i )
  {
    size_t channelNumber = ( mTopGroup + i ) * mChannelGroupSize;
    int    tickY = ( GroupTop( i ) + GroupBottom( i ) ) / 2;
    QRect tickRect(
        mLabelWidth - cAxisWidth - cTickLength,
        tickY - cTickWidth / 2,
        cTickLength,
        cTickWidth
    );
    if( inDrawTicks )
    {
      if( mDisplayColors && mChannelGroupSize == 1 )
      {
        tickRect.setTop( tickRect.top() - 1 );
        tickRect.setBottom( tickRect.bottom() + 1 );
        p.painter->fillRect( tickRect, p.signalBrushes[ channelNumber ] );
      }
      else
      {
        p.painter->fillRect( tickRect, p.axisBrush );
      }
    }
    if( tickY >= nextLabelPos )
    {
      const char* labelText = "";
      if( mChannelGroupSize == 1 && mChannelNameCache.size() > channelNumber )
        labelText = mChannelNameCache[ channelNumber ].c_str();
      tickRect.setRight( tickRect.right() - 2 * cAxisWidth );
      if( mChannelGroupSize == 1 )
        p.painter->drawText( tickRect,
          Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignRight | Qt::TextDontClip,
          labelText );
      nextLabelPos = tickY + p.painter->fontMetrics().height();
    }
  }
}

void
SignalDisplay::DrawNumericValues( const PaintInfo& p )
{
  p.painter->setPen( p.labelColor );
  p.painter->setFont( p.monoFont );
  p.painter->setBackgroundMode( Qt::TransparentMode );
  int nextLabelPos = mDataRect.top();

  if( !mData.Properties().IsEmpty() )
  {
    for( int i = 0; i < mNumDisplayGroups; ++i )
    {
      size_t channelNumber = ( mTopGroup + i ) * mChannelGroupSize;
      int    tickY = ( GroupTop( i ) + GroupBottom( i ) ) / 2;
      stringstream ss;
      ss.setf( ios::fixed );
      ss.precision( 10 );
      int sampleNumber = mSampleCursor - 1;
      if( sampleNumber < 0 ) sampleNumber += mData.Elements();
      ss << mData( channelNumber, sampleNumber ) << " ";
      string s = ss.str();
      const char* labelText = s.c_str();
      QFontMetrics metrics( p.monoFont );
      int w = metrics.width( labelText );
      int h = metrics.height();
      if( mNumericValueWidth < w ) mNumericValueWidth = w; 
      QRect tickRect(
          mDisplayRect.width() - w,
          tickY - h * 1.5,
          w,
          h
      );

      if( tickY >= nextLabelPos )
      {
        if( mChannelGroupSize == 1 )
        {
          p.painter->fillRect( tickRect, p.backgroundColor );
          p.painter->drawText( tickRect,
            Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignRight,
            labelText );
        }
        nextLabelPos = tickY + p.painter->fontMetrics().height();
      }
    }
  }
}

void
SignalDisplay::DrawAxes( const PaintInfo& p )
{
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
}

void
SignalDisplay::DrawMarkers( const PaintInfo& p )
{
  // Draw markers.
  for( size_t i = 0; i < mXAxisMarkers.size(); ++i )
  {
    int markerX = SampleRight( static_cast<int>( mXAxisMarkers[ i ].Address() - mSampleOffset ) );
    QRect markerBar(
        markerX - cAxisWidth / 2,
        p.axisY - 4 * cAxisWidth,
        cAxisWidth,
        4 * cAxisWidth - 1
    );
    p.painter->fillRect( markerBar, p.markerBrush );
  }
}

void
SignalDisplay::DrawChannelLabels( const PaintInfo& p )
{
  p.painter->setFont( p.labelFont );
  if( mShowChannelLabels && mChannelGroupSize > 1 )
  {  // Draw channel labels when channels don't coincide with groups.
    p.painter->setBackground( p.backgroundColor );
    p.painter->setBackgroundMode( Qt::OpaqueMode );
    QRect legendRect;
    for( size_t i = 0; i < mChannelLabels.size(); ++i )
    {
      RGBColor textColor = ChannelColor( mChannelLabels[ i ].Address() );
      if( mInverted && textColor == RGBColor::White )
        textColor = RGBColor::Black;
      p.painter->setPen( QColor( textColor.R(), textColor.G(), textColor.B() ) );
      p.painter->drawText( legendRect,
        Qt::TextSingleLine | Qt::AlignLeft | Qt::TextDontClip,
        mChannelLabels[ i ].Text().c_str() );
      legendRect.setTop( legendRect.top() + p.painter->fontMetrics().height() );
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
    p.painter->setFont( p.labelFont );
    p.painter->setPen( p.backgroundColor );
    p.painter->setBackground( p.markerColor );
    p.painter->setBackgroundMode( Qt::OpaqueMode );
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
    }
    else
    {
      int left = SampleLeft( 0 ),
          center = ( GroupBottom( mNumDisplayGroups - 1 ) + GroupTop( mNumDisplayGroups - 1 ) ) / 2;
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
    }
  }
}

void
SignalDisplay::CleanupPainting( PaintInfo& p )
{
  delete p.painter;
}

