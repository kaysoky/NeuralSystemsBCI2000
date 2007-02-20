//////////////////////////////////////////////////////////////////////
//
// File: ParamDisplay.h
//
// Date: Dec 30, 2004
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A class that handles a single parameter's associated
//       user interface, consisting of a group of VCL user interface
//       elements arranged inside a VCL window control.
//
//       Various types of parameter displays are implemented as sub-
//       classes transparent to user code.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#ifndef ParamDisplayH
#define ParamDisplayH

#include <vcl.h>
#include <Comctrls.hpp>
#include <map>
#include <set>
#include <string>

#include "UParameter.h"
class ParsedComment;

class ParamDisplay  // This class is the interface to the outside world.
{
  public:
    ParamDisplay();
    ParamDisplay( const PARAM&, TWinControl* );
    ParamDisplay( const ParamDisplay& );
    ~ParamDisplay();
    const ParamDisplay& operator=( const ParamDisplay& );

    void SetTop( int top );
    void SetLeft( int left );
    int  GetBottom();
    int  GetRight();
    void Hide();
    void Show();

    void WriteValuesTo( PARAM& ) const;
    void ReadValuesFrom( const PARAM& );
    bool Modified() const;

    enum
    {
      LABELS_OFFSETX =    0,
      LABELS_OFFSETY =    18,
      COMMENT_OFFSETX =   140,
      COMMENT_OFFSETY =   0,
      VALUE_OFFSETX =     COMMENT_OFFSETX,
      VALUE_OFFSETY =     15,
      VALUE_WIDTH =       220,
      BUTTON_WIDTH =      ( VALUE_WIDTH - 20 ) / 3,
      BUTTON_HEIGHT =     21,
      BUTTON_SPACINGX =   ( VALUE_WIDTH - 3 * BUTTON_WIDTH ) / 2,
      USERLEVEL_OFFSETX = VALUE_OFFSETX + 260,
      USERLEVEL_WIDTH =   70,
      USERLEVEL_OFFSETY = 12,
      USERLEVEL_HEIGHT =  30,
    };

  private:
    class DisplayBase;
    DisplayBase* mpDisplay;
    // Reference counting for DisplayBase pointers:
    static std::map<DisplayBase*, int> RefCount;

  // This private class hierarchy, descending from DisplayBase,
  // contains the actual implementations for various
  // kinds of parameters.
  private:
    class DisplayBase
    {
      protected:
        DisplayBase( const ParsedComment&, TWinControl* );

      public:
        virtual ~DisplayBase();

        void SetTop( int );
        void SetLeft( int );
        int  GetBottom();
        int  GetRight();
        void Show();
        void Hide();

        virtual void WriteValuesTo( PARAM& ) const;
        virtual void ReadValuesFrom( const PARAM& );
        bool Modified() const { return mModified; }

      protected:
        void AddControl( TControl* c )                { mControls.insert( c ); }
        void __fastcall OnContentChange( TObject* = NULL ) { mModified = true; }

      private:
        typedef std::set<TControl*>        ControlContainer;
        typedef ControlContainer::iterator ControlIterator;
        ControlContainer                   mControls;

        bool       mModified;
        int        mTop,
                   mLeft;
        TTrackBar* mpUserLevel;
    };

    // This is the base class for all displays where there is a separate label
    // holding the "comment" part of the parameter.
    class SeparateComment : public DisplayBase
    {
      protected:
        SeparateComment( const ParsedComment&, TWinControl* );
    };

    // This is the base class for all displays that contain an edit field
    // holding parameter values.
    class SingleEntryEdit : public SeparateComment
    {
      public:
        SingleEntryEdit( const ParsedComment&, TWinControl* );
        virtual void WriteValuesTo( PARAM& ) const;
        virtual void ReadValuesFrom( const PARAM& );
      private:
        void __fastcall OnEditChange( TObject* );
      protected:
        TEdit*  mpEdit;
    };

    // This is the base class for all displays that contain a button beside
    // the edit field.
    class SingleEntryButton : public SingleEntryEdit
    {
      public:
        SingleEntryButton( const ParsedComment&, TWinControl* );
      private:
        void __fastcall OnButtonClick( TObject* );
        virtual void ButtonClick() = 0;
      protected:
        std::string mComment;
    };

    class SingleEntryInputFile : public SingleEntryButton
    {
      public:
        SingleEntryInputFile( const ParsedComment&, TWinControl* );
      private:
        virtual void ButtonClick();
    };

    class SingleEntryOutputFile : public SingleEntryButton
    {
      public:
        SingleEntryOutputFile( const ParsedComment&, TWinControl* );
      private:
        virtual void ButtonClick();
    };

    class SingleEntryDirectory : public SingleEntryButton
    {
      public:
        SingleEntryDirectory( const ParsedComment&, TWinControl* );
      private:
        virtual void ButtonClick();
    };

    class SingleEntryColor : public SingleEntryButton
    {
      public:
        SingleEntryColor( const ParsedComment&, TWinControl* );
      private:
        virtual void ButtonClick();
    };

    class List : public SingleEntryEdit
    {
      public:
        List( const ParsedComment&, TWinControl* );
        virtual void WriteValuesTo( PARAM& ) const;
        virtual void ReadValuesFrom( const PARAM& );
    };

    class Matrix : public SeparateComment
    {
      public:
        Matrix( const ParsedComment&, TWinControl* );
        virtual void WriteValuesTo( PARAM& ) const;
        virtual void ReadValuesFrom( const PARAM& );
      private:
        void __fastcall OnEditButtonClick( TObject* );
        void __fastcall OnLoadButtonClick( TObject* );
        void __fastcall OnSaveButtonClick( TObject* );
      
        bool   mMatrixWindowOpen;
        PARAM  mParam;
    };
  
    class SingleEntryEnum : public SeparateComment
    {
      public:
        SingleEntryEnum( const ParsedComment&, TWinControl* );
        virtual void WriteValuesTo( PARAM& ) const;
        virtual void ReadValuesFrom( const PARAM& );
      private:
        TComboBox* mComboBox;
        int        mIndexBase;
    };

    class SingleEntryBoolean : public DisplayBase
    {
      public:
        SingleEntryBoolean( const ParsedComment&, TWinControl* );
        virtual void WriteValuesTo( PARAM& ) const;
        virtual void ReadValuesFrom( const PARAM& );
      private:
        TCheckBox* mCheckBox;
    };
};

#endif // ParamDisplayH
