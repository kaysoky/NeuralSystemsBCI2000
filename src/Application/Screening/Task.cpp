/*************************************************************************
Task.cpp is the source code for the Right Justified Boxes task
*************************************************************************/
#include <vcl.h>
#pragma hdrstop

#include "Task.h"
#include "Usr.h"
#include "BCIDirectry.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dos.h>


TTask::TTask(  PARAMLIST *plist, STATELIST *slist )
{
        char line[512];

        run= 0;

        vis= NULL;
        appl= NULL;

        strcpy(line,"UsrTask int TargetDuration= 20 0 0 1 // Duration of Target in cursor update units");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int ItiDuration= 10 0 0 1 // Duration of Intertrial Interval");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int RewardDuration= 10 0 0 1 // Duration of PostTrial Feedback");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int NumberTargets= 2 0 0 1 // Number of Targets");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int TargetOrientation= 1 0 0 1 // Orientation 1= Vertical 2= Horizontal 3= Both");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask float TargetWidth= 25 0 0 1 // Width of Targets in Pixels");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask float TargetHeight= 25 0 0 1 // Height of Targets in Pixels");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int TimeLimit= 180 180 0 1000 // Time Limit for Runs in seconds");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int RestingPeriod= 0 0 0 1   //  1 defines a rest periuod of data acquisition");
        plist->AddParameter2List(line,strlen(line));

        slist->AddState2List("TargetCode 5 0 0 0\n");
        slist->AddState2List("ResultCode 5 0 0 0\n");
        slist->AddState2List("StimulusTime 16 17528 0 0\n");
        slist->AddState2List("Feedback 2 0 0 0\n");
        slist->AddState2List("IntertrialInterval 2 1 0 0\n");
        slist->AddState2List("RestPeriod 2 0 0 0\n");

        User->SetUsr( plist, slist );

        OldRunning=0;
        OldCurrentTarget=0;

}

//-----------------------------------------------------------------------------

TTask::~TTask( void )
{
        if( vis ) delete vis;
        vis= NULL;
        fclose( appl );
}


void TTask::Initialize( PARAMLIST *plist, STATEVECTOR *new_svect, CORECOMM *new_corecomm, TApplication *applic)
{
        STATELIST       *slist;
        AnsiString FInit,SSes,SName,AName;
        BCIDtry *bcidtry;
        char FName[120];
        char slash[2];
        time_t ctime;
        struct tm *tblock;

        Applic= applic;

        corecomm=new_corecomm;

        TargetDuration=         atoi(plist->GetParamPtr("TargetDuration")->GetValue());
        ItiDuration=            atoi(plist->GetParamPtr("ItiDuration")->GetValue());
        Ntargets=               atoi(plist->GetParamPtr("NumberTargets")->GetValue());
        TargetOrien=            atoi(plist->GetParamPtr("TargetOrientation")->GetValue());
        TargetWidth=            atof(plist->GetParamPtr("TargetWidth")->GetValue());
        TargetHeight=           atof(plist->GetParamPtr("TargetHeight")->GetValue());
        Resting=                atoi(plist->GetParamPtr("RestingPeriod")->GetValue());

        timelimit=              atof(plist->GetParamPtr("TimeLimit")->GetValue());

        FInit= AnsiString (plist->GetParamPtr("FileInitials")->GetValue());
        SSes = AnsiString (plist->GetParamPtr("SubjectSession")->GetValue());
        SName= AnsiString (plist->GetParamPtr("SubjectName")->GetValue());


        if( appl == NULL )
        {
                bcidtry= new BCIDtry();

                bcidtry->SetDir( FInit.c_str() );
                bcidtry->ProcPath();
                bcidtry->SetName( SName.c_str() );
                bcidtry->SetSession( SSes.c_str() );

                strcpy(FName, bcidtry->ProcSubDir() );

                slash[0]= 0x5c;
                slash[1]= 0x00;
                strcat(FName,slash);

                AName= SName + "S" + SSes + ".apl";
                strcat(FName, AName.c_str() );               // cpy vs cat

                if( (appl= fopen(FName,"a+")) == NULL)       // report error if NULL
                {
                        bcidtry->FileError( Applic, FName );
                }

                fprintf(appl,"%s \n",AName.c_str() );

                ctime= time(NULL);
                tblock= localtime(&ctime);
                fprintf(appl,"%s \n", asctime( tblock ) );
        }

        bitrate.Initialize(Ntargets);

        trial=1;
        svect=new_svect;
        slist=svect->GetStateListPtr();

        if (vis) delete vis;
        vis= new GenericVisualization( plist, corecomm );
        vis->SetSourceID(SOURCEID_TASKLOG);
        vis->SendCfg2Operator(SOURCEID_TASKLOG, CFGID_WINDOWTITLE, "User Task Log");

        User->Initialize( plist, slist );
        User->GetLimits( &limit_right, &limit_left, &limit_top, &limit_bottom );

        ComputeTargets( Ntargets );
        targetcount= 0;
        ranflag= 0;

        time( &ctime );
        randseed= -ctime;

        ReadStateValues( svect );

        CurrentTargetDuration= 0;
        CurrentTarget= 0;
        TargetTime= 0;
        CurrentBaseline= 0;
        BaselineTime= 0;
        CurrentFeedback= 0;
        FeedbackTime= 0;
        CurrentIti= 1;
        ItiTime= 0;
        CurrentFeedback= 0;
        FeedbackTime= 0;
        CurrentRest= Resting;
        User->PutO(false);

        WriteStateValues( svect );
}

void TTask::ReadStateValues(STATEVECTOR *statevector)
{
        CurrentTarget=       statevector->GetStateValue("TargetCode");
        CurrentStimulusTime= statevector->GetStateValue("StimulusTime");
        CurrentFeedback=     statevector->GetStateValue("Feedback");
        CurrentIti=          statevector->GetStateValue("IntertrialInterval");
        CurrentRunning=      statevector->GetStateValue("Running");
        CurrentRest=         statevector->GetStateValue("RestPeriod");

}

void TTask::WriteStateValues(STATEVECTOR *statevector)
{
        bcitime=new BCITIME;
        CurrentStimulusTime= bcitime->GetBCItime_ms();                   // time stamp
        delete bcitime;
        statevector->SetStateValue("StimulusTime",CurrentStimulusTime);

        statevector->SetStateValue("TargetCode",CurrentTarget);
        statevector->SetStateValue("Feedback",CurrentFeedback);
        statevector->SetStateValue("IntertrialInterval",CurrentIti);
        statevector->SetStateValue("Running",CurrentRunning);
        statevector->SetStateValue("RestPeriod",CurrentRest);
}

void TTask::ComputeTargets( int ntargs )
{
        int i;
        float y_range;
        float x_range;

        y_range= limit_bottom - ( limit_top + TargetHeight );
        x_range= limit_right - ( limit_left + TargetWidth );

        if( TargetOrien == 1 )
        {
                for(i=0;i<ntargs;i++)
                {
                        targx[i+1]=      (limit_right - TargetWidth) / 2.0;
                        targsizex[i+1]=  TargetWidth;
                        targy[i+1]=      limit_top + i * y_range/ (ntargs-1);
                        targsizey[i+1]=  TargetHeight;
                        targy_btm[i+1]=  targy[i+1]+targsizey[i+1];
                }
        }
        else if( TargetOrien == 2 )
        {
                for(i=0;i<ntargs;i++)
                {
                        targy[i+1]=      (limit_bottom - TargetHeight) / 2.0;
                        targsizex[i+1]=  TargetWidth;
                        targx[i+1]=      limit_left + i * x_range/ (ntargs-1);
                        targsizey[i+1]=  TargetHeight;
                        targy_btm[i+1]=  targy[i+1]+targsizey[i+1];
                }
        }
        else if( TargetOrien == 3 )
        {
        }

}

void TTask::ShuffleTargs( int ntargs )
{
        int i,j;
        float rval;

        for(i=0;i<ntargs;i++)
        {
                rpt:    rval= User->ran1( &randseed );
                        targs[i]= 1 + (int)( rval * ntargs );

                if( (targs[i] < 1) || (targs[i]>Ntargets) )
                        goto rpt;

                for(j=0;j<i; j++)
                         if( targs[j]==targs[i] ) goto rpt;
        }
}

int TTask::GetTargetNo( int ntargs )
{
        int retval;

        if( ranflag == 0 )
        {
                ShuffleTargs( ntargs );
                ranflag= 1;
        }

        retval= targs[targetcount];

        targetcount++;
        if( targetcount > (ntargs-1) )
        {
                ranflag= 0;
                targetcount= 0;
        }

        return( retval );

}


void TTask::UpdateSummary( void )
{
        float pc;

        fprintf(appl,"Run %2d   \n",run);
        fprintf(appl,"Number of Targets=%2d \n",Ntargets);
        fprintf(appl,"Time Passed (sec)=%7.2f \n",timepassed);
        fprintf(appl,"..........................................................\n\n");
}

void TTask::UpdateDisplays( void )
{

char            memotext[256];


 if (CurrentRunning == 0)
    CurrentRunning=0;

 if ((CurrentRunning == 0) && (OldRunning == 1))            // put the T up there on the transition from not suspended to suspended
    {
    User->Clear();
    User->PutT(true);
    CurrentIti=1;
    }
 if (CurrentRunning == 1)
    {
    if (OldRunning == 0)
       {
       runstarttime=(double)(TDateTime::CurrentTime());   // run just started ?
       run++;
       }
    User->PutT(false);
    // in the ITI period ?
    if (CurrentIti > 0)
       {
            // in case the run was longer than x seconds
            timepassed= ((double)(TDateTime::CurrentTime())-runstarttime)*86400;

            if (timepassed > timelimit)
               {
                CurrentRunning=0;

                vis->SendMemo2Operator("No statistics for this run\r");
                vis->SendMemo2Operator("There were no trials\r");
                vis->SendMemo2Operator("**************************\r");
                User->Clear();
                User->PutT(true);
                UpdateSummary();
               }
        }

    }

 // new trial just started
 if ((CurrentTarget > 0) && (OldCurrentTarget == 0))
    {
    sprintf(memotext, "Target: %d\r", CurrentTarget);
    vis->SendMemo2Operator(memotext);
    }

 OldRunning=CurrentRunning;
 OldCurrentTarget=CurrentTarget;
}

void TTask::Iti( void )
{
        if( BaselineInterval == 2 )
                CurrentBaseline= 1;
        User->Clear();
        ItiTime++;

        if( ItiTime > ItiDuration )
        {
                CurrentIti = 0;
                ItiTime = 0;
                CurrentFeedback= 1;
                OldCurrentTarget= 0;
                CurrentTarget= GetTargetNo( Ntargets );
                User->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clRed   );

                if( BaselineInterval == 1 )
                        CurrentBaseline= 1;
                if( BaselineInterval == 2 )
                        CurrentBaseline= 0;


        }
}


void TTask::Feedback( short sig_x, short sig_y )
{
        CurrentTargetDuration++;


        if( CurrentTargetDuration > TargetDuration )
        {
                User->PutTarget( targx[ CurrentTarget ],
                       targy[ CurrentTarget ],
                       targsizex[ CurrentTarget ],
                       targsizey[ CurrentTarget ],
                       clBlack   );
                CurrentFeedback= 0;
                CurrentIti= 1;
                CurrentTarget= 0;
                ItiTime= 0;
                CurrentTargetDuration= 0;
                if( BaselineInterval == 1 )
                        CurrentBaseline= 0;
                OldCurrentTarget= CurrentTarget;

        }

}

void TTask::Rest( void )
{
        if ((CurrentRunning == 1) && (OldRunning == 0))
        {
                runstarttime=(double)(TDateTime::CurrentTime());   // run just started ?
                run++;
                OldRunning= 1;
                User->Clear();
                User->PutT(false);
                User->PutO(true);
                CurrentIti= 0;

        }

        timepassed= ((double)(TDateTime::CurrentTime())-runstarttime)*86400;

         if (timepassed > timelimit)
         {
               CurrentRunning=0;
               User->Clear();
               User->PutO(false);
               User->PutT(true);
               CurrentRest= 0;
               CurrentIti= 1;
         }

}

void TTask::Process( short *signals )
{

        ReadStateValues( svect );

        if( CurrentRunning > 0 )
        {
                if(CurrentRest > 0 )             Rest();
                else if (CurrentIti > 0)         Iti();
                else if (CurrentFeedback > 0 )   Feedback( signals[1], signals[0] );
        }

        UpdateDisplays();
        WriteStateValues( svect );
}


