//---------------------------------------------------------------------------

#ifndef UMainH
#define UMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include "UPreferences.h"

//---------------------------------------------------------------------------
class TfMain : public TForm
{
__published:	// IDE-managed Components
        TPanel *DropPanel;
        TLabel *tFileName;
        TButton *bShowParams;
        TMainMenu *MainMenu1;
        TMenuItem *File1;
        TMenuItem *Open1;
        TMenuItem *Quit1;
        TOpenDialog *OpenDialog1;
        TLabel *Label1;
        TLabel *Label2;
        TLabel *Label3;
        TLabel *Label4;
        TLabel *tSampleBlockSize;
        TLabel *tSamplingRate;
        TLabel *tUpdateRate;
        void __fastcall DropPanelWindowProc( TMessage& msg );
        void __fastcall bShowParamsClick(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall Quit1Click(TObject *Sender);
        void __fastcall Open1Click(TObject *Sender);
private:	// User declarations
        TWndMethod defaultDropPanelWindowProc;
        void    EnableAll();
        void    DisableAll();
        PREFERENCES preferences;
        PARAMLIST   paramlist;
        BCI2000DATA *bci2000data;
        bool    RetrieveFileInfo(AnsiString filename);
        void    DisplayFileInfo();
public:		// User declarations
        __fastcall TfMain(TComponent* Owner);
        void    ProcessFile( const char* inFileToProcess );
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
