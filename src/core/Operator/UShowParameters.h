/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UShowParametersH
#define UShowParametersH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#include "Param.h"
#include "ParamList.h"
#include <CheckLst.hpp>

//---------------------------------------------------------------------------
class TfShowParameters : public TForm
{
__published:  // IDE-managed Components
        TCheckListBox *ParameterListBox;
        void __fastcall FormShow(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:  // User declarations
public:   // User declarations
        __fastcall      TfShowParameters(TComponent* Owner);
        int             GetFilterStatus(Param *param, int filtertype);
        void            SetFilterStatus(Param *param, int filtertype, int);
#if 0
        void            UpdateParameterTags(ParamList *paramlist, int filtertype);
#endif
        ParamList       *parameterlist;
        int             filtertype;
};
//---------------------------------------------------------------------------
extern PACKAGE TfShowParameters *fShowParameters;
//---------------------------------------------------------------------------
#endif
