/*************************************************************************
Task.cpp is the source code for the Right Justified Boxes task
*************************************************************************/


#include "Task.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include "UMain.h"
class TfMain;
extern TfMain *fMain;

//------------------ Status Class Definition ------------------------
//                  programed by Dr. Thilo Hinterberger 2000
//--------------------------------------------------------------------------------------------------

TTask::TTask(PARAMLIST *paramlist, STATELIST *statelist )
{
         status = new TSTATUS();

         ClassSequencer = new TClassSequencer(paramlist, statelist);
         Decider = new TDecider(paramlist, statelist);
         TaskManager = new TTaskManager(paramlist, statelist);
         SessionManager = new TSessionManager(paramlist, statelist);
         FBForm = new TFBForm((TComponent*)fMain);
         FBForm->Show();
         FBForm->SetFBForm(paramlist, statelist);
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

 // if( vis ) delete vis;
 //    vis= NULL;
}


void TTask::Initialize( PARAMLIST *paramlist, STATEVECTOR *statevector, CORECOMM *new_corecomm, TApplication *applic)
{
          STATELIST *slist;
          svect = statevector;
          FBForm->Initialize(statevector);
          ClassSequencer->Initialize(paramlist, statevector);
          Decider->Initialize(paramlist, statevector);
          TaskManager->Initialize(paramlist, statevector);
          SessionManager->Initialize(paramlist, statevector, status);
}

void TTask::Process( short *signals )
{

   if (status->SysStatus==RUNNING) Decider->Process(signals);
   FBForm->Process(signals[0]);
   switch (status->SysStatus) {
       case STOP: {
                     if (svect->GetStateValue("Recording")==1) status->SysStatus=START;
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
                    ClassSequencer->Process(false);
                    SessionManager->Process(false);
                    TaskManager->Process(false);
                    }; break;
   } // switch
}


