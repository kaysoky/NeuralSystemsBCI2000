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

      private:
        int        mTop,
                   mLeft;
        TTrackBar* mpUserLevel;

      protected:
        std::set<TControl*> mControls;

    };

    class SeparateComment : public DisplayBase
    {
      protected:
        SeparateComment( const ParsedComment&, TWinControl* );
    };

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

    class SingleEntryFile : public SingleEntryEdit
    {
      public:
        SingleEntryFile( const ParsedComment&, TWinControl* );
      private:
        void __fastcall OnChooseFileClick( TObject* );
        virtual void ChooseFile() = 0;
      protected:
        std::string mComment;
    };

    class SingleEntryInputFile : public SingleEntryFile
    {
      public:
        SingleEntryInputFile( const ParsedComment&, TWinControl* );
      private:
        virtual void ChooseFile();
    };

    class SingleEntryOutputFile : public SingleEntryFile
    {
      public:
        SingleEntryOutputFile( const ParsedComment&, TWinControl* );
      private:
        virtual void ChooseFile();
    };

    class SingleEntryDirectory : public SingleEntryFile
    {
      public:
        SingleEntryDirectory( const ParsedComment&, TWinControl* );
      private:
        virtual void ChooseFile();
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