//---------------------------------------------------------------------------

#ifndef UShowParametersH
#define UShowParametersH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#include "UParameter.h"
#include <CheckLst.hpp>

//---------------------------------------------------------------------------
class TfShowParameters : public TForm
{
__published:	// IDE-managed Components
        TCheckListBox *ParameterListBox;
        void __fastcall FormShow(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
public:		// User declarations
        __fastcall      TfShowParameters(TComponent* Owner);
        int             GetFilterStatus(PARAM *param, int filtertype);
        void            SetFilterStatus(PARAM *param, int filtertype, int);
        void            UpdateParameterTags(PARAMLIST *paramlist, int filtertype);
        PARAMLIST       *parameterlist;
        int             filtertype;
};
//---------------------------------------------------------------------------
extern PACKAGE TfShowParameters *fShowParameters;
//---------------------------------------------------------------------------
#endif
