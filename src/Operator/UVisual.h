////////////////////////////////////////////////////////////////////////////////
//
// File:    UVisual.h
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
////////////////////////////////////////////////////////////////////////////////
#ifndef UVisualH
#define UVisualH

#include <vcl.h>
#include <iostream>
#include <map>
#include <algorithm>
#include "UGenericSignal.h"

#include "UGenericVisualization.h"

class VISUAL
{
 public:
  // This is the entire public interface of VISUAL.
  typedef BYTE id_type;
  static bool HandleMessage( std::istream& );
  static void clear() { VISUAL_BASE::clear(); }

 private:
  class VISUAL_BASE
  {
   protected:
    VISUAL_BASE( id_type sourceID );
   public:
    virtual ~VISUAL_BASE();
    static void clear() { visuals.clear(); }
    static bool HandleMessage( std::istream& );


   protected:
    virtual void Restore();
    virtual void Save() const;

   protected:
    id_type sourceID;
    TForm* form;
   private:
    std::string title;

   protected:
    typedef std::map< id_type, VISUAL_BASE* > vis_container_base;
    static class vis_container : public vis_container_base
    {
     public:
      ~vis_container() { clear(); }
      void clear();
    } visuals;

    // The following container mess is needed because the protocol
    // does not factor out properties and types of
    // visualizations properly. Previously, the role of these objects
    // was played by the registry, with additional potential for trouble.
    // The protocol should be re-defined and this stuff should go away.

    // configID->value
    typedef std::map< id_type, std::string > config_settings;

    // sourceID->config information
    typedef std::map< id_type, config_settings > config_container;
    static config_container visconfigs;
    virtual void SetConfig( config_settings& );
  };
  
 private:
  class VISUAL_GRAPH : public VISUAL_BASE
  {
   private:
    static const numSamplesDefault = 128,
                 minValueDefault = - 1 << 15,
                 maxValueDefault = 1 << 16 - 1;
    enum
    {
      channelBase = 1, // displayed number of first channel
      sampleBase = 0,  // displayed number of first sample
      labelWidth = 20,
      maxDisplayGroups = 16,
    };

   protected:
    enum DisplayMode
    {
      polyline,
      colorfield,
    } displayMode;
    
   public:
    VISUAL_GRAPH( id_type sourceID );
    virtual ~VISUAL_GRAPH();
    static bool HandleMessage( std::istream& );
    bool InstanceHandleMessage( std::istream& );

   protected:
    virtual void SetConfig( VISUAL_BASE::config_settings& );
    virtual void Restore();
    virtual void Save() const;

   private:
    void SetBottomGroup( int );
    void SetDisplayMode( DisplayMode );

   // Functions that centralize sample/channel -> pixel conversion in painting
   // and invalidating contexts.
   private:
    int dataWidth, dataHeight;
    RECT dataRect;
    void SyncGraphics()
    {
      dataRect = TRect( labelWidth, 0, form->ClientWidth, form->ClientHeight );
      dataWidth = std::max<int>( 0, dataRect.right - dataRect.left );
      dataHeight = std::max<int>( 0, dataRect.bottom - dataRect.top - labelWidth );
    }

    int SampleLeft( int s )
    { return labelWidth + ( s * dataWidth ) / ( int )numSamples; }
    int SampleRight( int s )
    { return SampleLeft( s + 1 ); }

    int GroupTop( int g )
    { return GroupBottom( g + 1 ); }
    int GroupBottom( int g )
    { return dataHeight - ( g * dataHeight ) / ( int )numDisplayGroups; }

    int ChannelTop( int ch )
    { return GroupTop( ChannelToGroup( ch ) ); }
    int ChannelBottom( int ch )
    { return GroupBottom( ChannelToGroup( ch ) ); }

    size_t ChannelToGroup( int ch )
    { return ch / channelGroupSize; }

    float NormData( size_t i, size_t j )
    { return ( data( i, j ) - minValue ) / ( maxValue - minValue ); }

   private:
    bool   showCursor,
           wrapAround,
           showBaselines;
    size_t numSamples,
           sampleCursor,
           numDisplayGroups,
           numDisplayChannels,
           bottomGroup,
           channelGroupSize;
    float  minValue,
           maxValue;
    GenericSignal data;

   // VCL/Win32 Graphics details.
   private:
    HRGN   redrawRgn;
    Graphics::TBitmap* offscreenBitmap;
    class PointBuf
    {
     public:
      PointBuf() : p( NULL ), s( 0 ) {}
      ~PointBuf() { delete[] p; }
      POINT& operator[]( size_t i ) { return p[ i ]; }
      operator const POINT* () { return p; } const
      void resize( size_t size )
      { if( size > s ) { s = size; delete[] p; p = new POINT[ s ]; } }
     private:
      POINT* p;
      size_t s;
    } signalPoints;
    void __fastcall FormResize( TObject* );
    void __fastcall FormPaint( TObject* );
    void __fastcall FormKeyUp( TObject*, WORD&, TShiftState );
    class TVisForm : public TForm
    {
     public:
      HRGN    updateRgn;

      TVisForm()
      : TForm( ( TComponent* )NULL, 1 ),
        updateRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ) {}
      __fastcall ~TVisForm()
      {
        ::DeleteObject( updateRgn );
      }
      // To avoid flicker and save memory bandwidth, use a WM_ERASEBKGND
      // handler that does not do anything.
      void __fastcall WMEraseBkgnd( TWMEraseBkgnd& ) {}
      // Obtain the window's update region before BeginPaint() in the VCL
      // paint handler destroys (validates) it.
      void __fastcall WMPaint( TWMPaint& Message )
      {
        ::GetUpdateRgn( Handle, updateRgn, false );
        PaintHandler( Message );
      }
      BEGIN_MESSAGE_MAP
        VCL_MESSAGE_HANDLER( WM_PAINT, TWMPaint, WMPaint )
        VCL_MESSAGE_HANDLER( WM_ERASEBKGND, TWMEraseBkgnd, WMEraseBkgnd )
      END_MESSAGE_MAP( TForm )
    };
  };

  class VISUAL_MEMO : public VISUAL_BASE
  {
   public:
    VISUAL_MEMO( id_type sourceID );
    virtual ~VISUAL_MEMO();
    static bool HandleMessage( std::istream& );
    bool InstanceHandleMessage( std::istream& );

   protected:
    virtual void SetConfig( VISUAL_BASE::config_settings& );
    virtual void Restore();
    virtual void Save() const;

   private:
    TMemo* memo;
    int numLines;
  };

};

#endif // UVisualH
