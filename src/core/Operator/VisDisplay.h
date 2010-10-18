////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef VIS_DISPLAY_H
#define VIS_DISPLAY_H

#include <vcl.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "Color.h"
#include "Label.h"
#include "GenericSignal.h"
#include "DisplayFilter.h"
#include "SignalDisplay.h"

#include "GenericVisualization.h"

class VisDisplay
{
 public:
  typedef uint8 IDType;
  // This is the outside interface of the VisDisplay class hierarchy.
  static void HandleMessage( const VisSignal& );
  static void HandleMessage( const VisSignalProperties& );
  static void HandleMessage( const VisMemo& );
  static void HandleMessage( const VisBitmap& );
  static void HandleMessage( const VisCfg& );
  static void Clear() { VisDisplayBase::Clear(); }
  class TVisForm;

 private:
  class VisDisplayBase
  {
   protected:
    VisDisplayBase( const std::string& sourceID );
   public:
    virtual ~VisDisplayBase();

    bool Visible() { return mpForm ? mpForm->Visible : false; }
    void SetVisible( bool v ) { if( mpForm ) mpForm->Visible = v; }

    static void Clear() { Visuals().Clear(); }
    static void HandleMessage( const VisCfg& );
    static void HandleMessage( const VisSignalProperties& );

   protected:
    virtual void Restore();
    virtual void Save() const;

    void __fastcall FormSizeMove( TObject* );

   protected:
    std::string mSourceID;
    TVisForm*   mpForm;
   private:
    std::string mTitle;

   protected:
    // sourceID->display instance
    typedef std::map< std::string, VisDisplayBase* > VisContainerBase;
    static class VisContainer : public VisContainerBase
    {
     public:
      ~VisContainer() { Clear(); }
      void Clear();
    }& Visuals();

   public:
    enum ConfigState   // Possible states of properties ("configs").
    {
      Default = 0,     // May be overridden by a message or by user settings.
      OnceUserDefined, // A previous user setting.
      UserDefined,     // Set by the user, may be overridden by a message.
      MessageDefined,  // Set by a message.
    };

   protected:
    // configID->value
    typedef std::map< IDType, std::string > ConfigSettingsBase;
    class ConfigSettings : public ConfigSettingsBase
    {
      public:
        template<typename T> bool Get( IDType id, T& t, ConfigState minState = Default );
        template<typename T> bool Put( IDType id, const T& t, ConfigState state );
        ConfigState& State( IDType id ) { return mStates[ id ]; }

      private:
        std::map< IDType, ConfigState > mStates;
    };

    // sourceID->config information
    class ConfigContainer : public std::map< std::string, ConfigSettings >
    {
      public:
        ConfigContainer()  { Restore(); }
        ~ConfigContainer() { Save(); }
        void Save();
        void Restore();
    };
    static ConfigContainer& Visconfigs();
    virtual void SetConfig( ConfigSettings& );
  };

  // A VCL form with a WM_EXISIZEMOVE handler.
  class TVisForm : public TForm
  {
   public:
    HRGN         mUpdateRgn;
    TNotifyEvent OnSizeMove;

    TVisForm()
      : TForm( ( TComponent* )NULL, 1 ),
        mUpdateRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ),
        OnSizeMove( NULL )
        {}
    __fastcall ~TVisForm()
      { ::DeleteObject( mUpdateRgn ); }
    // There is no inheritance for Message Maps.
    BEGIN_MESSAGE_MAP
      VCL_MESSAGE_HANDLER( WM_PAINT, TWMPaint, WMPaint )
      VCL_MESSAGE_HANDLER( WM_EXITSIZEMOVE, TMessage, WMExitSizeMove )
    END_MESSAGE_MAP( TForm )

   protected:
    void __fastcall WMExitSizeMove( TMessage& )
      { if( OnSizeMove ) OnSizeMove( this ); }
    // Obtain the window's update region before BeginPaint() in the VCL
    // paint handler destroys (validates) it.
    void __fastcall WMPaint( TWMPaint& Message )
    {
      ::GetUpdateRgn( Handle, mUpdateRgn, false );
      PaintHandler( Message );
    }
  };


 private:
  class Graph : public VisDisplayBase
  {
    enum
    {
      maxUserScaling = 4, // The maximum number of scaling steps a user
                          // can take from the default.
    };

   public:
    Graph( const std::string& sourceID );
    virtual ~Graph();
    static void HandleMessage( const VisSignal& );
    void InstanceHandleMessage( const VisSignal& );

   protected:
    virtual void SetConfig( VisDisplayBase::ConfigSettings& );
    virtual void Restore();
    virtual void Save() const;

    // User interaction.
   private:
    typedef void ( Graph::*MenuAction )( size_t );
    typedef bool ( Graph::*MenuStateGetter )( size_t ) const;
    struct MenuItemEntry
    {
      // The typedefs declare pointers to class instance member functions.
      MenuAction       mAction;
      MenuStateGetter  mGetEnabled,
                       mGetChecked;
      const char*      mCaption;
    };
    static const char cSubmenuSeparator;
    static struct MenuItemEntry sMenuItems[];
    std::vector<TMenuItem*> mMenuItems;
    void BuildContextMenu();

   public:
    void EnlargeSignal( size_t );
    bool EnlargeSignal_Enabled( size_t ) const;

    void ReduceSignal( size_t );
    bool ReduceSignal_Enabled( size_t ) const;

    void FewerSamples( size_t );
    bool FewerSamples_Enabled( size_t ) const;

    void MoreSamples( size_t );
    bool MoreSamples_Enabled( size_t ) const;

    void MoreChannels( size_t );
    bool MoreChannels_Enabled( size_t ) const;

    void FewerChannels( size_t );
    bool FewerChannels_Enabled( size_t ) const;

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

    void InvertDisplay( size_t );
    bool InvertDisplay_Checked( size_t ) const;

    void ChooseColors( size_t );
    bool ChooseColors_Enabled( size_t ) const;

    // Members related to display filter settings.
    double FilterUnitToValue( const std::string& ) const;
    double FilterItemToValue( size_t ) const;
    bool Filter_Enabled( size_t ) const;

    void SetHP( size_t );
    bool SetHP_Checked( size_t ) const;

    void SetLP( size_t );
    bool SetLP_Checked( size_t ) const;

    void SetNotch( size_t );
    bool SetNotch_Checked( size_t ) const;

   private:
    int           mNumChannels,
                  mSignalElements,
                  mUserScaling,
                  mUserZoom;
    float         mDisplaySamples;
    DisplayFilter mDisplayFilter;
    SignalDisplay mDisplay;

   // VCL/Win32 Graphics details.
   private:
    void SyncDisplay();
    void __fastcall FormPaint( TObject* );
    void __fastcall FormSizeMove( TObject* );
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
        VCL_MESSAGE_HANDLER( WM_EXITSIZEMOVE, TMessage, WMExitSizeMove )
        VCL_MESSAGE_HANDLER( WM_ERASEBKGND, TWMEraseBkgnd, WMEraseBkgnd )
      END_MESSAGE_MAP( TForm )
    };
  };

  class Memo : public VisDisplayBase
  {
   public:
    Memo( const std::string& sourceID );
    virtual ~Memo();
    static void HandleMessage( const VisMemo& );
    void InstanceHandleMessage( const VisMemo& );

   protected:
    virtual void SetConfig( VisDisplayBase::ConfigSettings& );
    virtual void Restore();
    virtual void Save() const;

   private:
    TMemo* mpMemo;
    int    mNumLines;
  };

  class Bitmap : public VisDisplayBase
  {
   public:
    Bitmap( const std::string& sourceID );
    virtual ~Bitmap();
    static void HandleMessage( const VisBitmap& );
    void InstanceHandleMessage( const VisBitmap& );

   protected:
    virtual void SetConfig( VisDisplayBase::ConfigSettings& );
    virtual void Restore();
    virtual void Save() const;

   private:
    BitmapImage mImageBuffer;
   // VCL/Win32 Graphics details.
    void __fastcall FormPaint( TObject* );
    class TVisBitmapForm : public TVisForm
    {
     public:
      TVisBitmapForm() {}
      // To avoid flicker and save memory bandwidth, use a WM_ERASEBKGND
      // handler that does not do anything.
      void __fastcall WMEraseBkgnd( TWMEraseBkgnd& ) {}
      // There is no inheritance for Message Maps.
      BEGIN_MESSAGE_MAP
        VCL_MESSAGE_HANDLER( WM_PAINT, TWMPaint, WMPaint )
        VCL_MESSAGE_HANDLER( WM_ERASEBKGND, TWMEraseBkgnd, WMEraseBkgnd )
        VCL_MESSAGE_HANDLER( WM_EXITSIZEMOVE, TMessage, WMExitSizeMove )
      END_MESSAGE_MAP( TForm )
    };
  };
};

template<typename T>
bool
VisDisplay::VisDisplayBase::ConfigSettings::Get( IDType id, T& t, ConfigState minState )
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
VisDisplay::VisDisplayBase::ConfigSettings::Put( IDType id, const T& t, ConfigState state )
{
  if( State( id ) > state )
    return false;
  State( id ) = state;
  stringstream os;
  os << t;
  ( *this )[ id ] = os.str();
  return !os.fail();
}

#endif // VIS_DISPLAY_H
