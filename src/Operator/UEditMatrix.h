//---------------------------------------------------------------------------

#ifndef UEditMatrixH
#define UEditMatrixH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "CSPIN.h"
#include <Grids.hpp>
#include <SyncObjs.hpp>
#include "UParameter.h"

//---------------------------------------------------------------------------
class TfEditMatrix : public TForm
{
__published:	// IDE-managed Components
        TStringGrid *StringGrid;
        TCSpinEdit *cColumnsMax;
        TCSpinEdit *cRowsMax;
        TLabel *Label1;
        TLabel *Label2;
        TButton *bChangeMatrixSize;
        TLabel *tComment;
        TButton *bToggleEditing;
        void __fastcall bChangeMatrixSizeClick(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
        PARAM   *matrix_param;
        AnsiString matrix_param_name;
        TCriticalSection* lock;
        void    UpdateDisplay();
        void    AdaptColumnWidths();
        void    Lock()   { lock->Acquire(); }
        void    Unlock() { lock->Release(); }
        void __fastcall ToggleLabelEditing( TObject* );
        void    EditLabels();
        void    EditEntries();
public:		// User declarations
        __fastcall TfEditMatrix(TComponent* Owner);
        __fastcall ~TfEditMatrix();
        void       SetDisplayedParam( PARAM* );
        AnsiString GetDisplayedParamName() const;
};
//---------------------------------------------------------------------------
extern PACKAGE TfEditMatrix *fEditMatrix;
//---------------------------------------------------------------------------
#endif
