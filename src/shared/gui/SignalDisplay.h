////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A class that draws GenericSignal data into a given window,
//          and maintains a context menu.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SIGNAL_DISPLAY_H
#define SIGNAL_DISPLAY_H

#include "GUI.h"
#include "GenericSignal.h"
#include "Color.h"
#include "Label.h"
#include <set>
#include <vector>

#include <windows.h>

class SignalDisplay
{
 private:
  static const int cNumSamplesDefault = 128,
                   cMinValueDefault = - ( 1 << 15 ),
                   cMaxValueDefault = ( 1 << 16 ) - 1,

                   cChannelBase = 1, // displayed number of first channel
                   cSampleBase = 0,  // displayed number of first sample

                   cLabelWidth = 25,
                   cAxisWidth = 2,
                   cTickWidth = cAxisWidth,
                   cTickLength = 4,
                   cInitialMaxDisplayGroups = 32;

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
  SignalDisplay& Paint( void* RegionHandle = NULL );

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

  SignalDisplay& SetBaselinesVisible( bool b )
                 { mShowBaselines = b; return Invalidate(); }
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
  RECT mDataRect;

  void SyncLabelWidth();
  void SyncGraphics();
  void AdaptTo( const GenericSignal& );

  int SampleLeft( int s )
    { return mLabelWidth + ( mNumSamples ? ( s * mDataWidth ) / int( mNumSamples ) : 0 ); }
  int SampleRight( int s )
    { return SampleLeft( s + 1 ); }

  int MarkerChannelTop( int ch )
    { return mMarkerChannels != 0 ? mMarkerHeight * ch + cAxisWidth : 0; }
  int MarkerChannelBottom( int ch )
    { return mMarkerChannels != 0 ? MarkerChannelTop( ch ) + mMarkerHeight : 0; }

  int GroupTop( int g )
    { return MarkerChannelBottom( mMarkerChannels - 1 ) + cAxisWidth
             + ( mNumDisplayGroups ? ( g * mDataHeight ) / int( mNumDisplayGroups ) : 0 ); }
  int GroupBottom( int g )
    { return GroupTop( g + 1 ); }

  int ChannelTop( int ch )
    { return GroupTop( ChannelToGroup( ch ) ); }
  int ChannelBottom( int ch )
    { return GroupBottom( ChannelToGroup( ch ) ); }

  int ChannelToGroup( int ch )
    { return ch / mChannelGroupSize; }

  float NormData( size_t i, size_t j )
    { return ( mData( i, j ) - mMinValue ) / ( mMaxValue - mMinValue ); }

  RGBColor ChannelColor( int ch )
    { return mChannelColors[ ch % mChannelColors.size() ]; }

 private:
  eDisplayMode  mDisplayMode;
  bool          mShowCursor,
                mWrapAround,
                mTimeLabels,
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
                mMarkerChannels;
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

 // Win32 Graphics details.
 private:
  HGDIOBJ            AxisFont();

  struct BunchOfHandles
  {
    BunchOfHandles( HDC inDC )
      : targetWindow( ::WindowFromDC( inDC ) ),
        targetDC( inDC )
      {}
    ~BunchOfHandles()
      {}

    HDC targetWindow,
        targetDC;
  }*                 mpHandles;
  RECT               mDisplayRect;
  HRGN               mDisplayRgn,
                     mRedrawRgn;
  class PointBuf
  {
   public:
    PointBuf()
      : p( NULL ), s( 0 )
      {}
    ~PointBuf()
      { delete[] p; }
    POINT& operator[]( size_t i )
      { return p[ i ]; }
    operator const POINT* () const
      { return p; }
    void resize( size_t size )
      { if( size > s ) { s = size; delete[] p; p = new POINT[ s ]; } }
   private:
    POINT* p;
    size_t s;
  } mSignalPoints;
};

#endif // SIGNAL_DISPLAY_H
