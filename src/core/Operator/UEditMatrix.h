/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
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
#include "Param.h"
#include "cspin.h"

//---------------------------------------------------------------------------
class TfEditMatrix : public TForm
{
__published: // IDE-managed Components
        TStringGrid *StringGrid;
    TCSpinEdit *cNumCols;
    TCSpinEdit *cNumRows;
        TLabel *Label1;
        TLabel *Label2;
        TButton *bChangeMatrixSize;
        TLabel *tComment;
        void __fastcall bChangeMatrixSizeClick(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall StringGridSelectCell(TObject *Sender, int ACol,
          int ARow, bool &CanSelect);
        void __fastcall StringGridDrawCell(TObject *Sender, int ACol, int ARow,
          TRect &Rect, TGridDrawState State);

public: // User declarations
        __fastcall TfEditMatrix(TComponent* Owner);
        __fastcall ~TfEditMatrix();
        void       SetDisplayedParam( Param* );
        AnsiString GetDisplayedParamName() const;

private: // User declarations
        void       UpdateDisplay();
        void       UpdateParam();
        void       Lock()   { mpLock->Acquire(); }
        void       Unlock() { mpLock->Release(); }
        void       EditLabels();
        void       EditEntries();
        void       SelectTopLeftCell();

        void       LabelEditing( int = -1, int = -1 );
        bool       LabelEditing_Checked( int, int );
        void       AdaptColumnWidth( int = -1, int = -1 );
        void       EditSubMatrix( int, int );
        bool       EditSubMatrix_Enabled( int, int ) const;
        void       PlainCellToMatrix( int, int );
        bool       PlainCellToMatrix_Enabled( int, int ) const;
        void       MatrixToPlainCell( int, int );
        bool       MatrixToPlainCell_Enabled( int, int ) const;
        void       Help( int, int );
        bool       Help_Enabled( int, int ) const;

        static int        sNumInstances;
        Param*            mpMatrixParam;
        AnsiString        mMatrixParamName;
        TfEditMatrix*     mpSubEditor;
        TCriticalSection* mpLock;

        // Context menu infrastructure.
        void BuildContextMenu();
        void __fastcall PopupMenuPopup( TObject* inSender, const TPoint&, bool& );
        void __fastcall PopupMenuItemClick( TObject* inSender );

        struct MenuItemEntry
        {
          // The typedefs declare pointers to class instance member functions.
          typedef void ( TfEditMatrix::*MenuAction )( int, int );
          typedef bool ( TfEditMatrix::*MenuStateGetter )( int, int );
          MenuAction       mpAction;
          MenuStateGetter  mpGetEnabled,
                           mpGetChecked;
          const char*      mCaption;
        };
        static struct MenuItemEntry sMenuItems[];
        int mContextRow, mContextCol;
};
//---------------------------------------------------------------------------
extern PACKAGE TfEditMatrix *fEditMatrix;
//---------------------------------------------------------------------------
#endif
