/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UOperatorCfgH
#define UOperatorCfgH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <map>
#include <string>

#include "ParamDisplay.h"

#define LABELS_OFFSETX          30
#define LABELS_OFFSETY          50

class ParamList;
class Param;
class PREFERENCES;
//---------------------------------------------------------------------------
class TfConfig : public TForm
{
__published:  // IDE-managed Components
        TTabControl *CfgTabControl;
        TButton *bLoadParameters;
        TButton *bSaveParameters;
        TOpenDialog *LoadDialog;
        TSaveDialog *SaveDialog;
        TButton *bConfigureSaveFilter;
        TButton *bConfigureLoadFilter;
        TScrollBox *ScrollBox;
  TButton *bHelp;
        void __fastcall CfgTabControlChange(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall CfgTabControlChanging(TObject *Sender, bool &AllowChange);
        void __fastcall bSaveParametersClick(TObject *Sender);
        void __fastcall bLoadParametersClick(TObject *Sender);
        void __fastcall bConfigureSaveFilterClick(TObject *Sender);
        void __fastcall bConfigureLoadFilterClick(TObject *Sender);
  void __fastcall bHelpClick(TObject *Sender);

public:  // User declarations
        __fastcall TfConfig(TComponent* Owner);
        __fastcall ~TfConfig();
        int     Initialize( ParamList*, PREFERENCES* );
        void    DeleteAllTabs();
        void    RenderParameters( const std::string& section );
        void    RenderParameter( Param* );
        void    UpdateParameters();

        typedef void ( *Notification )();
        void          OnParameterChange( Notification n )
                      { mOnParameterChange = n; }
        Notification  OnParameterChange() const
                      { return mOnParameterChange; }
        void          ParameterChange()
                      { if( mOnParameterChange ) mOnParameterChange(); }

private:
        bool    LoadParameters( const AnsiString& inFileName );
        void    DisposeControls();

        Notification mOnParameterChange;
        ParamList*   paramlist;
        PREFERENCES* preferences;
        typedef std::set<TControl*> ControlContainer;
        ControlContainer mControls;
        typedef std::map<std::string, ParamDisplay> DisplayContainer;
        DisplayContainer mParamDisplays;
        std::string      mCurTab;
};
//---------------------------------------------------------------------------
extern PACKAGE TfConfig *fConfig;
//---------------------------------------------------------------------------
#endif
