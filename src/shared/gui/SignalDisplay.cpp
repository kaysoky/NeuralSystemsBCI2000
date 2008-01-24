////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A class that draws GenericSignal data into a given window,
//          and maintains a context menu.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SignalDisplay.h"

#include <cassert>
#include <cmath>
#include <sstream>
#include <iomanip>

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
: mShowCursor( false ),
  mWrapAround( false ),
  mTimeLabels( false ),
  mNumSamples( cNumSamplesDefault ),
  mSampleCursor( 0 ),
  mNumDisplayGroups( 0 ),
  mNumDisplayChannels( 0 ),
  mTopGroup( 0 ),
  mShowBaselines( false ),
  mShowChannelLabels( false ),
  mShowValueUnit( false ),
  mDisplayColors( true ),
  mChannelGroupSize( 1 ),
  mMarkerChannels( 0 ),
  mMinValue( cMinValueDefault ),
  mMaxValue( cMaxValueDefault ),
  mSampleOffset( 0 ),
  mUnitsPerSample( 1 ),
  mUnitsPerChannel( 1 ),
  mUnitsPerValue( 1 ),
  mDisplayMode( polyline ),
  mRedrawRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ),
  mAxisColor( cAxisColorDefault ),
  mChannelColors( cChannelColorsDefault ),
  mDisplayRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ),
  mDataWidth( 0 ),
  mDataHeight( 0 ),
  mLabelWidth( cLabelWidth ),
  mMarkerHeight( 0 ),
  mpHandles( NULL )
{
  RECT r = { 0, 0, 0, 0 };
  mDisplayRect = r;
}

SignalDisplay::~SignalDisplay()
{
  delete mpHandles;
  ::DeleteObject( mRedrawRgn );
  ::DeleteObject( mDisplayRgn );
}

SignalDisplay&
SignalDisplay::SetContext( const GUI::DrawContext& dc )
{
  RECT r =
  {
    dc.rect.left,
    dc.rect.top,
    dc.rect.right,
    dc.rect.bottom
  };
  mDisplayRect = r;
  ::SetRectRgn( mDisplayRgn, r.left, r.top, r.right, r.bottom );
  delete mpHandles;
  mpHandles = new BunchOfHandles( dc.handle );
  return Invalidate();
}

SignalDisplay&
SignalDisplay::Invalidate()
{
  if( mpHandles != NULL && mpHandles->targetWindow != NULL )
    ::InvalidateRect( mpHandles->targetWindow, &mDisplayRect, false );
  return *this;
}

void
SignalDisplay::AdaptTo( const GenericSignal& inSignal )
{
  // Any changes in the signal size that we must react to?
  bool reconfigure = false;
  if( inSignal.Elements() > mNumSamples )
  {
    SetNumSamples( inSignal.Elements() );
    reconfigure = true;
  }
  if( inSignal.Channels() != mData.Channels() )
  {
    int newNumDisplayGroups = ( inSignal.Channels() - mMarkerChannels - 1 ) / mChannelGroupSize + 1;
    if( mNumDisplayGroups == 0 )
      mNumDisplayGroups = min( newNumDisplayGroups, cInitialMaxDisplayGroups );
    else if( newNumDisplayGroups < mNumDisplayGroups )
      mNumDisplayGroups = newNumDisplayGroups;
    reconfigure = true;
  }
  if( reconfigure )
  {
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
  if( mpHandles == NULL )
    return *this;

  AdaptTo( inSignal );

  mShowCursor = ( inSignal.Elements() < mNumSamples );

  for( int i = 0; i < inSignal.Channels(); ++i )
    for( int j = 0; j < inSignal.Elements(); ++j )
      mData( i, ( mSampleCursor + j ) % mData.Elements() ) = inSignal( i, j );

  SyncGraphics();

  int firstInvalidSample = mSampleCursor,
      firstValidSample = mSampleCursor + inSignal.Elements();
  if( mNumSamples > 0 )
  {
    mSampleCursor = firstValidSample % mNumSamples;
    mWrapAround |= bool( firstValidSample / mNumSamples > 0  );
  }

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
  if( mpHandles->targetWindow != NULL )
  {
    ::InvalidateRgn( mpHandles->targetWindow, mRedrawRgn, false );
    ::SetRectRgn( mRedrawRgn, 0, 0, 0, 0 );

    RECT invalidRect = mDataRect;

    // The non-wrapped area.
    invalidRect.left = max( firstInvalidPixel, mDataRect.left );
    invalidRect.right = min( firstValidPixel, mDataRect.right );
    if( invalidRect.left < invalidRect.right )
      ::InvalidateRect( mpHandles->targetWindow, &invalidRect, false );

    // The area wrapped around the left edge.
    invalidRect.left = max( firstInvalidPixel + mDataWidth, mDataRect.left );
    invalidRect.right = min( firstValidPixel + mDataWidth, mDataRect.right );
    if( invalidRect.left < invalidRect.right )
      ::InvalidateRect( mpHandles->targetWindow, &invalidRect, false );

    // The area wrapped around the right edge.
    invalidRect.left = max( firstInvalidPixel - mDataWidth, mDataRect.left );
    invalidRect.right = min( firstValidPixel - mDataWidth, mDataRect.right );
    if( invalidRect.left < invalidRect.right )
      ::InvalidateRect( mpHandles->targetWindow, &invalidRect, false );
  }
  return *this;
}

SignalDisplay&
SignalDisplay::ScrollForward( const GenericSignal& inSignal )
{
  if( mpHandles == NULL )
    return *this;

  if( inSignal.Elements() < 1 )
    return *this;

  AdaptTo( inSignal );
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
  if( mpHandles == NULL )
    return *this;

  if( inSignal.Elements() < 1 )
    return *this;

  AdaptTo( inSignal );
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

HGDIOBJ
SignalDisplay::AxisFont()
{
  const fontHeight = cLabelWidth / 2;
  return ::CreateFont( -fontHeight, 0, 0, 0, FW_DONTCARE,
             false, false, false, ANSI_CHARSET, OUT_RASTER_PRECIS,
             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
             VARIABLE_PITCH | FF_SWISS, NULL );
}

void
SignalDisplay::SyncLabelWidth()
{
  mChannelNameCache.clear();
  if( mpHandles == NULL )
  {
    mLabelWidth = cLabelWidth;
    mMarkerHeight = mLabelWidth;
  }
  else
  {
    HDC dc = mpHandles->targetDC;
    HGDIOBJ font = AxisFont();
    ::SelectObject( dc, font );
    SIZE size = { cLabelWidth, 0 };
    ostringstream oss;
    oss << mData.Channels();
    if( ::GetTextExtentPoint32( dc, oss.str().c_str(), oss.str().length(), &size ) )
    {
      mLabelWidth = size.cx;
      mMarkerHeight = size.cy;
    }
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
          if( ::GetTextExtentPoint32( dc, label.c_str(), label.length(), &size )
              && size.cx > mLabelWidth )
            mLabelWidth = size.cx;
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
    ::DeleteObject( font );
    mLabelWidth += 3 * cAxisWidth + cTickWidth;
  }
}

inline
void
SignalDisplay::SyncGraphics()
{
  mDataRect.left = mLabelWidth;
  mDataRect.top = 0;
  mDataRect.right = mDisplayRect.right - mDisplayRect.left;
  mDataRect.bottom = mDisplayRect.bottom - mDisplayRect.top;
  mDataWidth = std::max<int>( 0, mDataRect.right - mDataRect.left );
  mDataHeight = std::max<int>( 0, mDataRect.bottom - mDataRect.top
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
    mData = GenericSignal( mData.Channels(), newNumSamples );
    mSampleCursor = 0;
  }
  mNumSamples = newNumSamples;
  return *this;
}

SignalDisplay&
SignalDisplay::Paint( void* inUpdateRegion )
{
  if( mpHandles == NULL )
    return *this;

  HRGN updateRgn
   = inUpdateRegion ? static_cast<HRGN>( inUpdateRegion ) : mDisplayRgn;

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
  const COLORREF backgroundColor = RGBColor( RGBColor::Black ).ToWinColor();
  gdi[ backgroundBrush ] = ::CreateSolidBrush( backgroundColor );

  // Cursor properties
  const COLORREF cursorColor = RGBColor( RGBColor::Yellow ).ToWinColor();
  const cursorWidth = 3;
  gdi[ cursorBrush ] = ::CreateSolidBrush( cursorColor );

  // Axis properties
  const COLORREF axisColor = mAxisColor.ToWinColor();
  gdi[ axisBrush ] = ::CreateSolidBrush( axisColor );
  gdi[ baselinePen ] = ::CreatePen( PS_SOLID, 0, axisColor );
  const COLORREF markerColor = RGBColor( RGBColor::White ).ToWinColor();
  const markerWidth = 1;
  gdi[ markerBrush ] = ::CreateSolidBrush( markerColor );

  const labelColor = axisColor;
  gdi[ labelFont ] = AxisFont();

  // Signal properties
  if( mDisplayColors )
    for( int i = 0; i < mData.Channels(); ++i )
    {
      signalPens[ i ] = ::CreatePen( PS_SOLID, 0, ChannelColor( i ).ToWinColor() );
      signalBrushes[ i ] = ::CreateSolidBrush( ChannelColor( i ).ToWinColor() );
    }
  else
  {
    HGDIOBJ pen = ::CreatePen( PS_SOLID, 0, RGBColor( RGBColor::White ).ToWinColor() );
    for( int i = 0; i < mData.Channels(); ++i )
      signalPens[ i ] = pen;
  }

  // Do the drawing.
  int width = mDisplayRect.right - mDisplayRect.left,
      height = mDisplayRect.bottom - mDisplayRect.top;

  HBITMAP offscreenBmp = NULL;
  HDC dc = mpHandles->targetDC;
  switch( ::GetObjectType( dc ) )
  {
    case OBJ_METADC:
    case OBJ_ENHMETADC:
      break;

    default:
      dc = ::CreateCompatibleDC( mpHandles->targetDC );
      offscreenBmp = ::CreateCompatibleBitmap( mpHandles->targetDC, width, height );
      ::DeleteObject( ::SelectObject( dc, offscreenBmp ) );
      ::SelectClipRgn( dc, updateRgn );
      ::OffsetClipRgn( dc, -mDisplayRect.left, -mDisplayRect.top );
  }
  ::SelectClipRgn( mpHandles->targetDC, updateRgn );

  // Clear the background.
  RECT fullArea = { 0, 0, width, height };
  ::FillRect( dc, &fullArea, gdi[ backgroundBrush ] );

  // Draw the signal.
  SyncGraphics();
  switch( mDisplayMode )
  {
    case polyline:
    {
      float baseInterval = mNumDisplayGroups > 0
                           ? mDataHeight / mNumDisplayGroups
                           : mDataHeight;
      // Draw the baselines.
      if( mShowBaselines )
      {
        POINT baselinePoints[ 2 ];
        ::SelectObject( dc, gdi[ baselinePen ] );
        baselinePoints[ 0 ].x = SampleLeft( 0 );
        baselinePoints[ 1 ].x = SampleRight( mNumSamples );
        for( int i = 0; i < mNumDisplayGroups; ++i )
        {
          baselinePoints[ 0 ].y = ChannelBottom( i ) + ( baseInterval * mMinValue ) / ( mMaxValue - mMinValue );
          baselinePoints[ 1 ].y = baselinePoints[ 0 ].y;
          ::Polyline( dc, baselinePoints, 2 );
        }
      }

      mSignalPoints.resize( mNumSamples );
      for( int i = 0; i < mNumDisplayChannels; ++i )
      {
        for( int j = 0; j < mNumSamples; ++j )
        {
          mSignalPoints[ j ].x = SampleLeft( j );
          mSignalPoints[ j ].y = ChannelBottom( i ) - 1
               - baseInterval * NormData( i + mTopGroup * mChannelGroupSize, j );
        }

        ::SelectObject( dc, signalPens[ ( i + mTopGroup * mChannelGroupSize ) % signalPens.size() ] );
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
    } break;

    case field2d:
    {
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
          if( ::RectVisible( dc, &dotRect ) )
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
  // Draw the marker channels.
  int axisY = GroupBottom( mNumDisplayGroups - 1 );
  for( int markerCh = 0; markerCh < mMarkerChannels; ++markerCh )
  {
    ::SetTextColor( dc, labelColor );
    ::SelectObject( dc, gdi[ labelFont ] );
    ::SetBkMode( dc, TRANSPARENT );
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
            SampleLeft( sample ) - markerWidth,
            baseline,
            SampleLeft( sample ),
            baseline
          };
          ostringstream oss;
          oss << curVal;
          ::DrawText( dc,
              oss.str().c_str(),
              -1, &posRect, DT_CENTER | DT_SINGLELINE | DT_BOTTOM | DT_NOCLIP );
          RECT markerRect =
          {
            SampleLeft( sample ) - markerWidth,
            GroupTop( 0 ),
            SampleLeft( sample ),
            axisY
          };
          ::FillRect( dc, &markerRect, gdi[ axisBrush ] );
          prevVal = curVal;
        }
      }
    }
    ::SetBkColor( dc, backgroundColor );
    ::SetBkMode( dc, OPAQUE );
    RECT labelRect =
    {
      cTickWidth,
      baseline,
      cTickWidth,
      baseline
    };
    ::DrawText( dc, mChannelNameCache[ channelNumber ].c_str(),
       -1, &labelRect, DT_LEFT | DT_SINGLELINE | DT_BOTTOM | DT_NOCLIP );
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
      GroupBottom( mNumDisplayGroups - 1 )
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
  // Ticks and labels on the y axis.
  switch( mDisplayMode )
  {
    case field2d:
      break;
    case polyline:
      {
        int nextLabelPos = mDataRect.top;
        for( int i = 0; i < mNumDisplayGroups; ++i )
        {
          int channelNumber = ( mTopGroup + i ) * mChannelGroupSize,
              tickY = ( GroupTop( i ) + GroupBottom( i ) ) / 2;
          RECT tickRect =
          {
            mLabelWidth - cAxisWidth - cTickLength,
            tickY - cTickWidth / 2,
            mLabelWidth - cAxisWidth,
            tickY + cTickWidth / 2
          };
          if( mDisplayColors && mChannelGroupSize == 1 )
          {
            tickRect.top -= 1;
            tickRect.bottom += 1;
            ::FillRect( dc, &tickRect, signalBrushes[ channelNumber ] );
          }
          else
            ::FillRect( dc, &tickRect, gdi[ axisBrush ] );
          if( tickY >= nextLabelPos )
          {
            tickRect.right -= 2 * cAxisWidth;
            nextLabelPos = tickY + ::DrawText( dc, mChannelGroupSize == 1 ?
               mChannelNameCache[ channelNumber ].c_str() : "",
               -1, &tickRect, DT_RIGHT | DT_SINGLELINE | DT_VCENTER | DT_NOCLIP );
          }
        }
      } break;
    default:
      assert( false );
  }
  // Ticks on the x axis.
  float xStart = 0,
        xDivision = 1;
  if( mNumSamples > 0 )
  { // Are samples spaced too dense to allow for individual labels?
    float pixelsPerSample = ( SampleLeft( mNumSamples - 1 ) - SampleLeft( 0 ) ) / mNumSamples;
    if( pixelsPerSample < 10 )
    { // Samples are dense
      float displayLength = ::fabs( mNumSamples * mUnitsPerSample ),
            scale = ::pow( 10, ::floor( ::log10( displayLength ) + 0.5 ) );
      xDivision = scale / mUnitsPerSample / 5,
      xStart = xDivision - ::fmod( mSampleOffset, xDivision );
    }
  }
  if( xDivision < 1 )
    xDivision = 1;

  int nextLabelPos = mDataRect.left;
  for( float j = xStart; j < float( mNumSamples ); j += xDivision )
  {
    int tickX = 0;
    switch( mDisplayMode )
    {
      case field2d:
        tickX = ( SampleRight( j ) + SampleLeft( j ) ) / 2;
        break;
      case polyline:
      default:
        tickX = SampleLeft( j );
    }
    RECT tickRect =
    {
      tickX - cTickWidth / 2,
      axisY + cAxisWidth,
      tickX + cTickWidth / 2,
      axisY + cAxisWidth + cTickLength
    };
    ::FillRect( dc, &tickRect, gdi[ axisBrush ] );
    if( tickX > nextLabelPos )
    {
      tickRect.top += 2 * cAxisWidth;
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
    axisY,
    mDisplayRect.right,
    axisY + cAxisWidth
  };
  ::FillRect( dc, &xAxis, gdi[ axisBrush ] );
  RECT yAxis =
  {
    mLabelWidth - cAxisWidth,
    mMarkerChannels > 0 ? GroupTop( 0 ) : 0,
    mLabelWidth,
    mDisplayRect.bottom
  };
  ::FillRect( dc, &yAxis, gdi[ axisBrush ] );
  if( mMarkerChannels > 0 )
  {
    RECT divider =
    {
      0,
      GroupTop( 0 ) - cAxisWidth,
      mDisplayRect.right,
      GroupTop( 0 )
    };
    ::FillRect( dc, &divider, gdi[ axisBrush ] );
  }
  // Draw markers.
  for( size_t i = 0; i < mXAxisMarkers.size(); ++i )
  {
    int markerX = SampleRight( mXAxisMarkers[ i ].Address() - mSampleOffset );
    RECT markerBar =
    {
      markerX - cAxisWidth / 2,
      axisY - 4 * cAxisWidth,
      markerX + cAxisWidth / 2,
      axisY - 1
    };
    ::FillRect( dc, &markerBar, gdi[ markerBrush ] );
  }

  // Draw channel labels if channels don't coincide with groups.
  if( mShowChannelLabels && mChannelGroupSize > 1 )
  {
    ::SetBkColor( dc, RGBColor( RGBColor::Black ).ToWinColor() );
    ::SetBkMode( dc, OPAQUE );
    RECT legendRect =
    {
      0, 0, 0, 0
    };
    for( size_t i = 0; i < mChannelLabels.size(); ++i )
    {
      ::SetTextColor( dc, ChannelColor( mChannelLabels[ i ].Address() ).ToWinColor() );
      legendRect.top += ::DrawText( dc, mChannelLabels[ i ].Text().c_str(),
        mChannelLabels[ i ].Text().length(), &legendRect,
        DT_SINGLELINE | DT_LEFT | DT_NOCLIP | DT_EXTERNALLEADING );
    }
  }
  // Draw the value unit.
  switch( mDisplayMode )
  {
    case polyline:
    {
      float baseInterval = mDataHeight;
      if( mNumDisplayGroups > 0 )
        baseInterval /= mNumDisplayGroups;

      if( mShowValueUnit && mNumDisplayGroups > 0 && baseInterval > 0 )
      {
        ::SelectObject( dc, gdi[ labelFont ] );
        ::SetTextColor( dc, backgroundColor );
        ::SetBkColor( dc, markerColor );
        ::SetBkMode( dc, OPAQUE );
        // Find a round value that is near the display range.
        float unitsPerPixel = ::fabs( ( mMaxValue - mMinValue ) * mUnitsPerValue / baseInterval ),
              scale = ::pow( 10, ::ceil( ::log10( unitsPerPixel * 0.95 * baseInterval ) ) ),
              rulerLength = scale;
        while( rulerLength / unitsPerPixel >= 0.95 * baseInterval
               && rulerLength / unitsPerPixel > mMarkerHeight )
          rulerLength -= scale / 10;
        int pixelLength = rulerLength / unitsPerPixel;

        ostringstream label;
        label << rulerLength << mValueUnit;
        if( mMinValue == 0 )
        {
          int left = SampleLeft( 0 ) + cTickLength,
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
              center = ( GroupBottom( mNumDisplayGroups - 1 ) + GroupTop( mNumDisplayGroups - 1 ) ) / 2;
          RECT labelRect =
          {
            left,
            center,
            left,
            center
          };
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

          labelRect.top = center;
          labelRect.bottom = labelRect.top;
          ::DrawText( dc, label.str().c_str(), -1, &labelRect,
              DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP );
        }
      }
    } break;

    case field2d:
      break;

    default:
      assert( false );
  }

#ifdef TEST_UPDATE_RGN
  HBRUSH brush = ::CreateSolidBrush( ::rand() & 0x0000ffff );
  ::FillRect( dc, &mDisplayRect, brush );
  ::DeleteObject( brush );
#endif

  // Copy the data from the buffer into the target device context (usually a window).
  if( offscreenBmp )
  {
    ::BitBlt( mpHandles->targetDC,
              mDisplayRect.left,
              mDisplayRect.top,
              mDisplayRect.right - mDisplayRect.left,
              mDisplayRect.bottom - mDisplayRect.top,
              dc,
              0,
              0,
              SRCCOPY
    );
    ::DeleteObject( dc );
    ::DeleteObject( offscreenBmp );
  }
  return *this;
}


