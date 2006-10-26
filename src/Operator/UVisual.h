////////////////////////////////////////////////////////////////////////////////
// $Id$
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
//          Added the field2d display type for a graph to support
//          FFT data.
//
//          Nov 20, 2003, jm:
//          Added context menu to ease interactive configuration of graph
//          display properties.
//          Introduced colorized y axis ticks.
//
//          Dec 12, 2003, jm:
//          Introduced bookkeeping for configuration settings.
//
// $Log$
// Revision 1.18  2006/10/26 17:03:02  mellinger
// Added display filters.
//
// Revision 1.17  2006/03/15 14:52:58  mellinger
// Compatibility with BCB 2006.
//
// Revision 1.16  2006/02/01 11:01:09  mellinger
// Changed anonymous enum to constant definitions.
//
// Revision 1.15  2006/01/31 15:22:59  mellinger
// Fixed list of #includes; introduced CVS Id and Log.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UVisualH
#define UVisualH

#include <vcl.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "Color.h"
#include "Label.h"
#include "UGenericSignal.h"
#include "DisplayFilter.h"

#include "UGenericVisualization.h"

class VISUAL
{
 public:
  // This is the entire public interface of VISUAL.
  typedef BYTE id_type;
  static void HandleMessage( const VisSignal& );
  static void HandleMessage( const VisCfg& );
  static void HandleMessage( const VisMemo& );
  static void clear() { VisualBase::clear(); }
  class TVisForm;

 private:
  class VisualBase
  {
   protected:
    VisualBase( id_type sourceID );
   public:
    virtual ~VisualBase();
    static void clear() { Visuals().clear(); }
    static void HandleMessage( const VisCfg& );


   protected:
    virtual void Restore();
    virtual void Save() const;

   private:
    void __fastcall FormMove( TObject* );
    void __fastcall FormResize( TObject* );

   protected:
    id_type sourceID;
    TVisForm* form;
   private:
    std::string title;

   protected:
    typedef std::map< id_type, VisualBase* > vis_container_base;
    static class vis_container : public vis_container_base
    {
     public:
      ~vis_container() { clear(); }
      void clear();
    }& Visuals();

   public:
    enum config_state  // Possible states of properties ("configs").
    {
      Default = 0,     // May be overridden by a message or by user settings.
      OnceUserDefined, // A previous user setting. Will become user defined if modified
                       // by either MessageDefined or UserDefined information.
      MessageDefined,  // Set by a message, user may override.
      UserDefined,     // Set by the user, no override by a message.
    };

   protected:
    // configID->value
    typedef std::map< id_type, std::string > config_settings_base;
    class config_settings : public config_settings_base
    {
      public:
        template<typename T> bool Get( id_type id, T& t, config_state minState = Default );
        template<typename T> bool Put( id_type id, const T& t, config_state state );
        config_state& State( id_type id ) { return mStates[ id ]; }

      private:
        std::map< id_type, config_state > mStates;
    };

    // sourceID->config information
    class config_container : public std::map< id_type, config_settings >
    {
      public:
        config_container()  { Restore(); }
        ~config_container() { Save(); }
        void Save();
        void Restore();
    };
    static config_container& Visconfigs();
    virtual void SetConfig( config_settings& );
  };

  // A VCL form with a WM_MOVE handler.
  class TVisForm : public TForm
  {
   public:
    HRGN         updateRgn;
    TNotifyEvent OnMove;

    TVisForm()
    : TForm( ( TComponent* )NULL, 1 ),
      updateRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ),
      OnMove( NULL ) {}
    __fastcall ~TVisForm()
    {
      ::DeleteObject( updateRgn );
    }
    // There is no inheritance for Message Maps.
    BEGIN_MESSAGE_MAP
      VCL_MESSAGE_HANDLER( WM_PAINT, TWMPaint, WMPaint )
      VCL_MESSAGE_HANDLER( WM_MOVE, TMessage, WMMove )
    END_MESSAGE_MAP( TForm )

   protected:
    void __fastcall WMMove( TMessage& )
    {
      if( OnMove )
        OnMove( this );
    }
    // Obtain the window's update region before BeginPaint() in the VCL
    // paint handler destroys (validates) it.
    void __fastcall WMPaint( TWMPaint& Message )
    {
      ::GetUpdateRgn( Handle, updateRgn, false );
      PaintHandler( Message );
    }
  };


 private:
  class Graph : public VisualBase
  {
   private:
    static const cNumSamplesDefault = 128,
                 cMinValueDefault = - 1 << 15,
                 cMaxValueDefault = 1 << 16 - 1;
    static const RGBColor cChannelColorsDefault[];
    static const int cChannelBase = 1, // displayed number of first channel
                     cSampleBase = 0,  // displayed number of first sample
                     cLabelWidth = 25;
    static const unsigned int cMaxDisplayGroups = 16;
    static const char cSubmenuSeparator;

   protected:
    enum DisplayMode
    {
      polyline = 0,
      field2d,
      /* ... */
      cNumDisplayModes
    } mDisplayMode;

   public:
    Graph( id_type sourceID );
    virtual ~Graph();
    static void HandleMessage( const VisSignal& );
    void InstanceHandleMessage( const VisSignal& );

   protected:
    virtual void SetConfig( VisualBase::config_settings& );
    virtual void Restore();
    virtual void Save() const;

    // User interaction.
   private:
    struct MenuItemEntry
    {
      // The typedefs declare pointers to class instance member functions.
      typedef void ( VISUAL::Graph::*MenuAction )( size_t );
      typedef bool ( VISUAL::Graph::*MenuStateGetter )( size_t );
      MenuAction       mAction;
      MenuStateGetter  mGetEnabled,
                       mGetChecked;
      const char*      mCaption;
    };
    static struct MenuItemEntry sMenuItems[];
    std::vector<TMenuItem*> mMenuItems;
    void BuildContextMenu();

    void EnlargeSignal( size_t );
    bool EnlargeSignal_Enabled( size_t ) const;

    void ReduceSignal( size_t );
    bool ReduceSignal_Enabled( size_t ) const;
    enum { maxUserScaling = 4 }; // The maximum number of scaling steps a user
                                 // can take from the default.
    int  mUserScaling;

    void MoreChannels( size_t );
    bool MoreChannels_Enabled( size_t ) const;

    void LessChannels( size_t );
    bool LessChannels_Enabled( size_t ) const;

    void ToggleDisplayMode( size_t );

    void ToggleBaselines( size_t );
    bool ToggleBaselines_Enabled( size_t ) const;
    bool ToggleBaselines_Checked( size_t ) const;

    void ToggleValueUnit( size_t );
    bool ToggleValueUnit_Enabled( size_t ) const;
    bool ToggleValueUnit_Checked( size_t ) const;

    void ToggleChannelLabels( size_t );
    bool ToggleChannelLabels_Enabled( size_t ) const;
    bool ToggleChannelLabels_Checked( size_t ) const;

    void ToggleColor( size_t );
    bool ToggleColor_Enabled( size_t ) const;
    bool ToggleColor_Checked( size_t ) const;

    void ChooseColors( size_t );
    bool ChooseColors_Enabled( size_t ) const;

    // Members related to display filter settings.
    static double FilterUnitToValue( const std::string& );
    double FilterItemToValue( size_t ) const;
    bool Filter_Enabled( size_t ) const;

    void SetHP( size_t );
    bool SetHP_Checked( size_t ) const;

    void SetLP( size_t );
    bool SetLP_Checked( size_t ) const;

    void SetNotch( size_t );
    bool SetNotch_Checked( size_t ) const;

    void SetDisplayGroups( int );
    void SetBottomGroup( int );
    void SetDisplayMode( DisplayMode );
    void SetNumSamples( int );

   // Functions that centralize sample/channel -> pixel conversion in painting
   // and invalidating contexts.
   private:
    int  mDataWidth, mDataHeight;
    RECT mDataRect;
    void SyncGraphics();

    int SampleLeft( int s )
    { return cLabelWidth + ( s * mDataWidth ) / ( int )mNumSamples; }
    int SampleRight( int s )
    { return SampleLeft( s + 1 ); }

    int GroupTop( int g )
    { return GroupBottom( g + 1 ); }
    int GroupBottom( int g )
    { return mDataHeight - ( g * mDataHeight ) / ( int )mNumDisplayGroups; }

    int ChannelTop( int ch )
    { return GroupTop( ChannelToGroup( ch ) ); }
    int ChannelBottom( int ch )
    { return GroupBottom( ChannelToGroup( ch ) ); }

    size_t ChannelToGroup( int ch )
    { return ch / mChannelGroupSize; }

    float NormData( size_t i, size_t j )
    { return ( mData( i, j ) - mMinValue ) / ( mMaxValue - mMinValue ); }

    RGBColor ChannelColor( int ch )
    { return mChannelColors[ ch % mChannelColors.size() ]; }

   private:
    bool          mShowCursor,
                  mWrapAround,
                  mShowBaselines,
                  mShowChannelLabels,
                  mShowValueUnit,
                  mDisplayColors;
    size_t        mNumSamples,
                  mSampleCursor,
                  mNumDisplayGroups,
                  mNumDisplayChannels,
                  mBottomGroup,
                  mChannelGroupSize;
    float         mMinValue,
                  mMaxValue;
    std::string   mSampleUnit,
                  mValueUnit,
                  mChannelUnit;
    float         mUnitsPerSample,
                  mUnitsPerValue,
                  mUnitsPerChannel;
    Colorlist     mChannelColors;
    Labellist     mChannelLabels,
                  mXAxisMarkers;
    GenericSignal mData;
    DisplayFilter mDisplayFilter;

   // VCL/Win32 Graphics details.
   private:
    HRGN               mRedrawRgn;
    Graphics::TBitmap* mpOffscreenBitmap;
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
    } mSignalPoints;
    void __fastcall FormPaint( TObject* );
    void __fastcall FormKeyUp( TObject*, WORD&, TShiftState );
    void __fastcall PopupMenuPopup( TObject* );
    void __fastcall PopupMenuItemClick( TObject* );
    class TVisGraphForm : public TVisForm
    {
     public:
      TVisGraphForm() {}
      // To avoid flicker and save memory bandwidth, use a WM_ERASEBKGND
      // handler that does not do anything.
      void __fastcall WMEraseBkgnd( TWMEraseBkgnd& ) {}
      // There is no inheritance for Message Maps.
      BEGIN_MESSAGE_MAP
        VCL_MESSAGE_HANDLER( WM_PAINT, TWMPaint, WMPaint )
        VCL_MESSAGE_HANDLER( WM_MOVE, TMessage, WMMove )
        VCL_MESSAGE_HANDLER( WM_ERASEBKGND, TWMEraseBkgnd, WMEraseBkgnd )
      END_MESSAGE_MAP( TForm )
    };
  };

  class Memo : public VisualBase
  {
   public:
    Memo( id_type sourceID );
    virtual ~Memo();
    static void HandleMessage( const VisMemo& );
    void InstanceHandleMessage( const VisMemo& );

   protected:
    virtual void SetConfig( VisualBase::config_settings& );
    virtual void Restore();
    virtual void Save() const;

   private:
    TMemo* mpMemo;
    int    mNumLines;
  };

};

template<typename T>
bool
VISUAL::VisualBase::config_settings::Get( id_type id, T& t, config_state minState )
{
  const_iterator i = find( id );
  if( i == end() )
    return false;
  if( State( id ) < minState )
    return false;
  istringstream is( i->second );
  if( is.str() != "" )
  {
    T value;
    if( is >> value )
      t = value;
  }
  return !is.fail();
}

template<typename T>
bool
VISUAL::VisualBase::config_settings::Put( id_type id, const T& t, config_state state )
{
  if( State( id ) > state )
    return false;
  if( State( id ) == OnceUserDefined )
    State( id ) = UserDefined;
  else
    State( id ) = state;
  stringstream os;
  os << t;
  ( *this )[ id ] = os.str();
  return !os.fail();
}


#endif // UVisualH
