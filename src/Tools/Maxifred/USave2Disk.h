//---------------------------------------------------------------------------

#ifndef USave2DiskH
#define USave2DiskH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TfSave2Disk : public TForm
{
__published:	// IDE-managed Components
        TLabel *Label1;
        TLabel *Label2;
        TLabel *Label3;
        TLabel *Label4;
        TEdit *eFilename;
        TLabel *Label5;
        TLabel *Label6;
        TLabel *Label7;
        TButton *bSave;
        TButton *bCancel;
        void __fastcall bSaveClick(TObject *Sender);
        void __fastcall bCancelClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TfSave2Disk(TComponent* Owner);
        bool    save;
        AnsiString      filename;
};
//---------------------------------------------------------------------------
extern PACKAGE TfSave2Disk *fSave2Disk;
//---------------------------------------------------------------------------
#endif
