/*************************************************************************
Task.cpp is the source code for the Right Justified Boxes task
*************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include <vector>

#include "UState.h"
#include "Task.h"

RegisterFilter( TTask, 3 );

//------------------ Status Class Definition ------------------------
//                  programed by Dr. Thilo Hinterberger 2000
//--------------------------------------------------------------------------------------------------

TTask::TTask()
: status( new TSTATUS ),
  ClassSequencer( new TClassSequencer( Parameters, States ) ),
  Decider( new TDecider( Parameters, States ) ),
  TaskManager( new TTaskManager( Parameters, States ) ),
  SessionManager( new TSessionManager( Parameters, States ) )
{
  FBForm = new TFBForm( ( TComponent* )NULL );
  FBForm->Show();
  FBForm->SetFBForm( Parameters, States );
}

//-----------------------------------------------------------------------------

TTask::~TTask( void )
{
 delete ClassSequencer;
 delete Decider;
 delete TaskManager;
 delete SessionManager;
 delete status;
 delete FBForm;
}


void TTask::Initialize()
{
#ifdef BCI2000_STRICT
  // TSTATUS does not have an Initialize() member but needs to be
  // re-initialized anyway.
  delete status;
  status = new TSTATUS;
  // Reset all states declared by this module.
  {
    const char* statesToReset[] =
    {
      "Baseline",
      "Classification",
      "EndOfClass",
      "ResultCode",
      "Artifact",
      "TargetCode",
      "BaselineInterval",
      "FeedbackInterval",
      "InterTrialInterval",
      "EndOfTrial",
      "BeginOfTrial",
      "Feedback"
    };
    const int numStatesToReset = sizeof( statesToReset ) / sizeof( *statesToReset );
    for( int i = 0; i < numStatesToReset; ++i )
      Statevector->SetStateValue( statesToReset[ i ], 0 );
  }
#endif // BCI2000_STRICT
  FBForm->Initialize(Statevector);
  ClassSequencer->Initialize(Parameters, Statevector);
  Decider->Initialize(Parameters, Statevector);
  TaskManager->Initialize(Parameters, Statevector);
  SessionManager->Initialize(Parameters, Statevector, status);
}

void TTask::Process( const GenericSignal* Input, GenericSignal* Output )
{
  const std::vector< float >& signals = Input->GetChannel( 0 );
#ifdef BCI2000_STRICT
  {
    // Static is ok because there is only one instance anyway.
    static short lastRunning = 0;
    short curRunning = Statevector->GetStateValue( "Running" );
    if( lastRunning && !curRunning )
        FBForm->LeaveRunningState();
    lastRunning = curRunning;
    if( !curRunning )
      return;
  }
#endif // BCI2000_STRICT
   if (status->SysStatus==RUNNING) Decider->Process(signals);
#ifndef BCI2000_STRICT
   FBForm->Process(signals[0]);
#endif // BCI2000_STRICT
   switch (status->SysStatus) {
       case STOP: {
                     if (Statevector->GetStateValue("Recording")==1) status->SysStatus=START;
                     if (status->RecStatus>1) status->RecStatus = NOSTORAGE;
                  }; break;
       case START: {
                    //ClearStatevector();
                    FBForm->BringToFront();
                    ClassSequencer->Process(true);
                    SessionManager->Process(true);
                    TaskManager->Process(true);
                    status->SysStatus=RUNNING;
                    }; break;
       case RUNNING: {
#ifdef BCI2000_STRICT
                    FBForm->Process(signals[0]);
#endif // BCI2000_STRICT
                    ClassSequencer->Process(false);
                    SessionManager->Process(false);
                    TaskManager->Process(false);
                    }; break;
   } // switch
}


