/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#pragma hdrstop

#include "Unit1.h"
#include <stdio.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button1Click(TObject *Sender)
{
        int res;
        int delay;

        char cpy[256];

        strcpy(cpy,"copy ");
        strcat(cpy,vParmfile->Text.c_str());
        strcat(cpy," ");
        strcat(cpy,vRunParm->Text.c_str() );
        system( cpy );

        delay= atoi( vDelay->Text.c_str() );



        res= spawnl(P_NOWAIT,vOperator->Text.c_str(),"",NULL);
        Sleep(delay);
        res= spawnl(P_NOWAIT,vSource->Text.c_str(),vSource->Text.c_str(),"AUTOSTART",vIp->Text.c_str(),NULL);
        Sleep(delay);
        res= spawnl(P_NOWAIT,vSignalProcessing->Text.c_str(),vSignalProcessing->Text.c_str(),"AUTOSTART",vIp->Text.c_str(),NULL);
        Sleep(delay);
        res= spawnl(P_NOWAIT,vApplication->Text.c_str(),vApplication->Text.c_str(),"AUTOSTART",vIp->Text.c_str(),NULL);

}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button2Click(TObject *Sender)
{
        Close();        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SaveClick(TObject *Sender)
{

        FILE *save;

        SaveFile->Execute();
        vSave->Text= SaveFile->FileName;

        if( ( save= fopen( vSave->Text.c_str(),"w+") ) == NULL )
        {
           //     Application->MessageBox("Could not Open Output File", "Fatal Error",MB_OK);
                return;
        }

        fprintf(save,"Operator          %s \n",vOperator->Text.c_str());
        fprintf(save,"Source            %s \n",vSource->Text.c_str(),vSource->Text.c_str());
        fprintf(save,"SignalProcessing  %s \n",vSignalProcessing->Text.c_str());
        fprintf(save,"Application       %s \n",vApplication->Text.c_str());
        fprintf(save,"IP                %s \n",vIp->Text.c_str());
        fprintf(save,"Parmfile          %s \n",vParmfile->Text.c_str() );
        fprintf(save,"RunParm           %s \n",vRunParm->Text.c_str() );

        fclose( save );
}
//---------------------------------------------------------------------------

void __fastcall TForm1::GetClick(TObject *Sender)
{
        FILE *saved;
        char line1[256];
        char line2[256];


        OpenSaved->Execute();
        vGet->Text= OpenSaved->FileName;

        if( ( saved= fopen(vGet->Text.c_str(),"r")) == NULL )
        {
        //        Application->MessageBox("Could not Open Input File", "Fatal Error",MB_OK);
                return;
        }

        while(fscanf(saved,"%s %s",line1,line2) != EOF )
        {
                if( strcmp("Operator",line1) == 0 )
                {
                        vOperator->Text= line2;
                }
                else if( strcmp("Source",line1) == 0 )
                {
                        vSource->Text= line2;
                }
                else if( strcmp("SignalProcessing",line1) == 0 )
                {
                        vSignalProcessing->Text= line2;
                }
                else if( strcmp("Application",line1) == 0 )
                {
                        vApplication->Text= line2;
                }
                else if( strcmp("IP",line1) == 0 )
                {
                        vIp->Text= line2;
                }
                else if( strcmp("Parmfile",line1) == 0 )
                {
                        vParmfile->Text= line2;
                }
                else if( strcmp("RunParm",line1) == 0 )
                {
                        vRunParm->Text= line2;
                }
        }

        fclose(saved);

}
//---------------------------------------------------------------------------

void __fastcall TForm1::OperatorClick(TObject *Sender)
{
        OpenOperator->Execute();
        vOperator->Text= OpenOperator->FileName;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::EEGsourceClick(TObject *Sender)
{
        OpenSource->Execute();
        vSource->Text= OpenSource->FileName;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SignalProcessClick(TObject *Sender)
{
        OpenSignalProcessing->Execute();
        vSignalProcessing->Text= OpenSignalProcessing->FileName;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ApplicationClick(TObject *Sender)
{
        OpenApplication->Execute();
        vApplication->Text= OpenApplication->FileName;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ParmfileClick(TObject *Sender)
{
        OpenParmfile->Execute();
        vParmfile->Text= OpenParmfile->FileName;


}
//---------------------------------------------------------------------------

void __fastcall TForm1::RunParmClick(TObject *Sender)
{
        RunParmfile->Execute();
        vRunParm->Text= RunParmfile->FileName;
}
//---------------------------------------------------------------------------

