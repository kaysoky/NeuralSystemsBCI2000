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
        void __fastcall bChangeMatrixSizeClick(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
public:		// User declarations
        __fastcall TfEditMatrix(TComponent* Owner);
        PARAM   *matrix_param;
        void    UpdateDisplay();
};
//---------------------------------------------------------------------------
extern PACKAGE TfEditMatrix *fEditMatrix;
//---------------------------------------------------------------------------
#endif
