//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ProcessForm1.h"
#include "BCIOutput.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProcessForm *ProcessForm;
//---------------------------------------------------------------------------
__fastcall TProcessForm::TProcessForm(TComponent* Owner)
        : TForm(Owner)
{
  const struct
  {
    SidelobeSuppression type;
    const char* name;
  } sidelobeSuppressionTypes[] =
  {
    { none,     "None" },
    { hamming,  "Hamming" },
    { hann,     "Hann" },
    { blackman, "Blackman" },
  };
  cbSidelobeSuppression->Items->Clear();
  for( int i = 0; i < sizeof( sidelobeSuppressionTypes ) / sizeof( *sidelobeSuppressionTypes ); ++i )
  {
    if( sidelobeSuppressionTypes[ i ].type != i )
      Application->MessageBox(
        "Sidelobe suppression type names are messed up in " __FUNC__ ".",
        "Error", MB_OK | MB_ICONSTOP );
    cbSidelobeSuppression->Items->Append( sidelobeSuppressionTypes[ i ].name );
  }
  cbSidelobeSuppression->ItemIndex = 0;
}
//---------------------------------------------------------------------------
void __fastcall TProcessForm::ExitClick(TObject *Sender)
{
        Close();
}
//---------------------------------------------------------------------------

void __fastcall TProcessForm::MemWinTypeClick(TObject *Sender)
{
        if( MemWinType->ItemIndex == 1 )
        {
                Label8->Visible= true;
                vMemWindows->Visible= true;
                Label7->Visible= true;
                vMemBlockSize->Visible= true;
                Label5->Visible= true;
                vMemDataLength->Visible= true;
        }
        else
        {
                Label8->Visible= false;
                vMemWindows->Visible= false;
                Label7->Visible= false;
                vMemBlockSize->Visible= false;
                Label5->Visible= false;
                vMemDataLength->Visible= false;
        }
}
//---------------------------------------------------------------------------

void __fastcall TProcessForm::vMemWindowsChange(TObject *Sender)
{
        vMemDataLength->Text=  atoi(vMemWindows->Text.c_str() ) * atoi( vMemBlockSize->Text.c_str() );
}
//---------------------------------------------------------------------------

void __fastcall TProcessForm::vMemBlockSizeChange(TObject *Sender)
{
        vMemDataLength->Text=  atoi(vMemWindows->Text.c_str() ) * atoi( vMemBlockSize->Text.c_str() );
}
//---------------------------------------------------------------------------

void __fastcall TProcessForm::vMemDataLengthChange(TObject *Sender)
{
        vMemDataLength->Text=  atoi(vMemWindows->Text.c_str() ) * atoi( vMemBlockSize->Text.c_str() );
}
//---------------------------------------------------------------------------

