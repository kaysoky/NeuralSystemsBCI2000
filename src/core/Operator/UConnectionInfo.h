/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UConnectionInfoH
#define UConnectionInfoH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfConnectionInfo : public TForm
{
__published:  // IDE-managed Components
        TBevel *Bevel2;
        TLabel *tNumMessagesSent1;
        TLabel *Label6;
        TLabel *tNumMessagesSent2;
        TLabel *tNumMessagesSent3;
        TLabel *Label9;
        TLabel *Label10;
        TLabel *Label11;
        TLabel *tNumParametersSent1;
        TLabel *tNumParametersSent2;
        TLabel *tNumParametersSent3;
        TLabel *tNumStatesSent1;
        TLabel *tNumStatesSent2;
        TLabel *tNumStatesSent3;
        TLabel *Label3;
        TLabel *tNumStateVecsSent1;
        TLabel *tNumStateVecsSent2;
        TLabel *tNumStateVecsSent3;
        TBevel *Bevel1;
        TLabel *tNumMessagesRecv1;
        TLabel *Label1;
        TLabel *tNumMessagesRecv2;
        TLabel *tNumMessagesRecv3;
        TLabel *Label2;
        TLabel *Label4;
        TLabel *Label5;
        TLabel *tNumParametersRecv1;
        TLabel *tNumParametersRecv2;
        TLabel *tNumParametersRecv3;
        TLabel *tNumStatesRecv1;
        TLabel *tNumStatesRecv2;
        TLabel *tNumStatesRecv3;
        TLabel *Label18;
        TLabel *tNumDataRecv1;
        TLabel *tNumDataRecv2;
        TLabel *tNumDataRecv3;
    TLabel *tEEGSourceConnected;
        TLabel *tSigProcConnected;
    TLabel *tAppConnected;
    TCheckBox *cEEGSourceConnected;
        TCheckBox *cSigProcConnected;
    TCheckBox *cAppConnected;
private:  // User declarations
public:   // User declarations
        __fastcall TfConnectionInfo(TComponent* Owner);
        void UpdateDisplay( const class SYSSTATUS& );
};
//---------------------------------------------------------------------------
extern PACKAGE TfConnectionInfo *fConnectionInfo;
//---------------------------------------------------------------------------
#endif
