/*************************************************************************
Task.cpp is the source code for the Right Justified Boxes task
*************************************************************************/
#include <vcl.h>
#pragma hdrstop

#include "Task.h"
#include "Usr.h"
#include "BCIDirectry.h"
#include "UState.h"

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

        strcpy(line,"UsrTask int PreTrialPause= 10 0 0 1 // Duration of Target w/o cursor");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int ItiDuration= 10 0 0 1 // Duration of Intertrial Interval");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int RewardDuration= 10 0 0 1 // Duration of PostTrial Feedback");
        plist->AddParameter2List(line,strlen(line));

        strcpy(line,"UsrTask int FeedbackDuration= 20000 0 0 1 // Max Trial Duration");
        plist->AddParameter2List(line,strlen(line));

        strcpy(line,"UsrTask int BaselineInterval= 1 0 0 1 // Intercept Computation 1 = targets 2 = ITI");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int TimeLimit= 180 180 0 1000 // Time Limit for Runs in seconds");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int RestingPeriod= 0 0 0 1   //  1 defines a rest periuod of data acquisition");
        plist->AddParameter2List(line,strlen(line));

        strcpy(line,"JoyStick int UseJoyStick= 0 0 0 1    // 1 to use Joystick Input");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"JoyStick float JoyXgain= 1.0 0 -1000.0 1000.0 // Horizontal gain");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"JoyStick float JoyYgain= 1.0 0 -1000.0 1000.0 // Horizontal gain");
        plist->AddParameter2List(line,strlen(line));

        strcpy(line,"Targets int NumberTargets= 4 0 0 1 // Number of Targets");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"Targets int IncludeAllTargets= 0 0 0 1 // Test all target positions?");
        plist->AddParameter2List(line,strlen(line));

        strcpy(line,"Targets matrix TargetPos= 7 4 25 25 0 90 0 90 25 25 50 50 10 10 10 10 50 50 0 0 -1 1 -1 1 0 0 0 0 0 0 // Target Position Matrix - Values are 0-100");
        plist->AddParameter2List(line,strlen(line));
         

        slist->AddState2List("TargetCode 5 0 0 0\n");
        slist->AddState2List("ResultCode 5 0 0 0\n");
        slist->AddState2List("StimulusTime 16 17528 0 0\n");
        slist->AddState2List("Feedback 2 0 0 0\n");
        slist->AddState2List("IntertrialInterval 2 1 0 0\n");
        slist->AddState2List("RestPeriod 2 0 0 0\n");
        slist->AddState2List("CursorPosX 16 0 0 0\n");
        slist->AddState2List("CursorPosY 16 0 0 0\n");

        slist->AddState2List("Xadapt 16 0 0 0\n");
        slist->AddState2List("Yadapt 16 0 0 0\n");
        slist->AddState2List("AdaptCode 5 0 0 0 \n");



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


void TTask::Initialize( PARAMLIST *plist, STATEVECTOR *new_svect, CORECOMM *new_corecomm ) // , TApplication *applic)
{
        STATELIST *slist;
        AnsiString FInit,SSes,SName,AName;
        BCIDtry *bcidtry;
        char FName[120];
        char slash[2];
        time_t ctime;
        struct tm *tblock;
        int i,j;

    //    Applic= applic;

        corecomm= new_corecomm;

        PtpDuration=       atoi(plist->GetParamPtr("PreTrialPause")->GetValue());
        ItiDuration=       atoi(plist->GetParamPtr("ItiDuration")->GetValue());
        OutcomeDuration=   atoi(plist->GetParamPtr("RewardDuration")->GetValue());
        FeedbackDuration=  atoi(plist->GetParamPtr("FeedbackDuration")->GetValue() );
        Ntargets=          atoi(plist->GetParamPtr("NumberTargets")->GetValue());
    //    TargetWidth=       atof(plist->GetParamPtr("TargetWidth")->GetValue());
    //    TargetHeight=      atof(plist->GetParamPtr("TargetHeight")->GetValue());
        targetInclude=     atoi(plist->GetParamPtr("IncludeAllTargets")->GetValue() );
        BaselineInterval=  atoi(plist->GetParamPtr("BaselineInterval")->GetValue());
        Resting=           atoi(plist->GetParamPtr("RestingPeriod")->GetValue());

        timelimit=         atof(plist->GetParamPtr("TimeLimit")->GetValue());

        FInit= AnsiString (plist->GetParamPtr("FileInitials")->GetValue());
        SSes = AnsiString (plist->GetParamPtr("SubjectSession")->GetValue());
        SName= AnsiString (plist->GetParamPtr("SubjectName")->GetValue());

        useJoy= atoi(plist->GetParamPtr("UseJoyStick")->GetValue());
        joy_xgain= atof(plist->GetParamPtr("JoyXgain")->GetValue());
        joy_ygain= atof(plist->GetParamPtr("JoyYgain")->GetValue());

        n_tmat= Ntargets;
        m_tmat= 7;              // was 4;   - now includes width and height  and adaptation code

        for( i= 0; i<m_tmat; i++)
        {
                for(j=0; j<n_tmat; j++)
                {
                        tmat[i][j]= atoi(plist->GetParamPtr("TargetPos")->GetValue(i,j) );
                }
        }

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
        User->GetSize( &size_right, &size_left, &size_top, &size_bottom );
        User->Scale( (float)0x7fff, (float)0x7fff );
        ComputeTargets( Ntargets );
        targetcount= 0;
        ranflag= 0;

        cursor_x_start= ( limit_right - limit_left ) / 2;
        cursor_y_start= ( limit_bottom - limit_top ) / 2;

        time( &ctime );
        randseed= -ctime;

        ReadStateValues( svect );

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
        CurrentOutcome= 0;
        OutcomeTime= 0;
        CurrentRest= Resting;
        CurRunFlag= 0;
        Hits= 0;
        Misses= 0;

        PtpTime= 0;

        ntrials= 0;
        ntimeouts= 0;
        totalfeedbacktime= 0;
        feedflag= 0;


        CurrentXadapt= 0;
        CurrentYadapt= 0;
        CurrentAdaptCode= 0;

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

        CurrentXadapt=       statevector->GetStateValue("Xadapt");
        CurrentYadapt=       statevector->GetStateValue("Yadapt");
        CurrentAdaptCode=    statevector->GetStateValue("AdaptCode");

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
        statevector->SetStateValue("CursorPosX",(int)x_pos );
        statevector->SetStateValue("CursorPosY",(int)y_pos );

        statevector->SetStateValue("Xadapt",CurrentXadapt);
        statevector->SetStateValue("Yadapt",CurrentYadapt);
        statevector->SetStateValue("AdaptCode",CurrentAdaptCode);

}

void TTask::ComputeTargets( int ntargs )
{
        int i;
        float y_range;
        float x_range;

        y_range= size_bottom - size_top;
      //  y_range= y_range- (1.0 * TargetWidth);

        x_range= size_right - size_left;
     //   x_range= x_range - (1.0 * TargetHeight);

        for(i=0;i<ntargs;i++)
        {
                targx[i+1]=          tmat[0][i] * x_range / 100;
                targsizex[i+1]=      tmat[2][i] * x_range / 100;
                targx_rt[i+1]=       targx[i+1] + targsizex[i+1];
                targy[i+1]=          tmat[1][i] * y_range / 100;
                targsizey[i+1]=      tmat[3][i] * y_range / 100;
                targy_btm[i+1]=      targy[i+1] + targsizey[i+1];
                targx_adapt[i+1]=    tmat[4][i];
                targy_adapt[i+1]=    tmat[5][i];
                targ_adaptcode[i+1]= tmat[6][i];
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

int TTask::TestTarget( float xpos, float ypos, int targ )
{
        int result;
        int half;

        half= User->HalfCursorSize;

        result= 0;

        // could use cursor center by adding 1/2 of size to position

        if( ( (xpos+half) > targx[targ] ) && ( (xpos-half) < targx_rt[targ] ) )
        {
                if( ( (ypos+half) > targy[targ] ) && ( (ypos-half) < targy_btm[targ] ) ) result= 1;
        }

        return( result );
}


void TTask::TestCursorLocation( float x, float y )
{
        int i;
        int res;

 //       x*= User->scalex;
 //       y*= User->scaley;


        if( targetInclude == 0 )
        {
                res= TestTarget( x, y, CurrentTarget );
                if( res == 1 )
                {
                        User->PutCursor( &x_pos, &y_pos, clBlue );
                        User->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clYellow   );
                        CurrentFeedback= 0;
                        CurrentOutcome= CurrentTarget;
                        if( BaselineInterval == 1 )
                                CurrentBaseline= 0;
                }
        }


        else if( targetInclude == 1 )
        {

                res= TestTarget( x, y, CurrentTarget );         // test correct first
                if ( res == 1 )
                {
                        CurrentOutcome= CurrentTarget;
                        goto jmp;
                }

                for(i=1; i<Ntargets+1; i++)
                {
                        if( TestTarget( x, y, i ) == 1 )
                        {
                                CurrentOutcome= i;
                                goto jmp;
                        }
                }

                return;

jmp:            if( CurrentOutcome == CurrentTarget )
                {
                         User->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clYellow   );
                          Hits++;
                          HitOrMiss=true;
                }
                else
                {
                         User->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clBlack   );
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

        fprintf(appl," @ D2Box @ \n");

        if( targetInclude == 0 )
        {
                fprintf(appl,"NoError Mode Run %2d  Trials  %3d  Timeouts  %3d \n",run,ntrials,ntimeouts);
                if( ntrials > 0 ) fprintf(appl,"   Mean trial duration= %6.2f \n",totalfeedbacktime/(double)(ntrials) );
        }
        else
        {

                if( Hits+Misses > 0 )    pc= 100.0 * Hits / (Hits+Misses);
                else                     pc= -1.0;

                fprintf(appl,"Run %2d   Hits=%3d  Total=%3d  Percent=%6.2f \n",run,Hits,Hits+Misses,pc);
                fprintf(appl,"Number of Targets=%2d \n",Ntargets);
                fprintf(appl,"Bits= %7.2f \n",bitrate.TotalBitsTransferred() );
        }



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

                User->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clRed   );

                if( BaselineInterval == 1 )
                        CurrentBaseline= 1;
                if( BaselineInterval == 2 )
                        CurrentBaseline= 0;


                CurrentXadapt= targx_adapt[CurrentTarget];
                CurrentYadapt= targy_adapt[CurrentTarget];
                CurrentAdaptCode= targ_adaptcode[CurrentTarget];
         }
         if( (targetInclude == 0) && (feedflag == 1) )
         {
                totalfeedbacktime+= ((double)(TDateTime::CurrentTime() ) - feedstart ) * 86400;
                feedflag= 0;
         }
}

void TTask::Ptp( void )
{
        MMRESULT mmres;
        JOYINFO joyi;

        PtpTime++;

        if( PtpTime > PtpDuration )
        {

                x_pos= cursor_x_start;
                y_pos= cursor_y_start;
                User->PutCursor( &x_pos, &y_pos, clRed );
                CurrentFeedback= 1;
                FeedbackTime= 0;

                PtpTime= 0;

                if( useJoy == 1 )   // try && (PtpTime == 1) )
                {
                         mmres= joyGetPos( JOYSTICKID1, &joyi );
                         jx_cntr= joyi.wXpos;
                         jy_cntr= joyi.wYpos;
                }
        }

}


void TTask::GetJoy( )
{
        MMRESULT mmres;
        JOYINFO joyi;
        float tx;
        float ty;

        mmres= joyGetPos( JOYSTICKID1, &joyi );

        tx= (float)(joyi.wXpos)  - (float)(jx_cntr);
        ty= (float)(joyi.wYpos)  - (float)(jy_cntr);

        tx= tx / (float)jhalf;
        ty= ty / (float)jhalf;

        x_pos+= tx * joy_xgain;
        y_pos+= ty * joy_ygain;
}

void TTask::Feedback( short sig_x, short sig_y )
{
        if( useJoy==1 ) GetJoy();
        else
        {
                x_pos+= (float)sig_x * User->scalex;
                y_pos+= (float)sig_y * User->scaley;
        }

        User->PutCursor( &x_pos, &y_pos, clRed );
        TestCursorLocation( x_pos, y_pos );

        FeedbackTime++;
        if( FeedbackTime > FeedbackDuration )
        {
                FeedbackTime= 0;
                CurrentFeedback= 0;
                CurrentOutcome= 0;
                CurrentIti= 1;
                ntimeouts++;
        }
        if( feedflag == 0 )
        {
                feedflag= 1;
                ntrials++;
                feedstart= (double)(TDateTime::CurrentTime());
        }
}


void TTask::Outcome()
{
        OutcomeTime++;

        if( OutcomeTime > OutcomeDuration )
        {
                CurrentIti= 1;
                CurrentOutcome= 0;
                CurrentTarget= 0;
                OutcomeTime= 0;
                User->PutCursor( &x_pos, &y_pos, clBlack );
        }

        if( (targetInclude == 0) && (feedflag == 1) )
        {
                totalfeedbacktime+= ((double)(TDateTime::CurrentTime() ) - feedstart ) * 86400;
                feedflag= 0;
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

void TTask::Process( const GenericSignal * Input, GenericSignal * Output )
{
        const std::vector<float>& signals = Input->GetChannel( 0 );

        ReadStateValues( svect );

        if( CurrentRunning > 0 )
        {
                if(CurrentRest > 0 )             Rest();
                else if (CurrentIti > 0)         Iti();
                else if (CurrentFeedback > 0 )   Feedback( signals[1], signals[0] );
                else if (CurrentOutcome  > 0 )   Outcome();
                else if (CurrentTarget   > 0 )   Ptp();
        }

        UpdateDisplays();
        WriteStateValues( svect );
}


