/*************************************************************************
Task.cpp is the source code for the Right Justified Boxes task
*************************************************************************/


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

        strcpy(line,"UsrTask float CursorSpeed= 30 30 0 1000 // Sensitivity of Cursor");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask float BallSpeedX= 10 10 0 1000 // Ball Speed X");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask float BallSpeedY= 10 10 0 1000 // Ball Speed X");
        plist->AddParameter2List(line,strlen(line));

        slist->AddState2List("StimulusTime 16 17528 0 0\n");
        slist->AddState2List("CursorMovement 2 0 0 0\n");

        User->SetUsr( plist, slist );

}

//-----------------------------------------------------------------------------

TTask::~TTask( void )
{
        if( vis ) delete vis;
        vis= NULL;
        if( appl ) fclose( appl );
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

        CursorSpeed=            atof(plist->GetParamPtr("CursorSpeed")->GetValue());
        ball_delta_x=           atof(plist->GetParamPtr("BallSpeedX")->GetValue());
        ball_delta_y=           atof(plist->GetParamPtr("BallSpeedY")->GetValue());

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

                if( (appl= fopen(FName,"w+")) == NULL)       // report error if NULL
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

        targetcount= 0;
        ranflag= 0;

        time( &ctime );
        randseed= -ctime;

        cursor_x_start= (limit_left+limit_right)/2;
        cursor_y_start= limit_bottom;

        ball_x_start= (limit_left+limit_right)/2;
        ball_y_start= (limit_top+limit_bottom)/2;

        ReadStateValues( svect );
        CurrentCursorMovement=0;
        WriteStateValues( svect );
}


void TTask::ReadStateValues(STATEVECTOR *statevector)
{
        CurrentRunning=             statevector->GetStateValue("Running");
        CurrentCursorMovement=      statevector->GetStateValue("CursorMovement");
}

void TTask::WriteStateValues(STATEVECTOR *statevector)
{
        bcitime=new BCITIME;
        CurrentStimulusTime= bcitime->GetBCItime_ms();                   // time stamp
        delete bcitime;
        statevector->SetStateValue("StimulusTime",CurrentStimulusTime);

        statevector->SetStateValue("CursorMovement", CurrentCursorMovement);
        statevector->SetStateValue("Running",CurrentRunning);
}


void TTask::UpdateCursorPos(int sig_x)
{
 x_pos+=(float)sig_x/32768*CursorSpeed;

 User->PutCursor( x_pos, y_pos, clRed );
}


void TTask::UpdateBallPos()
{
 ball_x_pos += ball_delta_x;
 ball_y_pos += ball_delta_y;

 User->TestEdgeCollision(&ball_x_pos, &ball_y_pos, &ball_delta_x, &ball_delta_y);
 User->TestBrickCollision(&ball_x_pos, &ball_y_pos, &ball_delta_x, &ball_delta_y);

 User->PutBall( ball_x_pos, ball_y_pos, 20, 20, clLime );

 if (ball_delta_x > 0)
    CurrentCursorMovement=1;
 else
    CurrentCursorMovement=2;
}


void TTask::Process( short *signals )
{
static int OldRunning=0;

 ReadStateValues( svect );

 // start of game
 if ((CurrentRunning == 1) && (OldRunning == 0))
    {
    User->PutT(false);
    x_pos= cursor_x_start;
    y_pos= cursor_y_start;
    ball_x_pos= ball_x_start;
    ball_y_pos= ball_y_start;
    CurrentCursorMovement=0;
    // User->Ball->Visible=false;
    }
 // end of game
 if ((CurrentRunning == 0) && (OldRunning == 1))
    {
    User->PutT(true);
    // User->Ball->Visible=true;
    }

 if (CurrentRunning > 0)
    {
    UpdateCursorPos(signals[0]);
    UpdateBallPos();
    }

 OldRunning=CurrentRunning;
 WriteStateValues( svect );
}


