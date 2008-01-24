/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------
#define NSTATES 32
#define NGROUPS 32
#define STATELENGTH 32

#ifndef StateForm1H
#define StateForm1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TUseStateForm : public TForm
{
__published:	// IDE-managed Components
        TLabel *NumStates;
        TEdit *vNStates;
        TLabel *NumGroups;
        TEdit *vNValues;
        TStringGrid *Grid;
        TButton *Clear;
        TButton *Apply;
        TEdit *vInput;
        TEdit *vSave;
        TButton *Input;
        TButton *Save;
        TButton *Exit;
        TOpenDialog *OpenInput;
        TSaveDialog *SaveOutput;
        TCheckBox *IncludeNext;
        TCheckBox *Use;
        void __fastcall vNStatesChange(TObject *Sender);
        void __fastcall vNValuesChange(TObject *Sender);
        void __fastcall ClearClick(TObject *Sender);
        void __fastcall ApplyClick(TObject *Sender);
        void __fastcall ExitClick(TObject *Sender);
        void __fastcall InputClick(TObject *Sender);
        void __fastcall SaveClick(TObject *Sender);
private:	// User declarations
        void __fastcall ClearGrid( void );

public:		// User declarations
        int rows;
        int cols;
        int nstates;
        int NUstates;                   // number of unique state names
        int ntargs;
        char StateList[NSTATES][STATELENGTH];          // list of unique states
        int Group[NGROUPS];                  // Group for each row
        int State[NGROUPS][NSTATES];               // States for each row
        int Value[NGROUPS][NSTATES];              // values for each row
        
        __fastcall TUseStateForm(TComponent* Owner);
        __fastcall ~TUseStateForm();
        void __fastcall SetVals( void );
};
//---------------------------------------------------------------------------
extern PACKAGE TUseStateForm *UseStateForm;
//---------------------------------------------------------------------------
#endif
