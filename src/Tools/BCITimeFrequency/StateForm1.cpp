/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "StateForm1.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

// FILE *statefile;

TUseStateForm *UseStateForm;
//---------------------------------------------------------------------------
__fastcall TUseStateForm::TUseStateForm(TComponent* Owner)
        : TForm(Owner)
{

        Grid->Cells[1][0]= "Group Code";
        Grid->Cells[0][1]= "Group ";
        Grid->Cells[0][2]= "Group ";
        Grid->Cells[1][1]= "1";
        Grid->Cells[1][2]= "2";
        Grid->Cells[2][1]= "stimulustype";
        Grid->Cells[2][2]= "stimulustype";
        Grid->Cells[3][1]= "0";
        Grid->Cells[3][2]= "1";
        Grid->Cells[4][1]= "flashing";
        Grid->Cells[4][2]= "flashing";
        Grid->Cells[5][1]= "1";
        Grid->Cells[5][2]= "1";
        rows= 2;
        cols= 5;
        NUstates= 0;

 //       statefile= fopen("Statefile.asc","w+");


}
__fastcall TUseStateForm::~TUseStateForm()
{
   //     fclose(statefile);
}
//---------------------------------------------------------------------------
void __fastcall TUseStateForm::vNStatesChange(TObject *Sender)
{
        Grid->ColCount= atoi( vNStates->Text.c_str() ) * 2 + 2;
}
//---------------------------------------------------------------------------
void __fastcall TUseStateForm::vNValuesChange(TObject *Sender)
{
        Grid->RowCount= atoi( vNValues->Text.c_str() ) + 1;
}
//---------------------------------------------------------------------------
void __fastcall TUseStateForm::ClearClick(TObject *Sender)
{
        ClearGrid();
}

void __fastcall TUseStateForm::ClearGrid( void )
{
        int i;
        int j;

        cols= atoi( vNStates->Text.c_str() ) * 2 + 2;
        rows= atoi( vNValues->Text.c_str() ) +1;

        for(i=1;i<cols;i++)
                for(j=1;j<rows;j++)
                        Grid->Cells[i][j]= "";
}
//---------------------------------------------------------------------------

void __fastcall TUseStateForm::ApplyClick(TObject *Sender)
{
        SetVals();
}

void __fastcall TUseStateForm::SetVals( void )
{
        int i;
        int j;
        int k;
        int statecount;

        nstates= atoi( vNStates->Text.c_str() );
        cols= nstates * 2 + 2;
        ntargs= atoi( vNValues->Text.c_str() );
        rows= ntargs +1;

 // clear everything

        for(i=0; i<NGROUPS; i++)
        {
                Group[i]= 0;
                for(j=0;j<NSTATES;j++)
                {
                        State[i][j]= 0;
                        Value[i][j]= 0;
                }
        }
        for(j=0;j<NSTATES;j++)
                strcpy(StateList[j],"");

 // get all unique statenames

        statecount= 0;

        for(i=1;i<rows;i++)
        {
                for(j=2;j<cols;j+=2)
                {
                        for(k=0;k<statecount;k++)
                        {
                                if(strcmp(StateList[k],Grid->Cells[j][i].c_str() )== 0 )
                                        goto jmpout;
                        }

                        strcpy( StateList[statecount], Grid->Cells[j][i].c_str() );
                        statecount++;

jmpout:
                }
         }

         for(i=1;i<rows;i++)
         {
                Group[i-1]= atoi( Grid->Cells[1][i].c_str() );

                for(j=2;j<cols;j+=2)
                {
                        for(k=0;k<statecount;k++)
                        {
                                if( strcmp(StateList[k],Grid->Cells[j][i].c_str() )== 0 )
                                        State[i-1][j/2-1]= k;
                        }
                        Value[i-1][j/2-1]=  atoi( Grid->Cells[j+1][i].c_str() );
                }
         }
         NUstates= statecount;
}
//---------------------------------------------------------------------------

void __fastcall TUseStateForm::ExitClick(TObject *Sender)
{
        SetVals();
        Close();
}
//---------------------------------------------------------------------------

void __fastcall TUseStateForm::InputClick(TObject *Sender)
{
        FILE *infile;
        char line[32];
        char v1[32],v2[32],v3[32],v4[32];
        int i,j;


        OpenInput->Execute();
        vInput->Text= OpenInput->FileName;

        if( (infile= fopen(vInput->Text.c_str(),"r") ) == NULL )
        {
                Application->MessageBox("Error opening Input","File Error",MB_OK);
                return;
        }

        ClearGrid();

        while( fscanf(infile,"%s",line) != EOF )
        {
                if( strcmp(line,"NumStates=") == 0 )
                {
                        fscanf(infile,"%s",v1);
                        vNStates->Text= v1;
                }
                else if( strcmp(line,"NumGroups=") == 0 )
                {
                        fscanf(infile,"%s",v1);
                        vNValues->Text= v1;
                }
                else if( strcmp(line,"Cell") == 0 )
                {
                        fscanf(infile,"%s %s %s %s",v1,v2,v3,v4);
                        i= atoi( v1 );
                        j= atoi( v2 );
                        Grid->Cells[i][j]= v4;

                }
        }
        fclose( infile );
}
//---------------------------------------------------------------------------

void __fastcall TUseStateForm::SaveClick(TObject *Sender)
{
        int i,j;
        FILE *outfile;

        SetVals();

        SaveOutput->Execute();
        vSave->Text= SaveOutput->FileName;

        if(  (outfile= fopen(vSave->Text.c_str(),"w+" ) )  == NULL )
        {
                Application->MessageBox("Error opening output","File Error",MB_OK);
                return;
        }

        fprintf(outfile,"NumStates= %s \n",vNStates->Text.c_str() );
        fprintf(outfile,"NumGroups= %s \n",vNValues->Text.c_str() );

        for(i=1;i<cols;i++)
                for(j=1;j<rows;j++)
                        fprintf(outfile,"Cell %2d %2d = %s \n",i,j,Grid->Cells[i][j]);

        fclose( outfile );

}
//---------------------------------------------------------------------------




