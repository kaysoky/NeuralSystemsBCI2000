/*************************************************************************
Task.cpp is the source code for the Right Justified Boxes task
*************************************************************************/
#include <vcl.h>
#pragma hdrstop

#include <vector>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dos.h>

#include "UState.h"
#include "BCIDirectry.h"
#include "Usr.h"
#include "Task.h"

TTask::TTask(  PARAMLIST *plist, STATELIST *slist )
{
        char line[512];

        run= 0;

        vis= NULL;
        appl= NULL;

        strcpy(line,"UsrTask int PreRunInterval= 20 0 0 100 // Pause prior to starting a run (in units of SampleBlocks)");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int PreTrialPause= 10 0 0 1 // Duration of Target w/o cursor");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int ItiDuration= 10 0 0 1 // Duration of Intertrial Interval");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int RewardDuration= 10 0 0 1 // Duration of PostTrial Feedback");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int NumberTargets= 2 0 0 1 // Number of Targets");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask float TargetWidth= 25 0 0 1 // Width of Targets");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int BaselineInterval= 1 0 0 1 // Intercept Computation 1 = targets 2 = ITI");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int TimeLimit= 180 180 0 1000 // Time Limit for Runs in seconds");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int RestingPeriod= 0 0 0 1   //  1 defines a rest periuod of data acquisition");
        plist->AddParameter2List(line,strlen(line));

        slist->AddState2List("TargetCode 5 0 0 0\n");
        slist->AddState2List("ResultCode 5 0 0 0\n");
        slist->AddState2List("StimulusTime 16 17528 0 0\n");
        slist->AddState2List("Feedback 2 0 0 0\n");
        slist->AddState2List("IntertrialInterval 2 0 0 0\n");
        slist->AddState2List("RestPeriod 2 0 0 0\n");
        slist->AddState2List("CursorPosX 16 0 0 0\n");
        slist->AddState2List("CursorPosY 16 0 0 0\n");

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


void TTask::Initialize( PARAMLIST *plist, STATEVECTOR *new_svect, CORECOMM *new_corecomm)
{
        STATELIST *slist;
        AnsiString FInit,SSes,SName,AName;
        BCIDtry *bcidtry;
        char FName[256];
        time_t ctime;
        struct tm *tblock;

        corecomm=new_corecomm;

        PreRunInterval=         atoi(plist->GetParamPtr("PreRunInterval")->GetValue());
        PtpDuration=            atoi(plist->GetParamPtr("PreTrialPause")->GetValue());
        ItiDuration=            atoi(plist->GetParamPtr("ItiDuration")->GetValue());
        OutcomeDuration=        atoi(plist->GetParamPtr("RewardDuration")->GetValue());
        Ntargets=               atoi(plist->GetParamPtr("NumberTargets")->GetValue());
        TargetWidth=            atof(plist->GetParamPtr("TargetWidth")->GetValue());
        BaselineInterval=       atoi(plist->GetParamPtr("BaselineInterval")->GetValue());
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

                strcat(FName,"\\");

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

        User->Initialize( plist, slist, corecomm );
        User->GetLimits( &limit_right, &limit_left, &limit_top, &limit_bottom );
        User->Scale( (float)0x7fff, (float)0x7fff );
        ComputeTargets( Ntargets );
        targetcount= 0;
        ranflag= 0;

        time( &ctime );
        randseed= -ctime;

        cursor_x_start= limit_left;
        cursor_y_start= ( limit_top + limit_bottom ) /2;

        ReadStateValues( svect );

        CurrentTarget= 0;
        TargetTime= 0;
        CurrentBaseline= 0;
        BaselineTime= 0;
        CurrentFeedback= 0;
        FeedbackTime= 0;
        // if there is no PreRunInterval (i.e., it is 0), switch to ITI directly
        if (PreRunInterval > 0)
           {
           CurrentPri= 1;
           CurrentIti= 0;
           }
        else
           {
           CurrentPri= 0;
           CurrentIti= 1;
           }
        PriTime= 0;
        ItiTime= 0;
        CurrentFeedback= 0;
        FeedbackTime= 0;
        CurrentOutcome= 0;
        OutcomeTime= 0;
        CurrentRest= Resting;
        CurRunFlag= 0;
        Hits= 0;
        Misses= 0;
        User->PutO(false);

        WriteStateValues( svect );
}

void TTask::ReadStateValues(STATEVECTOR *statevector)
{
        CurrentTarget=       statevector->GetStateValue("TargetCode");
        CurrentOutcome=      statevector->GetStateValue("ResultCode");
        CurrentStimulusTime= statevector->GetStateValue("StimulusTime");
        CurrentFeedback=     statevector->GetStateValue("Feedback");
        CurrentIti=          statevector->GetStateValue("IntertrialInterval");
        CurrentRunning=      statevector->GetStateValue("Running");
                if( CurRunFlag == 1 )     // 0 must cycle through at least once
                {
                        if( CurrentRunning == 1 ) CurrentRunning = 0;
                        else                      CurRunFlag= 0;
                }

        CurrentRest=         statevector->GetStateValue("RestPeriod");

}

void TTask::WriteStateValues(STATEVECTOR *statevector)
{
        bcitime=new BCITIME;
        CurrentStimulusTime= bcitime->GetBCItime_ms();                   // time stamp
        delete bcitime;
        statevector->SetStateValue("StimulusTime",CurrentStimulusTime);

        statevector->SetStateValue("TargetCode",CurrentTarget);
        statevector->SetStateValue("ResultCode",CurrentOutcome);
        statevector->SetStateValue("Feedback",CurrentFeedback);
        statevector->SetStateValue("IntertrialInterval",CurrentIti);
        statevector->SetStateValue("Running",CurrentRunning);
        statevector->SetStateValue("RestPeriod",CurrentRest);
        statevector->SetStateValue("CursorPosX",(int)x_pos*User->scale_x );
        statevector->SetStateValue("CursorPosY",(int)y_pos*User->scale_y );
}

void TTask::ComputeTargets( int ntargs )
{
        int i;
        float y_range;

        y_range= limit_bottom - limit_top;

        for(i=0;i<ntargs;i++)
        {
                User->targx[i+1]=      limit_right - ( 1.0 * TargetWidth );
                User->targsizex[i+1]=  TargetWidth;
                User->targy[i+1]=      limit_top + i * y_range/ntargs;
                User->targsizey[i+1]=  y_range/ntargs;
                User->targy_btm[i+1]=  User->targy[i+1]+User->targsizey[i+1];
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


void TTask::TestCursorLocation( float x, float y )
{
        int i;

        x*= User->scale_x;
        y*= User->scale_y;

        if( x > ( limit_right - TargetWidth ) )
        {
                User->PutCursor( x_pos, y_pos, CURSOR_RESULT );

                CurrentOutcome= 1;

                if( y < User->targy[1] )              y= User->targy[1];
                if( y > User->targy_btm[Ntargets] )   y= User->targy_btm[Ntargets];

                for(i=0;i<Ntargets;i++)
                {
                        if( ( y > User->targy[i+1] ) && ( y<= User->targy_btm[i+1] ) )
                        {
                                CurrentOutcome= i+1;

                        }
                }

                // flash outcome (only) when YES/NO (display the same for hits and misses)
                User->Outcome(0, CurrentOutcome );

                if( CurrentOutcome == CurrentTarget )
                {
                          User->PutTarget(CurrentTarget, TARGET_RESULT );
                          Hits++;
                          HitOrMiss=true;
                }
                else
                {
                         User->PutTarget(CurrentTarget, TARGET_OFF );
                         Misses++;
                         HitOrMiss=false;
                }
                CurrentFeedback= 0;
                if( BaselineInterval == 1 )
                        CurrentBaseline= 0;
        }

}

void TTask::UpdateSummary( void )
{
        float pc;

        if( Hits+Misses > 0 )    pc= 100.0 * Hits / (Hits+Misses);
        else                     pc= -1.0;

        fprintf(appl,"Run %2d   Hits=%3d  Total=%3d  Percent=%6.2f \n",run,Hits,Hits+Misses,pc);
        fprintf(appl,"Number of Targets=%2d \n",Ntargets);
        fprintf(appl,"Bits= %7.2f \n",bitrate.TotalBitsTransferred() );
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
    CurrentFeedback=0;
    CurrentTarget=0;
    CurrentOutcome=0;
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
       // ITI period just started
       if (ItiTime == 0)
          {
          sprintf(memotext, "Run: %d; ITI -> new trial: %d\r", run, trial);
          vis->SendMemo2Operator(memotext);
          trial++;
          }
            // in case the run was longer than x seconds
            timepassed= ((double)(TDateTime::CurrentTime())-runstarttime)*86400;

            if (timepassed > timelimit)
               {
               CurrentRunning=0;
               CurRunFlag= 1;
               if (Hits+Misses > 0)
                  {
                  sprintf(memotext, "Run %d - %.1f%% correct\r", run, (float)Hits*100/((float)Hits+(float)Misses));
                  vis->SendMemo2Operator(memotext);
              //    fprintf( appl,"%s \n",memotext);
                  }
               else
                  {
                  vis->SendMemo2Operator("No statistics for this run\r");
                  vis->SendMemo2Operator("There were no trials\r");
                  }
               sprintf(memotext, "Total Bits transferred: %.2f\r", bitrate.TotalBitsTransferred());
               vis->SendMemo2Operator(memotext);
            //   fprintf( appl,"%s \n",memotext);
               sprintf(memotext, "Average Bits/Trial: %.2f\r", bitrate.BitsPerTrial());
               vis->SendMemo2Operator(memotext);
           //    fprintf( appl,"%s \n",memotext);
               sprintf(memotext, "Average Bits/Minute: %.2f\r", bitrate.BitsPerMinute());
               vis->SendMemo2Operator(memotext);
            //   fprintf( appl,"%s \n",memotext);
               vis->SendMemo2Operator("**************************\r");
             //  fprintf( appl,"**************************\r\n");
               User->Clear();
               User->PutT(true);
               UpdateSummary();
               }
        }

    else if (CurrentOutcome > 0 )
            {
            // trial just ended ...
            if (OutcomeTime == 0)
               {
               sprintf(memotext, "%d hits %d missed\r", Hits, Misses);
               vis->SendMemo2Operator(memotext);
               bitrate.Push(HitOrMiss);
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


// if we have defined this period, show some initial feedback
// after a run has been started
void TTask::Pri( void )
{
 User->PreRunInterval(PriTime);
 PriTime++;

 // set all state variables to 0 in this period
 CurrentIti=0;
 CurrentTarget=0;
 CurrentBaseline=0;
 CurrentFeedback=0;
 CurrentOutcome=0;

 if (PriTime >= PreRunInterval)
    {
    CurrentPri=0;
    CurrentIti=1;
    }
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
                CurrentTarget= GetTargetNo( Ntargets );
                User->PutTarget(CurrentTarget, TARGET_ON );

                if( BaselineInterval == 1 )
                        CurrentBaseline= 1;
                if( BaselineInterval == 2 )
                        CurrentBaseline= 0;


        }
}

void TTask::Ptp( void )
{
        PtpTime++;

        if( PtpTime > PtpDuration )
        {

                x_pos= cursor_x_start;
                y_pos= cursor_y_start;
                if( User->scale_x > 0.0 ) x_pos/= User->scale_x;
                if( User->scale_y > 0.0 ) y_pos/= User->scale_y;
                User->PutCursor( x_pos, y_pos, CURSOR_ON );
                CurrentFeedback= 1;
                PtpTime= 0;
        }

}

void TTask::Feedback( short sig_x, short sig_y )
{

        x_pos+= sig_x;
        y_pos+= sig_y;
        User->PutCursor( x_pos, y_pos, CURSOR_ON );
        TestCursorLocation( x_pos, y_pos );
}


void TTask::Outcome()
{
        OutcomeTime++;
        User->Outcome( OutcomeTime, CurrentOutcome );                   // flash outcome when YES/NO

        if( OutcomeTime >= OutcomeDuration-1 )
        {
                CurrentIti= 1;
                CurrentOutcome= 0;
                CurrentTarget= 0;
                OutcomeTime= 0;
                User->PutCursor( x_pos, y_pos, CURSOR_OFF );
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
               // CurrentRest= 1;
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

void TTask::Process( const GenericSignal* Input, GenericSignal* Output )
{
        const std::vector< float >& signals = Input->GetChannel( 0 );

        ReadStateValues( svect );

        if( CurrentRunning > 0 )
        {
                if(CurrentRest > 0 )             Rest();
                else if (CurrentPri > 0)         Pri();
                else if (CurrentIti > 0)         Iti();
                else if (CurrentFeedback > 0 )   Feedback(signals[1], signals[0]);
                else if (CurrentOutcome  > 0 )   Outcome();
                else if (CurrentTarget   > 0 )   Ptp();
        }

        UpdateDisplays();
        WriteStateValues( svect );
}


