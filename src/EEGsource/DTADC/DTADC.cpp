/******************************************************************************
 * Program:   DTsource.EXE                                                    *
 * Module:    DTADC.CPP                                                       *
 * Comment:   Definition for the Data Translation class                       *
 * Version:   0.01                                                            *
 * Author:    Gerwin Schalk & Dennis McFarland                                *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 05/11/2000 - First start                                           *
 *         05/23/2000 - completed first start and documented                  *
 *         05/24/2000 - Data Translation Driver Added                         *
 ******************************************************************************/

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "GenericADC.h"
#include "DTADC.h"
#include "MsgWin1.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

// FILE *dttest;

// **************************************************************************
// Function:   ADInit
// Purpose:    This function parameterizes the DTADC
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop (fMain->MainDataAcqLoop())
// Parameters: N/A
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************


DTFUN dtfun;

int DTADC::ADInit()
{
int     i;
bool    flag;


        channels=atoi(paramlist->GetParamPtr("SoftwareCh")->GetValue());
        blocksize=atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
        samplerate=atoi(paramlist->GetParamPtr("SamplingRate")->GetValue());
        strcpy( dtfun.BoardName, paramlist->GetParamPtr("BoardName")->GetValue() );

        dtfun.InitBoard();

        // MsgWin->Show();
        //   dtfun.SetWindow( MsgWin->Msgw );


        dtfun.SetFunction( );
        ADConfig();

        StartFlag= 0;

 if ((!paramlist->GetParamPtr("SamplingRate")) || (!paramlist->GetParamPtr("TransmitCh")) || (!paramlist->GetParamPtr("SampleBlockSize")))
    return(0);

        if( signal ) delete signal;



 signal= new GenericIntSignal( channels, blocksize );


 return(1);
}

int DTADC::ADConfig()
{
        ECODE status;

        ChanType= OL_CHNT_SINGLEENDED;
        ListSize= channels;
        dGain= 2;
        ClkSource= 0;   // 0 means use internal clock
        dfFreq= channels * samplerate;
        Bufferpts= (UINT) channels * blocksize;

        status= dtfun.ConfigAD(  ChanType,
                                ListSize,
                                dGain,
                                ClkSource,
                                dfFreq,
                                Bufferpts );
        return( status );
}

// **************************************************************************
// Function:   ADReadDataBlock
// Purpose:    This function is called within fMain->MainDataAcqLoop()
//             it fills the already initialized array RawEEG with values
//             and DOES NOT RETURN, UNTIL ALL DATA IS ACQUIRED
// Parameters: N/A
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int DTADC::ADReadDataBlock()
{
        int sample, channel, count;
        int value;
        int i;
      
        if( StartFlag == 0 )
        {
                dtfun.Start();
                StartFlag= 1;
        }

   /*
        while( MsgWin->DoneFlag < 1 )
        {

                WaitMessage();
                Application->ProcessMessages();
        }
   */
        dtfun.bdone->WaitFor(3000);

        count= 0;

        for(i=0;i<dtfun.BufferCount;i++)
        {

                for (sample=0; sample<blocksize; sample++)
                {
                        for (channel=0; channel<channels; channel++)
                        {
                                value= dtfun.data[count];

                                signal->SetValue( channel, sample, value );
                                count++;
                        }
                }
        }

        dtfun.BufferCount= 0;
        dtfun.BufferPtr= 0;

        dtfun.bdone->ResetEvent();

        // MsgWin->DoneFlag= 0;
        return(1);
}

int DTADC::ADShutdown()
{

        dtfun.Stop();
        dtfun.Reset();
        dtfun.CleanUp();

        return(1);
}


// **************************************************************************
// Function:   RandomNumberADC
// Purpose:    The constructor for the RandomNumberADC
//             it fills the provided list of parameters and states
//             with the parameters and states it requests from the operator
// Parameters: plist - the list of parameters
//             slist - the list of states
// Returns:    N/A
// **************************************************************************
DTADC::DTADC(PARAMLIST *plist, STATELIST *slist)
{

char            line[512];

 channels=0;

 // store the pointer to the parameter list and state list
 // we need the lists later on, e.g., in ADInit()
 // of course, we then can't meanwhile destroy the paramlist object
 paramlist=plist;
 statelist=slist;

 // add all the parameters that this ADC requests to the parameter list
 // in this case, the parameters come from a file
 // of course, they could be 'hard-coded', just the way the states
 // are added on the bottom

strcpy(line,"Source int SoftwareCh=      64 64 1 128  // this is the number of digitized channels");
paramlist->AddParameter2List(line, strlen(line) );

strcpy(line,"Source int TransmitCh=      16 5 1 128  // this is the number of transmitted channels");
paramlist->AddParameter2List( line, strlen(line) );

strcpy(line,"Source int SampleBlockSize= 16 5 1 128 // this is the number of samples transmitted at a time");
paramlist->AddParameter2List(line, strlen(line) );

strcpy(line,"Source int SamplingRate=    128 128 1 4000  // this is the sample rate");
paramlist->AddParameter2List(line, strlen(line) );

strcpy(line,"Source string BoardName=    BCI_IN // this is the name of the AD board");
paramlist->AddParameter2List(line, strlen( line ) );

// strcpy(line,"Storage string FileName=    DTdata.dat // this is the name of the data file");
// paramlist->AddParameter2List(line, strlen( line ) );

 // add all states that this ADC requests to the list of states
 // this is just an example (here, we don't really need all these states)
 statelist->AddState2List("Running 1 0 0 0\n");
 statelist->AddState2List("Active 1 1 0 0\n");
 statelist->AddState2List("SourceTime 16 2347 0 0\n");
 statelist->AddState2List("RunActive 1 1 0 0\n");

 // dtfun= new DTFUN();

}


DTADC::~DTADC()
{
int     i;

if( signal ) delete signal;
signal= NULL;

// if( dtfun ) delete dtfun;
// dtfun= NULL;

}

