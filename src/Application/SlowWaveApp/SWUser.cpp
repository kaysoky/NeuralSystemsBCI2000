//---------------------------------------------------------------------------
#include "PCHIncludes.h"
#pragma hdrstop

#include <stdio.h>
#ifdef BCI2000_STRICT
# include <assert>
#endif // BCI2000_STRICT

#include "UState.h"
#include "SWUser.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFBForm *FBForm;
//---------------------------------------------------------------------------

#ifdef BCI2000_STRICT
// For consistent values, we simply set the Ball's left coordinate to
// initialBallLeft whenever PosInTrial is reset.
const int initialBallLeft = 40;
#endif // BCI2000_STRICT

__fastcall TFBForm::TFBForm(TComponent* Owner)
        : TForm(Owner)
#ifdef BCI2000_STRICT
        , outHandle( NULL )
#endif // !BCI2000_STRICT
{
}

void TFBForm::SetFBForm(PARAMLIST *NewParamlist, STATELIST *statelist) {

   char line[255];
   paramlist = NewParamlist;
   statelist->AddState2List("Feedback 1 0 0 0\n");

   strcpy(line, "Presentation int VisOptionMode= 1 1 0 3 // 0 no choice, 1 Task, 2 Speller, 3 Question");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int AudOptionMode= 0 1 0 3 // 0 no choice, 1 Task, 2 Speller, 3 Question");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int VisFBMode= 1 1 0 3 // 0 no FB, 1 Ball, 2 Bar");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int AudFBMode= 0 2 0 2 // 0 no acoustic FB, 1 half tone, 2 Dur");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int VisTaskMode= 1 1 0 3 // 0 no task, 1 light goal");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int AudTaskMode= 0 1 0 3 // 0 no auditory task, 1 Midi, 2 Wav, 3 oddball");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int VisResultMode= 1 1 0 2 // 0 no visual result, 1 blink goal");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int AudResultMode= 0 1 0 2 // 0 no auditory result, 1 Midi, 2 Wav");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int VisReinforcement= 1 1 0 1 // 0 no visual reinforce, 1 smiley");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int AudReinforcement= 0 1 0 2 // 0 no auditory reinforce, 1 Midi, 2 Wav");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int VisMarker= 0 0 0 1 // 0 no visual marker");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int AudMarker= 0 2 0 3 // 0 no auditory marker, & 1: BITick, & 2: FITack");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int VisInvalid= 1 1 0 1 // 0 no visual invalid indicator 1 cross");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Presentation int AudInvalid= 0 1 0 1 // 0 no auditory invalid indicator 1 GMIVNote");
   paramlist->AddParameter2List(line, strlen(line));



   strcpy(line, "Sequencer int FBBegin= 2.0 2.0 0 60 // Begin of cursor movement in s");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "Sequencer int FBEnd= 6.5 6.5 0 60 // End of cursor movement in s");
   paramlist->AddParameter2List(line, strlen(line));
// Sound Parameters
   strcpy(line, "PresentGM float GMFBInterval= 0.2 0.2 1 10 // Interval multiplier for acoustic FB");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMFBInstrument= 1 1 1 128 // Instrument for acoustic FB");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMFBVelocity= 70 70 0 127 // Velocity for acoustic FB");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMFBCenterNote= 69 69 0 127 // Note at zero position for acoustic FB (69: 440 Hz)");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMBIInstrument= 14 14 1 128 // Instrument for initial BI signal");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMBIVelocity= 100 100 0 127 // Velocity for initial BI signal");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMBINote= 88 88 0 127 // Note at zero position for BI signal (88: 1318 Hz (E))");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMFIInstrument= 14 14 1 128 // Instrument for initial FI signal");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMFIVelocity= 100 100 0 127 // Velocity for initial FI signal");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMFINote= 72 72 0 127 // Note at zero position for FI signal (72: 523 Hz (C))");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMRIInstrument= 10 10 1 128 // Instrument for Reinforcement");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMRIVelocity= 70 70 0 127 // Velocity for Reinforcement");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMIVInstrument= 57 57 1 128 // Instrument for invalid trial");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMIVVelocity= 100 100 0 127 // Velocity for invalid trial");
   paramlist->AddParameter2List(line, strlen(line));
   strcpy(line, "PresentGM int GMIVNote= 36 36 0 127 // Note for invalid trial)");
   paramlist->AddParameter2List(line, strlen(line));

#ifndef BCI2000_STRICT
   unsigned int GMdeviceID = GMDEVICEID;
   result = midiOutOpen(&outHandle, GMdeviceID, NULL, NULL, CALLBACK_NULL);
#endif // !BCI2000_STRICT
   STree = new TSTree();
   STree->CurIndex = 0;
   STree->MaxIndex = 0;
   STree->WrittenText = "";
   for (int n=0; n<255; n++) {
       STree->Caption[n] = " ";
       STree->OnSelect[n] = 0;
       STree->OnReject[n] = 0;
       STree->OnNoChoice[n] = 0;
   }
   PosInTrial = -1;
#ifdef BCI2000_STRICT
   Ball->Left = initialBallLeft;
#endif // BCI2000_STRICT
   // defining oddballstr
   OddballStr[0] = "Odd1.wav";
   OddballStr[1] = "Odd2.wav";
   OddballStr[2] = "Odd3.wav";
   OddballStr[3] = "Odd4.wav";
   OddballStr[4] = "Odd5.wav";
   OddballStr[5] = "Odd6.wav";
   OddballStr[6] = "Odd7.wav";
   OddballStr[7] = "Odd8.wav";
   OddballStr[8] = "Odd9.wav";
   OddballStr[9] = "Odd10.wav";
}

//---------------------------------------------------------------------------
void __fastcall TFBForm::FormDestroy(TObject *Sender)
{
 delete STree;
 midiOutClose(outHandle);
}
//---------------------------------------------------------------------------

void TFBForm::ReInitialize() //Initialize after Resize
{
  PARAM *paramptr;
  int NegTh = -80;
  int PosTh = 80;
  float ZeroInt = 0.5;
  if (paramlist) {
      paramptr=paramlist->GetParamPtr("NegThreshold");
      if (paramptr) NegTh = atoi(paramptr->GetValue());
      else NegTh = -80;
      paramptr=paramlist->GetParamPtr("PosThreshold");
      if (paramptr) PosTh = atoi(paramptr->GetValue());
      else PosTh = 80;
      paramptr=paramlist->GetParamPtr("ZeroInterval");
      if (paramptr) ZeroInt = atof(paramptr->GetValue());
      else ZeroInt = 0.5;
  }
  UpperGoal->Height = ((100+NegTh)*ClientHeight/200)-8;
  LowerGoal->Top = ((100+PosTh)*ClientHeight)/200;
  LowerGoal->Height = ClientHeight - LowerGoal->Top - 8;
  ZeroBar->Height = (2*ZeroInt*ClientHeight)/200;
  ZeroBar->Top = ClientHeight/2 - (ZeroInt*ClientHeight)/200;
}

#ifdef BCI2000_STRICT
void
TFBForm::LeaveRunningState()
{
  const DWORD midiMsgAllNotesOff = 0x7b; // Little endian byte ordering assumed here.
  if( outHandle )
    midiOutShortMsg( outHandle, midiMsgAllNotesOff );
}
#endif // !BCI2000_STRICT


void TFBForm::Initialize(STATEVECTOR *Newstatevector)
{
#ifdef BCI2000_STRICT
  if( outHandle )
    midiOutClose( outHandle );
  midiOutOpen(&outHandle, MIDI_MAPPER, NULL, NULL, CALLBACK_NULL);
#endif // !BCI2000_STRICT
  statevector = Newstatevector;
  int NegTh = atoi(paramlist->GetParamPtr("NegThreshold")->GetValue());
  int PosTh = atoi(paramlist->GetParamPtr("PosThreshold")->GetValue());
  float ZeroInt = atof(paramlist->GetParamPtr("ZeroInterval")->GetValue());
  UpperGoal->Height = ((100+NegTh)*ClientHeight/200)-8;
  LowerGoal->Top = ((100+PosTh)*ClientHeight)/200;
  LowerGoal->Height = ClientHeight - LowerGoal->Top - 8;
  ZeroBar->Height = (2*ZeroInt*ClientHeight)/200;
  ZeroBar->Top = ClientHeight/2 - (ZeroInt*ClientHeight)/200;

  VisOptionMode = atoi(paramlist->GetParamPtr("VisOptionMode")->GetValue());
  AudOptionMode = atoi(paramlist->GetParamPtr("AudOptionMode")->GetValue());
  VisReinforcement = atoi(paramlist->GetParamPtr("VisReinforcement")->GetValue());
  AudReinforcement = atoi(paramlist->GetParamPtr("AudReinforcement")->GetValue());
  VisTaskMode = atoi(paramlist->GetParamPtr("VisTaskMode")->GetValue());
  AudTaskMode = atoi(paramlist->GetParamPtr("AudTaskMode")->GetValue());
  VisResultMode = atoi(paramlist->GetParamPtr("VisResultMode")->GetValue());
  AudResultMode = atoi(paramlist->GetParamPtr("AudResultMode")->GetValue());
  VisMarker = atoi(paramlist->GetParamPtr("VisMarker")->GetValue());
  AudMarker = atoi(paramlist->GetParamPtr("AudMarker")->GetValue());
  VisFBMode = atoi(paramlist->GetParamPtr("VisFBMode")->GetValue());
  AudFBMode = atoi(paramlist->GetParamPtr("AudFBMode")->GetValue());
  VisInvalid = atoi(paramlist->GetParamPtr("VisInvalid")->GetValue());
  AudInvalid = atoi(paramlist->GetParamPtr("AudInvalid")->GetValue());

  int BS = atoi(paramlist->GetParamPtr("SamplingRate")->GetValue())/atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
  FBBegin = atof(paramlist->GetParamPtr("FBBegin")->GetValue())*BS;
  FBEnd = (atof(paramlist->GetParamPtr("FBEnd")->GetValue())-0.001)*BS;

  GMFBInterval = atof(paramlist->GetParamPtr("GMFBInterval")->GetValue());
  GMFBInstrument = atoi(paramlist->GetParamPtr("GMFBInstrument")->GetValue())-1;
  GMFBVelocity = atoi(paramlist->GetParamPtr("GMFBVelocity")->GetValue());
  GMFBCenterNote = atoi(paramlist->GetParamPtr("GMFBCenterNote")->GetValue());
  GMBIInstrument = atoi(paramlist->GetParamPtr("GMBIInstrument")->GetValue())-1;
  GMBIVelocity = atoi(paramlist->GetParamPtr("GMBIVelocity")->GetValue());
  GMBINote = atoi(paramlist->GetParamPtr("GMBINote")->GetValue());
  GMFIInstrument = atoi(paramlist->GetParamPtr("GMFIInstrument")->GetValue())-1;
  GMFIVelocity = atoi(paramlist->GetParamPtr("GMFIVelocity")->GetValue());
  GMFINote = atoi(paramlist->GetParamPtr("GMFINote")->GetValue());
  GMRIInstrument = atoi(paramlist->GetParamPtr("GMRIInstrument")->GetValue())-1;
  GMRIVelocity = atoi(paramlist->GetParamPtr("GMRIVelocity")->GetValue());
  GMIVInstrument = atoi(paramlist->GetParamPtr("GMIVInstrument")->GetValue())-1;
  GMIVVelocity = atoi(paramlist->GetParamPtr("GMIVVelocity")->GetValue());
  GMIVNote = atoi(paramlist->GetParamPtr("GMIVNote")->GetValue());
  result = midiOutShortMsg(outHandle, GMFBInstrument*256 + 12*16 + GMFBChannel);
  result = midiOutShortMsg(outHandle, GMBIInstrument*256 + 12*16 + GMBIChannel);
  result = midiOutShortMsg(outHandle, GMFIInstrument*256 + 12*16 + GMFIChannel);
  result = midiOutShortMsg(outHandle, GMRIInstrument*256 + 12*16 + GMRIChannel);
  result = midiOutShortMsg(outHandle, GMIVInstrument*256 + 12*16 + GMIVChannel);
  if (AudFBMode>0) GMTimer->Enabled = true;
  else GMTimer->Enabled = false;
  ProCounter = -1;
  TaskActive = false;
  BIState = false;
  PosInTrial = -1;
#ifdef BCI2000_STRICT
  Ball->Left = initialBallLeft;
  Ball->Top = ZeroPosition;
#endif // BCI2000_STRICT
}

void __fastcall TFBForm::FormResize(TObject *Sender)
{
 UpperGoal->Width = ClientWidth-16;
 LowerGoal->Width = ClientWidth-16;
 LowerGoal->Top = ClientHeight-LowerGoal->Height-8;
 ZeroBar->Width = ClientWidth-16;
 ZeroBar->Top = (ClientHeight-ZeroBar->Height)/2;
 Ball->Height = ClientHeight/10;
 Ball->Width = Ball->Height;
 ZeroPosition = ClientHeight/2 - Ball->Height/2;
 MiddleGoal->Top = ClientHeight/2-MiddleGoal->Height/2;
 MiddleGoal->Left = ClientWidth-8-MiddleGoal->Width;
 PB1->Width = 200;
 PB1->Height = 200;
 PB1->Top = ClientHeight/2-PB1->Height/2;
 PB1->Left = ClientWidth/2-PB1->Width/2;
 Ball->Top = ZeroPosition;
 BottomText->Top = LowerGoal->Top + 12;
 BottomText->Left = (ClientWidth-BottomText->Width)/2;
 ReInitialize();
}


void __fastcall TFBForm::FormCreate(TObject *Sender)
{
 UpperGoal->Width = ClientWidth-16;
 LowerGoal->Width = ClientWidth-16;
 LowerGoal->Top = ClientHeight-LowerGoal->Height-8;
 ZeroBar->Width = ClientWidth-16;
 ZeroBar->Top = (ClientHeight-ZeroBar->Height)/2;
 ZeroPosition = ClientHeight/2 - Ball->Height/2;
 MiddleGoal->Top = ClientHeight/2-MiddleGoal->Height/2;
 MiddleGoal->Left = ClientWidth-8-MiddleGoal->Width;
#ifdef BCI2000_STRICT
  Ball->Left = initialBallLeft;
#endif // BCI2000_STRICT
 Ball->Top = ZeroPosition;
 BottomText->Width = 600;
 TopText->Visible = false;
 BottomText->Visible = false;
 Visible = false;
}

void TFBForm::ShowTask()
{
 int TargetCode = statevector->GetStateValue("TargetCode");
 if (TargetCode!=0) TaskActive = true;
// show visual task
 if (VisOptionMode==1) {
     if (TargetCode  & 4) {
         //------ for AB Epitraining ----------
         // ABLabel->Caption = "B";
         // ABLabel->Visible = true;
         //------------------------------------
         LowerGoal->Brush->Color = clBlue;
         LowerGoal->Pen->Color = clYellow;
         LowerGoal->Update();
     }
     if (TargetCode  & 2) {
         //------ for AB Epitraining ----------
         //ABLabel->Visible = false;
         //------------------------------------
         MiddleGoal->Brush->Color = clBlue;
         MiddleGoal->Pen->Color = clYellow;
         MiddleGoal->Update();
     }
     if (TargetCode  & 1) {
         //------ for AB Epitraining ----------
         // ABLabel->Caption = "A";
         // ABLabel->Visible = true;
         //------------------------------------
         UpperGoal->Brush->Color = clBlue;
         UpperGoal->Pen->Color = clYellow;
         UpperGoal->Update();
     }
  }
  if (VisOptionMode==2) ShowLetters2Select();
// show auditory task
  if (AudOptionMode==1) {
     int Note = 0;
     if (TargetCode  & 4) {
         switch (AudTaskMode) {
             case 1: Note = CalcAcNote(atoi(paramlist->GetParamPtr("PosThreshold")->GetValue())); break;
             case 2: MediaPlayer4->Play();
                     break;
             case 3: MediaPlayer4->Play(); // oddball4.wav is played
                     break;
         }
     }
     if (TargetCode  & 2) {
          switch (AudTaskMode) {
             case 1: Note = CalcAcNote(0); break;
             case 2: MediaPlayer2->Play();
                     break;
             case 3: MediaPlayer2->Play(); // oddball2 is played
                     break;
         }
     }
     if (TargetCode  & 1) {
          switch (AudTaskMode) {
             case 1: Note = CalcAcNote(atoi(paramlist->GetParamPtr("NegThreshold")->GetValue())); break;
             case 2: MediaPlayer1->Play();
                     break;
             case 3: MediaPlayer1->Play(); // odd1.wav - odd10.wav is played
                     break;
         }
     }
     if ((TargetCode!=0) && (AudTaskMode==1)) {      // play the Note
         result = midiOutShortMsg(outHandle, GMFBVelocity*65536 + Note*256 + 9*16 + GMFBChannel);
         Sleep(50);
         result = midiOutShortMsg(outHandle, Note*256 + 9*16 + GMFBChannel);
     }
  }
}

void TFBForm::ShowLetters2Select()
{
   BottomText->Caption = STree->Caption[STree->CurIndex];
   BottomText->Visible = true;
   TaskActive = true;
}

void TFBForm::ShowResult()
{
     if (VisResultMode==1) {
         if (statevector->GetStateValue("ResultCode") & 4) {
             LowerGoal->Brush->Color = clGreen;
         }
         if (statevector->GetStateValue("ResultCode") & 1) {
             UpperGoal->Brush->Color = clGreen;
         }
         if (statevector->GetStateValue("ResultCode") & 2) {
             MiddleGoal->Brush->Color = clGreen;
         }
     }
     if (statevector->GetStateValue("Artifact")==1) Invalid();
     else {
         if ((VisOptionMode==1) || AudOptionMode==1) {     // Check for Reinforce only if there was a task
		if (((statevector->GetStateValue("TargetCode")  & 4) && (statevector->GetStateValue("ResultCode") & 4))
		 || ((statevector->GetStateValue("TargetCode")  & 1) && (statevector->GetStateValue("ResultCode") & 1))
		 || ((statevector->GetStateValue("TargetCode")  & 2) && (statevector->GetStateValue("ResultCode") & 2)))
                 Reinforce();
         }
         if (VisOptionMode==2) {   // Spellermode
	     if (statevector->GetStateValue("ResultCode") & 4) { // if the item is selected:
	        if (STree->Caption[STree->CurIndex][1]=='#') {     // letter is written
	       	    AnsiString AStr = STree->Caption[STree->CurIndex];
		    AStr.Delete(1,1);
		    STree->WrittenText += AStr;
		    TopText->Caption = STree->WrittenText;
		    TopText->Visible = true;
                }
		if (STree->Caption[STree->CurIndex][2]=='ö') {   // delete last letter
		    STree->WrittenText.Delete(STree->WrittenText.Length(),1);
		    TopText->Caption = STree->WrittenText;
                }
		STree->CurIndex = STree->OnSelect[STree->CurIndex];
             }
	     if (statevector->GetStateValue("ResultCode")==1) STree->CurIndex = STree->OnReject[STree->CurIndex];
			// if (statevector->GetStateValue("Result0")==1) STree->CurIndex = STree->OnNoChoice[STree->CurIndex];
         }
     }
}

int TFBForm::CalcAcNote(int FBValue)
{
        int AcNote = 0;
        switch (AudFBMode) {
           case 1: AcNote = GMFBCenterNote + int (-GMFBInterval*FBValue); break;
           case 2: AcNote = GMFBCenterNote + ceil(12.0/7.0*(int (-GMFBInterval*FBValue))-0.2); break;
        }
        if (AcNote<0) AcNote = 0;
        if (AcNote>127) AcNote = 127;
        return AcNote;
}

void TFBForm::ShowFB(int FBValue)
{
  static int AcNote = 0;

  if (statevector->GetStateValue("BeginOfTrial")==1) {
      Ball->Visible = true;
      Ball->Left = 40;
#ifdef BCI2000_STRICT
      Ball->Left = initialBallLeft;
#endif // BCI2000_STRICT
      Ball->Top =  ZeroPosition;
  }
  if (PosInTrial==FBBegin) statevector->SetStateValue("Feedback", 1);
  if (PosInTrial==(FBEnd+1)) {
     statevector->SetStateValue("Feedback", 0);
     result = midiOutShortMsg(outHandle, AcNote*256 + 9*16 + GMFBChannel);
  }
  if (statevector->GetStateValue("Feedback")==1) {
     if (AudFBMode>0) {
        result = midiOutShortMsg(outHandle, AcNote*256 + 9*16 + GMFBChannel);  // Turn off old tone
        AcNote = CalcAcNote(FBValue);
     /*   if (FBValue<0) result = midiOutShortMsg(outHandle, 0 + 8);
        else result = midiOutShortMsg(outHandle, 16000*256 + 8); // SetBalance  */
        result = midiOutShortMsg(outHandle, GMFBVelocity*65536 + AcNote*256 + 9*16 + GMFBChannel); // Turn on new tone
     }
     if ((VisFBMode==1) && (Visible)) {
         XStepSize = ( ClientWidth-100)/(FBEnd-FBBegin);
         Ball->Left += XStepSize;
         if (FBValue<-100) FBValue = -100;
         if (FBValue>100) FBValue = 100;
         FBValue = FBValue * ClientHeight/200;
         Ball->Top =  ZeroPosition + FBValue;
         Ball->Update();
     }
  }
}

void TFBForm::Process(int FBValue)
{
  ++PosInTrial;
  if (ProCounter++>10000) ProCounter = 0;
  if (statevector->GetStateValue("BeginOfTrial")==1) {
      PosInTrial = 0;
#ifdef BCI2000_STRICT
      Ball->Left = initialBallLeft;
#endif // BCI2000_STRICT
      BIState = true;
      // acoustic signal for BI begin
      if (AudMarker & 1) {  // Tick
          result = midiOutShortMsg(outHandle, GMBIVelocity*65536 + GMBINote*256 + 9*16 + GMBIChannel);
          Sleep(1);
          result = midiOutShortMsg(outHandle, GMBINote*256 + 9*16 + GMBIChannel);
      }
      PB1->Refresh();
  }
  Label1->Caption = AnsiString(PosInTrial);
  if (statevector->GetStateValue("FeedbackInterval")==1) {
      if (BIState) {
         if (AudMarker & 2) {            // acoustic signal for FI begin Tack
             result = midiOutShortMsg(outHandle, GMFIVelocity*65536 + GMFINote*256 + 9*16 + GMFIChannel);
             Sleep(1);
             result = midiOutShortMsg(outHandle, GMFINote*256 + 9*16 + GMFIChannel);
         }
         BIState = false;
      }
  }
  if (!(statevector->GetStateValue("TargetCode")  & 8)) ShowFB(FBValue);
  else Ball->Visible = false;
  if (statevector->GetStateValue("EndOfClass")==1) ShowResult();
  if (TaskActive) {     // reset the visual target
      if ((statevector->GetStateValue("TargetCode")==0) || (statevector->GetStateValue("BeginOfTrial")==1)) {
         TaskActive = false;
         LowerGoal->Brush->Color = clPurple;
         LowerGoal->Pen->Color = clGreen;
         MiddleGoal->Brush->Color = clPurple;
         MiddleGoal->Pen->Color = clGreen;
         UpperGoal->Brush->Color = clPurple;
         UpperGoal->Pen->Color = clGreen;
         Update();
      }
  }
  if (!TaskActive) ShowTask();
}

void __fastcall TFBForm::GMTimerTimer(TObject *Sender)
{
 static int OldCounter;
 if ((ProCounter == OldCounter) && (ProCounter!=-1)) {
    ProCounter = -1;
    for (int i=0; i<128; i++) result = midiOutShortMsg(outHandle, i*256 + 9*16 + GMFBChannel);
 }
 OldCounter = ProCounter;
}
//---------------------------------------------------------------------------

void TFBForm::Invalid() // Handle Visual and Auditory Invalid indication
{
    if (VisInvalid==1) {
	TCanvas *pCanvas =  PB1->Canvas;
	pCanvas->Pen->Color = clRed;
	pCanvas->Brush->Color = clRed;
	pCanvas->Pen->Width = 20;
	pCanvas->MoveTo(0,0);
	pCanvas->LineTo( PB1->Width,  PB1->Height);
	pCanvas->MoveTo( PB1->Width,0);
	pCanvas->LineTo(0,  PB1->Height);
    }
    if (AudInvalid==1) result = midiOutShortMsg(outHandle, GMIVVelocity*65536 + GMIVNote*256 + 9*16 + GMIVChannel);
    if ((VisInvalid==1) || (AudInvalid==1)) Sleep(300);
    if (AudInvalid==1) result = midiOutShortMsg(outHandle, GMIVNote*256 + 9*16 + GMIVChannel);
    if (VisInvalid==1) PB1->Refresh();
}

void TFBForm::Reinforce()   // Handle Visual and Auditory Reinforcement
{
    TCanvas *pCanvas =  PB1->Canvas;
    unsigned int phrase[] = {0x3C, 0x40, 0x43, 0x3C, 0x43, 0x48, 0x3C, 0x43, 0x4C};
    if (VisReinforcement==1) {
        pCanvas->Pen->Width = 1;
#if 0 // jm
        PB1->Top =  Height/2 -  PB1->Height/2;
        PB1->Left =  Width/2 -  PB1->Width/2;
        PB1->Width = 200;
        PB1->Height = 200;
#endif
        //the face:
        pCanvas->Brush->Color = clYellow;
        pCanvas->Pen->Color = clYellow;
        pCanvas->Ellipse(0, 0,  PB1->Width,  PB1->Height);
        //the mouth:
        pCanvas->Brush->Color = TColor(0x000080FF);
        pCanvas->Pen->Color = TColor(0x000080FF);
        pCanvas->Arc(33, 33, 167, 167, 53, 153, 147, 153);
        pCanvas->Arc(30, 37, 170, 163, 53, 153, 147, 153);
        pCanvas->FloodFill(100, 163, TColor(0x000080FF), fsBorder);
        //the eyes:
        pCanvas->Pen->Color = clBlue;
        pCanvas->Brush->Color = clBlue;
        pCanvas->Ellipse(53, 68, 77, 62);
        pCanvas->Ellipse(123, 68, 147, 62);
    }
    // smile
    int j = 0;
    int k = 0;
    int l = 0;
    for (int i = 0; i < 15; i += 3){
        if (VisReinforcement==1) {
           pCanvas->Pen->Color = clBlue;
           pCanvas->Brush->Color = clBlue;
           pCanvas->Ellipse(53, 65-i, 77, 65+i);
           pCanvas->Ellipse(123, 65-i, 147, 65+i);
           pCanvas->Brush->Color = TColor(0x000080FF);
           pCanvas->Pen->Color = TColor(0x000080FF);
           pCanvas->Arc(33, 33, 167, 167, 51 - j, 148 - k, 149 + j, 148 - k);
           pCanvas->Arc(30 - l, 37 + i, 170 + l, 163 - i, 51 - j, 148 - k, 149 + j, 148 - k);
           pCanvas->FloodFill(100, 163 - i, TColor(0x000080FF), fsBorder);
           j += 2;
           k += 5;
           l++;
        }
        if (AudReinforcement==1) {
           if (l<9) result = midiOutShortMsg(outHandle, GMRIVelocity*65536 + phrase[l]*256 + 9*16 + GMRIChannel);
           Sleep(60);
           if (l<9) result = midiOutShortMsg(outHandle, phrase[l]*256 + 9*16 + GMRIChannel);
        }
    }  
   if (VisReinforcement==1) {
    Sleep(100);
    PB1->Refresh();
   }
}

bool TFBForm::LoadTree(char *FName)
{
 return (STree->LoadTree(FName));
}


//------------------------Speller definitions-------------------------------------

// --------- Reading speller-tree parameters from speller-file ----------------------
bool TSTree::LoadTree(char *FName)
{
  char Line[255];
  FILE *PFile;
  int PIndex = 0;
  char *CPtr;
  // reads Lines of structure (Index "Caption" OnSelect OnReject OnNoChoice)
  if ((PFile = fopen(FName,"r+"))!=NULL) {
     do {
        fgets(Line, 255, PFile);
        PIndex = atoi(Line);
        if (PIndex>MaxIndex) MaxIndex = PIndex;
        CPtr = strstr(Line, "\"");
        if (CPtr!=NULL) {
           ++CPtr;
           sprintf( Line, "%s", CPtr);
           CPtr = strstr(Line, "\"");
           *CPtr = '\0';
           Caption[PIndex] = AnsiString(Line);
           CPtr = strstr(Line, "\0");
           CPtr += strlen(CPtr)+2;
           sprintf( Line, "%s", CPtr);
           OnSelect[PIndex] = atoi(Line);
           CPtr = strstr(Line, " ");
           sprintf( Line, "%s", CPtr+1);
           OnReject[PIndex] = atoi(Line);
           CPtr = strstr(Line, " ");
           sprintf( Line, "%s", CPtr+1);
           OnNoChoice[PIndex] = atoi(Line);
        }
     } while (!feof(PFile));
     fclose(PFile);
     return (true);
  }
  return (false);
}

void TSTree::ClearText()
{
  WrittenText = "";
}
//---------------------------------------------------------------------------



