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
#ifndef SIGNAL_DISPLAY_H
#define SIGNAL_DISPLAY_H

#ifdef __BORLANDC__
# error Borland specific code has been removed from the SignalDisplay class.
#endif // __BORLANDC__

#include "GUI.h"
#include "GenericSignal.h"
#include "Color.h"
#include "Label.h"
#include "OSMutex.h"
#include <set>
#include <vector>

#include <QFont>
#include <QPoint>
#include <QPaintDevice>
#include <QRect>
#include <QRegion>
#include <QColor>
#include <QFont>
#include <QPen>

class SignalDisplay
{
 private:
  enum
  {
    cNumSamplesDefault = 128,
    cMinValueDefault = - ( 1 << 15 ),
    cMaxValueDefault = ( 1 << 16 ) - 1,

    cChannelBase = 1, // displayed number of first channel
    cSampleBase = 0,  // displayed number of first sample

    cLabelWidth = 25,
    cAxisWidth = 2,
    cTickWidth = cAxisWidth,
    cTickLength = 4,
    cInitialMaxDisplayGroups = 32,
  };

  static const RGBColor cAxisColorDefault,
                        cChannelColorsDefault[];

 public:
  enum eDisplayMode
  {
    polyline = 0,
    field2d,
    /* ... */
    numDisplayModes
  };

 public:
  SignalDisplay();
  virtual ~SignalDisplay();

 private:
  SignalDisplay( const SignalDisplay& );
  SignalDisplay& operator=( const SignalDisplay& );

 public:
  SignalDisplay& SetContext( const GUI::DrawContext& );
  SignalDisplay& Invalidate();
  SignalDisplay& Paint( const void* RegionHandle = NULL );

  SignalDisplay& WrapForward( const GenericSignal& );
  SignalDisplay& ScrollForward( const GenericSignal& );
  SignalDisplay& ScrollBack( const GenericSignal& );

  SignalDisplay& SetNumSamples( int );
  int            NumSamples() const
                 { return mNumSamples; }
  SignalDisplay& SetDisplayGroups( int );
  int            DisplayGroups() const
                 { return mNumDisplayGroups; }
  SignalDisplay& SetChannelGroupSize( int i )
                 { mChannelGroupSize = i; return Invalidate(); }
  int            ChannelGroupSize() const
                 { return mChannelGroupSize; }
  SignalDisplay& SetNumMarkerChannels( int n )
                 { mMarkerChannels = n; return Invalidate(); }
  int            NumMarkerChannels() const
                 { return mMarkerChannels; }

  SignalDisplay& SetMinValue( float f )
                 { mMinValue = f; return Invalidate(); }
  float          MinValue() const
                 { return mMinValue; }
  SignalDisplay& SetMaxValue( float f )
                 { mMaxValue = f; return Invalidate(); }
  float          MaxValue() const
                 { return mMaxValue; }

  SignalDisplay& SetSampleOffset( float f )
                 { mSampleOffset = f; return Invalidate(); }
  float          SampleOffset() const
                 { return mSampleOffset; }
  SignalDisplay& SetSampleUnit( const std::string& s )
                 { mSampleUnit = s; mTimeLabels = ( s == ":s" ); return Invalidate(); }
  const std::string& SampleUnit() const
                 { return mSampleUnit; }
  SignalDisplay& SetValueUnit( const std::string& s )
                 { mValueUnit = s; return Invalidate(); }
  const std::string& ValueUnit() const
                 { return mValueUnit; }
  SignalDisplay& SetChannelUnit( const std::string& s )
                 { mChannelUnit = s; return Invalidate(); }
  const std::string& ChannelUnit() const
                 { return mChannelUnit; }

  SignalDisplay& SetUnitsPerSample( float f )
                 { mUnitsPerSample = f; return Invalidate(); }
  float          UnitsPerSample() const
                 { return mUnitsPerSample; }
  SignalDisplay& SetUnitsPerValue( float f )
                 { mUnitsPerValue = f; return Invalidate(); }
  float          UnitsPerValue() const
                 { return mUnitsPerValue; }
  SignalDisplay& SetUnitsPerChannel( float f )
                 { mUnitsPerChannel = f; return Invalidate(); }
  float          UnitsPerChannel() const
                 { return mUnitsPerChannel; }

  SignalDisplay& SetAxisColor( const RGBColor& c )
                 { mAxisColor = c; return Invalidate(); }
  const RGBColor& AxisColor() const
                 { return mAxisColor; }
  SignalDisplay& SetChannelColors( const ColorList& l )
                 { mChannelColors = l; return Invalidate(); }
  const ColorList& ChannelColors() const
                 { return mChannelColors; }
  SignalDisplay& SetChannelLabels( const LabelList& l )
                 { mChannelLabels = l; SyncLabelWidth(); return Invalidate(); }
  const LabelList& ChannelLabels() const
                 { return mChannelLabels; }
  SignalDisplay& SetXAxisMarkers( const LabelList& l )
                 { mXAxisMarkers = l; return Invalidate(); }
  const LabelList& XAxisMarkers() const
                 { return mXAxisMarkers; }

  SignalDisplay& SetTopGroup( int );
  int            TopGroup() const
                 { return mTopGroup; }

  SignalDisplay& SetDisplayMode( int );
  int            DisplayMode() const
                 { return mDisplayMode; }

  SignalDisplay& SetNumericValuesVisible( bool b )
                 { mShowNumericValues = b; return Invalidate(); }
  SignalDisplay& SetBaselinesVisible( bool b )
                 { mShowBaselines = b; return Invalidate(); }
  bool           NumericValuesVisible() const
                 { return mShowNumericValues; }
  bool           BaselinesVisible() const
                 { return mShowBaselines; }
  SignalDisplay& SetChannelLabelsVisible( bool b )
                 { mShowChannelLabels = b; SyncLabelWidth(); return Invalidate(); }
  bool           ChannelLabelsVisible() const
                 { return mShowChannelLabels; }
  SignalDisplay& SetValueUnitVisible( bool b )
                 { mShowValueUnit = b; return Invalidate(); }
  bool           ValueUnitVisible() const
                 { return mShowValueUnit; }

  SignalDisplay& SetColorDisplay( bool b )
                 { mDisplayColors = b; return Invalidate(); }
  bool           ColorDisplay() const
                 { return mDisplayColors; }

  SignalDisplay& SetInverted( bool b )
                 { mInverted = b; return Invalidate(); }
  bool           Inverted() const
                 { return mInverted; }

 // Functions that centralize sample/channel -> pixel conversion in painting
 // and invalidating contexts.
 private:
  int  mDataWidth,
       mDataHeight,
       mLabelWidth,
       mMarkerHeight;

  void SyncLabelWidth();
  void SyncGraphics();
  void AdaptTo( const GenericSignal& );

  inline
  int SampleLeft( int s )
    { return mLabelWidth + ( mNumSamples ? ( s * mDataWidth ) / int( mNumSamples ) : 0 ); }
  inline
  int SampleRight( int s )
    { return SampleLeft( s + 1 ); }
  inline
  int PosToSample( int p )
    { return mDataWidth ? ( ( p - mLabelWidth ) * mNumSamples ) / int( mDataWidth ) : 0; }

  inline
  int MarkerChannelTop( int ch )
    { return mMarkerChannels != 0 ? mMarkerHeight * ch + cAxisWidth : 0; }
  inline
  int MarkerChannelBottom( int ch )
    { return mMarkerChannels != 0 ? MarkerChannelTop( ch ) + mMarkerHeight : 0; }

  inline
  int GroupTop( int g )
    { return MarkerChannelBottom( mMarkerChannels - 1 ) + cAxisWidth
             + ( mNumDisplayGroups ? ( g * mDataHeight ) / int( mNumDisplayGroups ) : 0 ); }
  inline
  int GroupBottom( int g )
    { return GroupTop( g + 1 ); }

  inline
  int ChannelTop( int ch )
    { return GroupTop( ChannelToGroup( ch ) ); }
  inline
  int ChannelBottom( int ch )
    { return GroupBottom( ChannelToGroup( ch ) ); }

  inline
  int ChannelToGroup( int ch )
    { return ch / mChannelGroupSize; }

  inline
  float NormData( size_t i, size_t j )
    { return ( mData( i, j ) - mMinValue ) / ( mMaxValue - mMinValue ); }

  inline
  RGBColor ChannelColor( int ch )
    { return mChannelColors[ ch % mChannelColors.size() ]; }

 private:
  eDisplayMode  mDisplayMode;
  bool          mShowCursor,
                mWrapAround,
                mTimeLabels,
                mShowNumericValues,
                mShowBaselines,
                mShowChannelLabels,
                mShowValueUnit,
                mDisplayColors,
                mInverted;
  int           mNumSamples,
                mSampleCursor,
                mNumDisplayGroups,
                mMaxDisplayGroups,
                mNumDisplayChannels,
                mTopGroup,
                mChannelGroupSize,
                mMarkerChannels,
                mNumericValueWidth;
  float         mMinValue,
                mMaxValue;
  std::string   mSampleUnit,
                mValueUnit,
                mChannelUnit;
  float         mSampleOffset,
                mUnitsPerSample,
                mUnitsPerValue,
                mUnitsPerChannel;
  RGBColor      mAxisColor;
  ColorList     mChannelColors;
  std::vector<std::string> mChannelNameCache;
  LabelList     mChannelLabels,
                mXAxisMarkers;
  GenericSignal mData;
  OSMutex       mDataLock;

 private:
  struct PaintInfo;
  void SetupPainting( PaintInfo&, const void* );
  void ClearBackground( const PaintInfo& );
  void DrawSignalPolyline( const PaintInfo& );
  void DrawSignalField2d( const PaintInfo& );
  void DrawMarkerChannels( const PaintInfo& );
  void DrawCursor( const PaintInfo& );
  void DrawXTicks( const PaintInfo& );
  void DrawYLabels( const PaintInfo&, bool inDrawTicks );
  void DrawNumericValues( const PaintInfo& p );
  void DrawAxes( const PaintInfo& );
  void DrawMarkers( const PaintInfo& );
  void DrawChannelLabels( const PaintInfo& );
  void DrawValueUnit( const PaintInfo& );
  void CleanupPainting( PaintInfo& );

  // Qt graphics.
  QFont          AxisFont();
  QFont          MonoFont();
  QPaintDevice*  mTargetDC;
  QRect          mDisplayRect,
                 mDataRect,
                 mCursorRect;
  QRegion        mDisplayRgn;
  QPoint*        mpSignalPoints;

  struct PaintInfo
  {
    QPainter* painter;
    const QRegion* updateRgn;
    QColor    backgroundColor,
              cursorColor,
              axisColor,
              markerColor,
              labelColor;
    QBrush    backgroundBrush,
              cursorBrush,
              axisBrush,
              markerBrush;
    QFont     labelFont,
              monoFont;
    QPen      baselinePen;
    int       cursorWidth,
              markerWidth,
              axisY;
    std::vector<QPen>   signalPens;
    std::vector<QBrush> signalBrushes;
  };
};

#endif // SIGNAL_DISPLAY_H
