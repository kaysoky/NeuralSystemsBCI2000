//--- ParmIO.cpp ---

#include "ParmIO.h"
#include <string.h>

ParIO::ParIO(  )
{
}

void ParIO::SaveStateForm()
{
        int i,j;

        fprintf(sfile,"State_NumStates= %s \n",usf->vNStates->Text.c_str() );
        fprintf(sfile,"State_NumGroups= %s \n",usf->vNValues->Text.c_str() );

        for(i=1;i<usf->cols;i++)
                for(j=1;j<usf->rows;j++)
                        fprintf(sfile,"State_Cell %2d %2d = %s \n",i,j,usf->Grid->Cells[i][j] );
}

void ParIO::SaveInputForm( void )
{
        int i;

        if( iform->CheckSpatialFilter->Checked )
        {
                fprintf(sfile,"Input_SpatialFilter true \n");
                fprintf(sfile,"Input_SpatialFile %s \n", iform->vSpatialFile->Text);
        }
        else
                fprintf(sfile,"Input_SpatialFilter false \n");

        if( iform->CheckAlign->Checked )
        {
                fprintf(sfile,"Input_Align true \n");
        }
        else
                fprintf(sfile,"Input_Align false \n");

        if( iform->AllCh->Checked )
        {
                fprintf(sfile,"Input_AllCh true \n");
        }
        else
                fprintf(sfile,"Input_AllCh false \n");

        if( iform->CheckTemporalFilter->Checked )
        {
                fprintf(sfile,"Input_TemporalFilter true \n");
                fprintf(sfile,"Input_TemporalFile %s \n",iform->vTemporalFile->Text);
        }
        else
                fprintf(sfile,"Input_TemporalFilter false \n");

        if( iform->CheckStateList->Checked )
        {
                fprintf(sfile,"Input_StateList true \n");
                fprintf(sfile,"Input_StateCount %d \n",iform->StateList->Lines->Count);
                for(i=0;i<iform->StateList->Lines->Count;i++)
                        fprintf(sfile,"Input_StateName %s \n",iform->StateList->Lines->Strings[i].c_str() );
        }
        else
                fprintf(sfile,"Input_StateList false \n");

        fprintf(sfile,"Input_ChanCount %d \n",iform->ChanList->Lines->Count);
        for(i=0;i<iform->ChanList->Lines->Count;i++)
                fprintf(sfile,"Input_ChanValue %d \n",atoi( iform->ChanList->Lines->Strings[i].c_str() ) );
}

void ParIO::SaveProcessForm( void )
{
        if( pform->UseMEM->Checked == true )
        {
                fprintf(sfile,"Process_UseMEM true \n");
                fprintf(sfile,"Process_MEMStart= %6.2f \n",atof( pform->vStart->Text.c_str() ) );
                fprintf(sfile,"Process_MEMEnd= %6.2f \n",atof( pform->vEnd->Text.c_str() ) );
                fprintf(sfile,"Process_MEMDensity= %6.2f \n",atof( pform->vDensity->Text.c_str() ) );
                fprintf(sfile,"Process_MEMBandwidth= %6.2f \n",atof( pform->vBandwidth->Text.c_str() ) );
                fprintf(sfile,"Process_MEMModel= %2d \n",atoi( pform->vModel->Text.c_str() ) );
                fprintf(sfile,"Process_MEMRemove= %2d \n",pform->Remove->ItemIndex );
        }
        else
                fprintf(sfile,"Process_UseMEM false \n");
}

void ParIO::SaveOutputForm( void )
{
        int i;
        
        fprintf(sfile,"Output_Start= %6.2f \n",atof( oform->vStart->Text.c_str() ) );
        fprintf(sfile,"Output_End= %6.2f \n",atof( oform->vEnd->Text.c_str() ) );

        if( oform->OverlapMode->Checked == true )
                fprintf(sfile,"Output_Overlap true \n");
        else
                fprintf(sfile,"Output_Overlap false \n");

        if( oform->SubGroups->Checked == true )
        {
                fprintf(sfile,"Output_SubGroups true \n");
                fprintf(sfile,"Output_ComputeMeans %s \n",oform->vCompuMeans->Text.c_str() );
        }
        else
                fprintf(sfile,"Output_SubGroups false \n");
                
        fprintf(sfile,"Output_Decimate %3d \n",atoi(oform->vDecimate->Text.c_str() ) );
        fprintf(sfile,"Output_Statistics= %1d \n",oform->Statistics->ItemIndex );
        fprintf(sfile,"Output_OutOrder= %1d \n",oform->OutputOrder->ItemIndex );

        fprintf(sfile,"Output_TimesCount %d \n",oform->Times->Lines->Count);
        for(i=0;i<oform->Times->Lines->Count;i++)
                fprintf(sfile,"Output_TimeVals %d \n",atoi( oform->Times->Lines->Strings[i].c_str() ) );


}
void ParIO::SaveF( FILE *savefile, TUseStateForm *usesform , TInputForm *inform,
        TProcessForm *procform, TOutputForm *outform)
{
        sfile= savefile;
        iform= inform;
        usf= usesform;
        pform= procform;
        oform= outform;

        SaveStateForm();
        SaveInputForm();
        SaveProcessForm();
        SaveOutputForm();
}

void ParIO::GetF( FILE *getfile, TUseStateForm *usesform, TInputForm *inform,
        TProcessForm *procform, TOutputForm *outform)
{
        char l1[128],l2[128],l3[32],l4[32],l5[32];
        int i,j;

        gfile= getfile;
        iform= inform;
        usf= usesform;
        pform= procform;
        oform= outform;

        while( fscanf(gfile,"%s %s",l1,l2) != EOF )
        {

                if( strcmp(l1,"State_NumStates=") == 0 )
                {
                        usf->vNStates->Text= l2;
                }
                else if( strcmp(l1,"State_NumGroups=") == 0 )
                {
                        usf->vNValues->Text= l2;
                }
                else if( strcmp( l1,"State_Cell") == 0 )
                {
                        fscanf(gfile,"%s %s %s",l3,l4,l5);
                        i= atoi( l2 );
                        j= atoi( l3 );
                        usf->Grid->Cells[i][j]= l5;
                }
                else if( strcmp( l1,"Input_SpatialFilter") == 0 )
                {
                        if( strcmp(l2,"true") == 0 ) iform->CheckSpatialFilter->Checked= true;
                        else                         iform->CheckSpatialFilter->Checked= false;
                }
                else if( strcmp( l1,"Input_SpatialFile") == 0 )
                        iform->vSpatialFile->Text= l2;
                else if( strcmp( l1,"Input_Align") == 0 )
                {
                        if( strcmp(l2,"true") == 0 ) iform->CheckAlign->Checked= true;
                        else                         iform->CheckAlign->Checked= false;
                }
                else if( strcmp( l1,"Input_TemporalFilter") == 0 )
                {
                        if( strcmp(l2,"true") == 0 ) iform->CheckTemporalFilter->Checked= true;
                        else                         iform->CheckTemporalFilter->Checked= false;
                }
                else if( strcmp( l1,"Input_StateList") == 0 )
                {
                        if( strcmp(l2,"true") == 0 )
                        {
                                iform->CheckStateList->Checked= true;
                                iform->StateList->Clear();
                        }
                        else    iform->CheckStateList->Checked= false;
                }
                else if( strcmp( l1,"Input_StateName") == 0 )
                        iform->StateList->Lines->Add( l2 );
                else if( strcmp( l1,"Input_ChanCount") == 0 )
                        iform->ChanList->Clear();
                else if( strcmp( l1,"Input_ChanValue") == 0 )
                        iform->ChanList->Lines->Add( atoi( l2 ) );
                else if( strcmp( l1,"Process_UseMEM") == 0 )
                {
                        if( strcmp(l2,"true") == 0 )
                        {
                                pform->UseMEM->Checked= true;
                        }
                        else
                                pform->UseMEM->Checked= false;
                }
                else if( strcmp( l1,"Process_MEMStart=") == 0 )
                        pform->vStart->Text= l2;
                else if( strcmp( l1,"Process_MEMEnd=") == 0 )
                        pform->vEnd->Text= l2;
                else if( strcmp( l1,"Process_MEMDensity=") == 0 )
                        pform->vDensity->Text= l2;
                else if( strcmp( l1,"Process_MEMBandwidth=") == 0 )
                        pform->vBandwidth->Text= l2;
                else if( strcmp( l1,"Process_MEMModel=") == 0 )
                        pform->vModel->Text= l2;
                else if( strcmp( l1,"Process_MEMRemove=") == 0 )
                        pform->Remove->ItemIndex= atoi(l2);
                else if( strcmp( l1,"Output_Start=") == 0 )
                        oform->vStart->Text= l2;
                else if( strcmp( l1,"Output_End=") == 0 )
                        oform->vEnd->Text= l2;
                else if( strcmp( l1,"Output_Overlap") == 0 )
                {
                        if( strcmp( l2,"true") == 0 )
                                oform->OverlapMode->Checked= true;
                        else
                                oform->OverlapMode->Checked= false;
                }
                else if( strcmp( l1,"Output_SubGroups") == 0 )
                {
                        if( strcmp( l2,"true") == 0 )
                                oform->SubGroups->Checked= true;
                        else
                                oform->SubGroups->Checked= false;
                }
                else if( strcmp( l1,"Output_ComputeMeans" ) == 0 )
                        oform->vCompuMeans->Text= l2;
                else if( strcmp( l1, "Output_Decimate" ) == 0 )
                        oform->vDecimate->Text= l2;
                else if( strcmp( l1, "Output_Statistics=" ) == 0 )
                        oform->Statistics->ItemIndex= atoi( l2 );
                else if( strcmp( l1,"Output_OutOrder=" ) == 0 )
                        oform->OutputOrder->ItemIndex= atoi( l2 );
                else if( strcmp( l1,"Output_TimesCount" ) == 0 )
                        oform->Times->Clear();
                else if( strcmp( l1,"Output_TimeVals" ) == 0 )
                        oform->Times->Lines->Add( l2 );
        }
}
