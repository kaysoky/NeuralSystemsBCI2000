//--------------------------------------------------------------------------------------------------
//------------------ Unit for Decision-, Run- and Trial-Management ---------------------------------
//-------------------- programed by Dr. Thilo Hinterberger 2001 ------------------------------------
//--------------------------------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <dir.h>

#include "UState.h"
#include "UParameter.h"
#include "AppManager.h"

#pragma package(smart_init)

TSTATUS::TSTATUS()
{
   SysStatus = STOP;
   OldSysStatus = STOP;
   SourceStatus = DAS1402;
   RecStatus = STORING;
   FBFlag = true;
   TrialStatus = -1;
   getcwd(WorkingDir, 128);
   getcwd(DataDir, 128);
}

//---------------------------------------------------------------------------

  TDecider::TDecider(PARAMLIST *paramlist, STATELIST *statelist)
  {
    char line[255];

    statelist->AddState2List("EndOfClass 1 0 0 0\n");
    statelist->AddState2List("Classification 1 0 0 0\n");
    statelist->AddState2List("ResultCode 4 0 0 0\n");
    statelist->AddState2List("Artifact 1 0 0 0\n");

    strcpy(line, "Decider int DecisionMode= 2 2 0 4 // 0 no, 1 last class. position, 2 integral, 3 threshold, 4 time exceed threshold");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Decider float DecisionTime= 0.5 0.5 0 60 // time to keep the target in the goal");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Decider int DecisionCh= 0 0 0 1024 // Channel in FBSignal to decide");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Decider int NegThreshold= -80 -80 0 1023 // neg threshold for hit ");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Decider int PosThreshold= 80 80 0 1023 // pos threshold for hit ");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Decider float ZeroInterval= 0.5 0.5 0 1023 // +/-intervall of Result0 ");
    paramlist->AddParameter2List(line, strlen(line));

    Initialized = false;
    //Initialize(paramlist, NULL);
  }

  void TDecider::Initialize(PARAMLIST *paramlist, STATEVECTOR *NewStateVector)
  {
    statevector = NewStateVector;
    int BS = atoi(paramlist->GetParamPtr("SamplingRate")->GetValue())/atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
    DecisionMode = atoi(paramlist->GetParamPtr("DecisionMode")->GetValue());
    DecisionTime = atof(paramlist->GetParamPtr("DecisionTime")->GetValue())*BS;
    DecisionCh = atoi(paramlist->GetParamPtr("DecisionCh")->GetValue());
    NegThreshold = atoi(paramlist->GetParamPtr("NegThreshold")->GetValue());
    PosThreshold = atoi(paramlist->GetParamPtr("PosThreshold")->GetValue());
    ZeroInt = atof(paramlist->GetParamPtr("ZeroInterval")->GetValue());
    ResetVote();
    Initialized = true;
  }

  void TDecider::SetResultStates(float Res)
  {
   if (Res>ZeroInt) statevector->SetStateValue("ResultCode",4);
   if (Res<(-ZeroInt)) statevector->SetStateValue("ResultCode",1);
   if (fabs(Res)<=ZeroInt) statevector->SetStateValue("ResultCode",2);
  }

  float TDecider::Process(const std::vector<float>& signals)
  {                   // here also the Classification state could be queried
    if (statevector->GetStateValue("BeginOfTrial")==1) {
       ResultValue = 0;
       CoeffNum = 0;
       statevector->SetStateValue("ResultCode",0);
    }
    if (statevector->GetStateValue("Classification")==1) {
       switch (DecisionMode) {
           case 0: { ResultValue = signals[DecisionCh];      // no decision is made
                     statevector->SetStateValue("ResultCode",0);
                    }
                    break;
           case 1: if (statevector->GetStateValue("EndOfClass")==1) {     // last position is counting
                       ResultValue = signals[DecisionCh];
                       SetResultStates(ResultValue);
                   }
                   break;
           case 2: {
                     ResultValue = (ResultValue*CoeffNum +signals[DecisionCh])/(CoeffNum+1);
                     ++CoeffNum;
                     SetResultStates(ResultValue);
                   }
                   break;
           case 3: {
                     ResultValue = signals[DecisionCh];
                     if (ResultValue<=NegThreshold) {
                        statevector->SetStateValue("ResultCode",1);
                        statevector->SetStateValue("EndOfClass",1);
                     }
                     if (ResultValue>=PosThreshold) {
                        statevector->SetStateValue("ResultCode",4);
                        statevector->SetStateValue("EndOfClass",1);
                     }
                     if ((statevector->GetStateValue("EndOfClass")==1) && (ResultValue>NegThreshold) && (ResultValue<PosThreshold)) {
                        statevector->SetStateValue("ResultCode",2);
                     }
                   }
                   break;
           case 4: {
                     ResultValue = signals[DecisionCh];
                     if (ResultValue<=NegThreshold) {
                        ++CoeffNum;
                        if (CoeffNum==DecisionTime) {
                           statevector->SetStateValue("ResultCode",1);
                           statevector->SetStateValue("EndOfClass",1);
                        }
                     }
                     if (ResultValue>=PosThreshold) {
                        ++CoeffNum;
                        if (CoeffNum==DecisionTime) {
                           statevector->SetStateValue("ResultCode",4);
                           statevector->SetStateValue("EndOfClass",1);
                        }
                     }
                     if ((ResultValue>NegThreshold) && (ResultValue<PosThreshold)) {
                        CoeffNum = 0;
                        if (statevector->GetStateValue("EndOfClass")==1) statevector->SetStateValue("ResultCode",2);
                     }
                   }
                   break;
       } // end switch
       if (statevector->GetStateValue("EndOfClass")==1) {
           Vote();
       }
    }   // end if
    return ResultValue;
  }     // end function

  void TDecider::Vote()
  {
   int aux = 0;  // 0 no, -1 wrong, 1 right
   if (statevector->GetStateValue("Artifact")==0) {
      if ((statevector->GetStateValue("TargetCode")  & 4) && (statevector->GetStateValue("ResultCode") & 4)) {
         ++PosOK;
         aux = 1;
      }
      if ((statevector->GetStateValue("TargetCode")  & 4) && !(statevector->GetStateValue("ResultCode") & 4)) {
         ++Posf;
         aux = -1;
      }
      if ((statevector->GetStateValue("TargetCode")  & 1) && (statevector->GetStateValue("ResultCode") & 1)) {
         ++NegOK;
         aux = 1;
      }
      if ((statevector->GetStateValue("TargetCode")  & 1) && !(statevector->GetStateValue("ResultCode") & 1)) {
         ++Negf;
         aux = -1;
      }
      if ((statevector->GetStateValue("TargetCode")  & 2) && (statevector->GetStateValue("ResultCode") & 2)) {
         ++IDFOK;
         aux = 1;
      }
      if ((statevector->GetStateValue("TargetCode")  & 2) && !(statevector->GetStateValue("ResultCode") & 2)) {
         ++IDFf;
         aux = -1;
      }
      if (aux==1) ++Right;
      if (aux==-1) ++Wrong;
   }
   else ++Invalid;
  }

  void TDecider::GetVote(int &GetRight, int &GetWrong, int &GetInvalid)
  {
   GetRight = Right;
   GetWrong = Wrong;
   GetInvalid = Invalid;
  }

  void TDecider::GetVote(int &GetPosOK, int &GetPosf, int &GetNegOK, int &GetNegf, int &GetIDFOK, int &GetIDFf)
  {
   GetPosOK = PosOK;
   GetPosf = Posf;
   GetNegOK = NegOK;
   GetNegf = Negf;
   GetIDFOK = IDFOK;
   GetIDFf = IDFf;
  }

  void TDecider::ResetVote()
  {
   PosOK = 0;
   Posf = 0;
   NegOK = 0;
   Negf = 0;
   IDFOK = 0;
   IDFf = 0;
   Right = 0;
   Wrong = 0;
   Invalid = 0; 
  }

// --------------TaskManager for setting the task for the following dataset -------------------------------------

  TTaskManager::TTaskManager(PARAMLIST *Newparamlist, STATELIST *statelist)
  {
    char line[255];
    paramlist = Newparamlist;

    statelist->AddState2List("TargetCode 4 0 0 0\n");
    strcpy(line, "Sequencer int TaskBegin= 0.0 0.0 0 60 // begin of task in s");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Sequencer int TaskMode= 1 1 0 3 // 0: no, 1: random, 2: sequence, 3 speller");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Sequencer intlist TaskSequence= 16 4 1 4 4 1 1 4 1 4 4 1 4 1 1 1 4 4 0 15 // sequence of tasks");
    paramlist->AddParameter2List(line, strlen(line));
  }

 void TTaskManager::Initialize(PARAMLIST *Newparamlist, STATEVECTOR *NewStateVector)
 {
    statevector = NewStateVector;
    paramlist = Newparamlist;
    int BS = atoi(paramlist->GetParamPtr("SamplingRate")->GetValue())/atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
    TaskBegin = atof(paramlist->GetParamPtr("TaskBegin")->GetValue())*BS;
    TaskMode = atoi(paramlist->GetParamPtr("TaskMode")->GetValue());
    SeqLength = paramlist->GetParamPtr("TaskSequence")->GetNumValues();
    SeqPosition = 0;
    PosInTrial = -1;
 }

 void TTaskManager::Process(bool StartFlag)
 {
    if (PosInTrial>-1) ++PosInTrial;
    if (StartFlag) PosInTrial = 0;
    if (statevector->GetStateValue("BeginOfTrial")==1) {
       statevector->SetStateValue("TargetCode",0);
       PosInTrial=0;
    }
    if (PosInTrial==TaskBegin) SetTask();
 }

 void TTaskManager::SetTask()
 {
  if (TaskMode==1) {
     int task;
     task = rand() % 100;
     if (task<50) {
        statevector->SetStateValue("TargetCode",4);
     }
     else {
        statevector->SetStateValue("TargetCode",1);
     }
  }
  if (TaskMode==2) {
     if (SeqPosition >= SeqLength) SeqPosition = 0;
     statevector->SetStateValue("TargetCode",atoi(paramlist->GetParamPtr("TaskSequence")->GetValue(SeqPosition)));
     ++SeqPosition;
  }
 }

 // --------------SessionManager for organizing a run -------------------------------------

  TSessionManager::TSessionManager(PARAMLIST *paramlist, STATELIST *statelist)
  {
    char line[255];

    statelist->AddState2List("BaselineInterval 1 0 0 0\n");
    statelist->AddState2List("FeedbackInterval 1 0 0 0\n");
    statelist->AddState2List("InterTrialInterval 1 0 0 0\n");
    statelist->AddState2List("EndOfTrial 1 0 0 0\n");
    statelist->AddState2List("BeginOfTrial 1 0 0 0\n");

    strcpy(line, "Sequencer int TrialsARun= 70 70 1 10000 // number of trials in one run");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Sequencer float BIPts= 2.0 2.0 0 60 // Baseline-Intervall in s");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Sequencer float FIPts= 5.0 5.0 0 60 // Feedback-Intervall in s");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Sequencer float ITIPts= 0.0 0.0 0 60 // ITI-Intervall in s");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Sequencer float StartAfterTrig= 0.0 0.0 0 60 // Time to start Trial after Trigger in s");
    paramlist->AddParameter2List(line, strlen(line));
    strcpy(line, "Sequencer int TrialStartMode= 0 0 0 1 // 0: Trial starts after ITI, 1: Trial starts after StartAfterTrig s");
    paramlist->AddParameter2List(line, strlen(line));
  }

 void TSessionManager::Initialize(PARAMLIST *paramlist, STATEVECTOR *NewStateVector, TSTATUS *NewStatus)
 {
    statevector = NewStateVector;
    status = NewStatus;
    TrialsARun = atoi(paramlist->GetParamPtr("TrialsARun")->GetValue());
    int BS = atoi(paramlist->GetParamPtr("SamplingRate")->GetValue())/atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
    BIPts = atof(paramlist->GetParamPtr("BIPts")->GetValue())*BS;
    FIPts = atof(paramlist->GetParamPtr("FIPts")->GetValue())*BS;
    ITIPts = atof(paramlist->GetParamPtr("ITIPts")->GetValue())*BS;
    StartAfterTrig = atof(paramlist->GetParamPtr("StartAfterTrig")->GetValue())*BS;
    TrialStartMode = atoi(paramlist->GetParamPtr("TrialStartMode")->GetValue());
    PosAfterTrig = -1;
    status->TrialStatus = -1;
    PosInTrial = -1;
 }

 void TSessionManager::Process(bool StartFlag)
 {
    ++PosInTrial;
    if ((TrialStartMode == 1) && (PosAfterTrig>0)) PosAfterTrig++;
    if (StartFlag) {
          status->TrialStatus = 0;
          PosInTrial = 0;
          PosAfterTrig = 0;
    }
  /*  if (TrialStartMode == 1) {
       if ((statevector->GetStateValue("Trigger")==1) && (PosAfterTrig<=0)) PosAfterTrig = 1;
       if (PosAfterTrig>StartAfterTrig) {
           PosInTrial = 0;
           PosAfterTrig = 0;
       }
    }
    else {  */
        if (PosInTrial>=(BIPts+FIPts+ITIPts)) PosInTrial = 0;
  //  }
    if (PosInTrial==0) {
        statevector->SetStateValue("BeginOfTrial",1);
        statevector->SetStateValue("EndOfTrial",0);
        statevector->SetStateValue("InterTrialInterval",0);
        statevector->SetStateValue("FeedbackInterval",0);
        statevector->SetStateValue("BaselineInterval",1);
        if ((status->TrialStatus>=TrialsARun) && (status->SourceStatus>=DAS1402)) {
          statevector->SetStateValue("BeginOfTrial",0);
          statevector->SetStateValue("BaselineInterval",0);
          status->SysStatus = STOP;
          statevector->SetStateValue("Running",0);
          statevector->SetStateValue("Recording",0);
        }
        ++status->TrialStatus;
    }
    if (PosInTrial>=1) statevector->SetStateValue("BeginOfTrial",0);
    if (PosInTrial==BIPts) {
        statevector->SetStateValue("BaselineInterval",0);
        statevector->SetStateValue("FeedbackInterval",1);
    }
    if (PosInTrial==(BIPts+FIPts-1)) statevector->SetStateValue("EndOfTrial",1);
    if (PosInTrial==(BIPts+FIPts)) {
        statevector->SetStateValue("EndOfTrial",0);
        statevector->SetStateValue("FeedbackInterval",0);
        statevector->SetStateValue("InterTrialInterval",1);
    }
 }
//---------------------------------------------------------------------------
//--------------------------- Sequencing routine -----------------------
TClassSequencer::TClassSequencer(PARAMLIST *paramlist, STATELIST *statelist)
{
   char line[255];

   statelist->AddState2List("Classification 1 0 0 0\n");
   statelist->AddState2List("EndOfClass 1 0 0 0\n");

   strcpy(line, "Sequencer int FBBegin= 2.0 2.0 0 60 // Begin of cursor movement in s");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Sequencer int FBEnd= 6.5 6.5 0 60 // End of cursor movement in s");
   paramlist->AddParameter2List(line, strlen(line));
   Initialized = false;

}

void TClassSequencer::Initialize(PARAMLIST *paramlist, STATEVECTOR *Newstatevector)
{
  statevector = Newstatevector;

  int BS = atoi(paramlist->GetParamPtr("SamplingRate")->GetValue())/atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
  ClBeg = atof(paramlist->GetParamPtr("FBBegin")->GetValue())*BS;
  ClEnd = (atof(paramlist->GetParamPtr("FBEnd")->GetValue())-0.001)*BS;
  PosInTrial = -1;
  Initialized = true;
}


void TClassSequencer::Process(bool StartFlag)
{
 bool ret = false;
 ++PosInTrial;
 if ((statevector->GetStateValue("BeginOfTrial")==1) || (StartFlag)) {
    PosInTrial = 0;
 }
 if ((PosInTrial==0) || (PosInTrial>=ClEnd)) {
    statevector->SetStateValue("Classification",0);
    statevector->SetStateValue("EndOfClass",0);
 }
 int TimeCoeff = PosInTrial-ClBeg;
 if (TimeCoeff==0) statevector->SetStateValue("Classification",1);
 if ((PosInTrial==(ClEnd-1)) && (statevector->GetStateValue("Classification")))
     statevector->SetStateValue("EndOfClass",1);
}

