//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "UBCI2000DATA.h"
#include "UParameter.h"
#include "engine.h"
#include "matrix.h"
#include "mat.h"

#include "UMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CGAUGES"
#pragma resource "*.dfm"

#define MAX_STATES      100

TfMain *fMain;
Engine *ep;


//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner) : TForm(Owner)
{
 bci2000data=NULL;
}
//---------------------------------------------------------------------------


int TfMain::InitMatlabEngine()
{
 /*
  * Start the MATLAB engine
  */
 ep = engOpen(NULL);
 if (!ep)
    {
    Application->MessageBox ("Can't start MATLAB engine", "Engwindemo.c", MB_OK);
    return(-1);
    }

 return(0);
}


void TfMain::ShutdownMatlabEngine()
{
 engClose(ep);
}


//-----------------------------------------------------------------------------
void __fastcall TfMain::ContinueClick(TObject *Sender)
{
int     ret, firstrun, lastrun, cur_run, channel, state, i;
double  cur_value_double, cur_state_double, dummy_double;
ULONG   sample, numsamples;
char    cur_statename[256], buffer[5000];
int     samplingrate, channels, cur_value, cur_state, cur_trial;
int     numstates, oldnumstates, totalsamples, cur_totalsample;
bool    consistent, exportfile, exportmatlab;
FILE    *fp;
mxArray *signal, *state_var[MAX_STATES], *run_var, *trial_var, *sample_var;
MATFile *pmat;

 exportmatlab=rExportMatlab->Checked;
 exportfile=rExportFile->Checked;

 // if we did not select to export to anything
 if ((!exportfile) && (!exportmatlab))
    {
    Application->MessageBox("You probably want to export to either Matlab or a file ...", "Warning", MB_OK);
    return;
    }


 totalsamples=GetNumSamples();
 cur_totalsample=0;
 channels=bci2000data->GetNumChannels();
 samplingrate=bci2000data->GetSampleFrequency();
 for (i=0; i < MAX_STATES; i++)
  state_var[i]=NULL;


 // initialize Batlab
 // create Matlab variables, if we want to export
 if (exportmatlab)
    {
    ret=InitMatlabEngine();
    signal = mxCreateDoubleMatrix(totalsamples, channels, mxREAL);
    mxSetName(signal, "signal");
    }

 firstrun= atoi(frun->Text.c_str());
 lastrun= atoi(lrun->Text.c_str());
 cur_trial=0;

 // consistency checks
 oldnumstates=-1;
 consistent=true;
 for (cur_run=firstrun; cur_run<=lastrun; cur_run++)
  {
  bci2000data->SetRun(cur_run);
  numsamples=bci2000data->GetNumSamples();
  numstates=bci2000data->GetStateListPtr()->GetNumStates();
  if ((numstates != oldnumstates) && (oldnumstates != -1))
     consistent=false;
  oldnumstates=numstates;
  }

 // different number of states, etc.
 if (!consistent)
    {
    Application->MessageBox("Runs are not consistent (e.g., different # states)", "Error", MB_OK);
    return;
    }

 // open destination file
 fp=NULL;
 if (exportfile)
    {
    fp=fopen(eDestinationFile->Text.c_str(), "wb");
    if (!fp)
       {
       Application->MessageBox("Error opening output ASCII file", "Error", MB_OK);
       return;
       }
    }
 if (exportmatlab)
    {
    pmat = matOpen(eDestinationFile->Text.c_str(), "w");
    if (!pmat)
       {
       Application->MessageBox("Error opening output Matlab file", "Error", MB_OK);
       return;
       }
    }

 // write header, if we export to a file
 if (exportfile)
    {
    fprintf(fp, "run trial sample ");
    for (channel=0; channel < channels; channel++)
     fprintf(fp, "ch%d ", channel+1);
    // write a state's name if we decided to save this state
    for (state=0; state < bci2000data->GetStateListPtr()->GetNumStates(); state++)
     if (cStateListBox->Checked[state])
        fprintf(fp, "%s ", bci2000data->GetStateListPtr()->GetStatePtr(state)->GetName());
    fprintf(fp, "\r\n");
    }
 // create state variables in matlab, if we export to matlab
 if (exportmatlab)
    {
    run_var = mxCreateDoubleMatrix(totalsamples, 1, mxREAL);
    mxSetName(run_var, "runnr");
    trial_var = mxCreateDoubleMatrix(totalsamples, 1, mxREAL);
    mxSetName(trial_var, "trialnr");
    sample_var = mxCreateDoubleMatrix(totalsamples, 1, mxREAL);
    mxSetName(sample_var, "samplenr");
    for (state=0; state < bci2000data->GetStateListPtr()->GetNumStates(); state++)
     if (cStateListBox->Checked[state])
        {
        state_var[state] = mxCreateDoubleMatrix(totalsamples, 1, mxREAL);
        mxSetName(state_var[state], bci2000data->GetStateListPtr()->GetStatePtr(state)->GetName());
        }
    }

 // go through each run
 Gauge->MinValue=0;
 Gauge->Progress=0;
 Gauge->MaxValue=totalsamples;
 for (cur_run=firstrun; cur_run<=lastrun; cur_run++)
  {
  bci2000data->SetRun(cur_run);
  numsamples=bci2000data->GetNumSamples();
  // go through all samples in each run
  for (sample=0; sample<numsamples; sample++)
   {
   if (cur_totalsample % 100 == 0)
      Gauge->Progress=cur_totalsample;
   // read the state vector
   bci2000data->ReadStateVector(sample);
   cur_trial=IncrementTrial(cur_trial, bci2000data->GetStateVectorPtr());
   if (SaveSampleOrNot(bci2000data->GetStateVectorPtr()))
      {
      if (exportfile)
         {
         fprintf(fp, "%d ", cur_run);
         fprintf(fp, "%d ", cur_trial);
         fprintf(fp, "%d ", sample);
         }
      if (exportmatlab)
         {
         dummy_double=(double)cur_run;
         memcpy((char *)((double *)mxGetPr(run_var)+cur_totalsample), (char *)&dummy_double, sizeof(double));
         dummy_double=(double)cur_trial;
         memcpy((char *)((double *)mxGetPr(trial_var)+cur_totalsample), (char *)&dummy_double, sizeof(double));
         dummy_double=(double)sample;
         memcpy((char *)((double *)mxGetPr(sample_var)+cur_totalsample), (char *)&dummy_double, sizeof(double));
         }
      if (sample % 1000 == 0) Application->ProcessMessages();
      // go through each channel
      for (channel=0; channel<channels; channel++)
       {
       cur_value=bci2000data->ReadValue(channel, sample);
       cur_value_double=(double)cur_value;
       if (exportmatlab) memcpy((char *)((double *)mxGetPr(signal)+channel*totalsamples+cur_totalsample), (char *)&cur_value_double, sizeof(double));
       if (exportfile) fprintf(fp, "%d ", cur_value);
       }
      // store the value of each state
      for (state=0; state < bci2000data->GetStateListPtr()->GetNumStates(); state++)
       {
       // if we decided to save this state
       if (cStateListBox->Checked[state])
          {
          strcpy(cur_statename, bci2000data->GetStateListPtr()->GetStatePtr(state)->GetName());
          cur_state=(int)bci2000data->GetStateVectorPtr()->GetStateValue(cur_statename);
          cur_state_double=(double)cur_state;
          if (exportfile) fprintf(fp, "%d ", cur_state);
          if (exportmatlab) memcpy((char *)((double *)mxGetPr(state_var[state])+cur_totalsample), (char *)&cur_state_double, sizeof(double));
          }
       }
      if (exportfile) fprintf(fp, "\r\n");
      }
   cur_totalsample++;
   } // end samples
  }

 // put the data into matlab
 if (exportmatlab)
    {
    /*
    engPutArray(ep, run_var);
    engPutArray(ep, trial_var);
    engPutArray(ep, sample_var);
    engPutArray(ep, signal);
    for (i=0; i < MAX_STATES; i++)
     if (state_var[i])
        engPutArray(ep, state_var[i]);
    */
    matPutArray(pmat, run_var);
    matPutArray(pmat, trial_var);
    matPutArray(pmat, sample_var);
    matPutArray(pmat, signal);
    for (i=0; i < MAX_STATES; i++)
     if (state_var[i])
        matPutArray(pmat, state_var[i]);
    engOutputBuffer(ep, buffer, 5000);
    // engEvalString(ep, "whos");
    // Application->MessageBox(buffer, "MATLAB - whos", MB_OK);
    mxDestroyArray(signal);
    for (i=0; i < MAX_STATES; i++)
     if (state_var[i])
        mxDestroyArray(state_var[i]);
    matClose(pmat);
    ShutdownMatlabEngine();
    }

 if (fp) fclose(fp);

 frun->Enabled=false;
 lrun->Enabled=false;

 Gauge->Progress=0;
 Application->MessageBox("Export finished successfully", "Message", MB_OK);
}
//---------------------------------------------------------------------------


int TfMain::GetNumSamples()
{
int     sample, num_samples_run, num_samples;
int     cur_trial, cur_run, firstrun, lastrun;

 firstrun= atoi( frun->Text.c_str() );
 lastrun= atoi( lrun->Text.c_str() );
 cur_trial=0;

 // go through each run
 Gauge->MinValue=0;
 Gauge->Progress=0;
 Gauge->MaxValue=lastrun-firstrun+1;
 num_samples=0;
 for (cur_run=firstrun; cur_run<=lastrun; cur_run++)
  {
  Gauge->Progress=cur_run-firstrun;
  bci2000data->SetRun(cur_run);
  num_samples_run=bci2000data->GetNumSamples();
  // go through all samples in each run
  for (sample=0; sample<num_samples_run; sample++)
   {
   // read the state vector
   bci2000data->ReadStateVector(sample);
   cur_trial=IncrementTrial(cur_trial, bci2000data->GetStateVectorPtr());
   if (SaveSampleOrNot(bci2000data->GetStateVectorPtr()))
      num_samples++;
   } // end samples
  }

 Gauge->Progress=0;
 return(num_samples);
}

// uses constraints to determine whether sample should be saved
// returns true if yes, otherwise false
bool TfMain::SaveSampleOrNot(STATEVECTOR *statevector)
{
int     desiredstate, cur_state;
bool    result1a, result1b, result2a, result2b;

 result1a=result1b=false;
 result2a=result2b=false;

 // test first constraint
 // both "horizontal constraints" (if there are both) have to be true
 if (ListBox1a->Text != "")
    {
    result1a=result1b=true;
    desiredstate=atoi(eState1aVal->Text.c_str());
    cur_state=statevector->GetStateValue(ListBox1a->Text.c_str());
    if (desiredstate != cur_state)
       result1a=false;
    if (ListBox1b->Text != "")
       {
       desiredstate=atoi(eState1bVal->Text.c_str());
       cur_state=statevector->GetStateValue(ListBox1b->Text.c_str());
       if (desiredstate != cur_state)
          result1b=false;
       }
    }

 // test second constraint
 // both "horizontal constraints" (if there are both) have to be true
 if (ListBox2a->Text != "")
    {
    result2a=result2b=true;
    desiredstate=atoi(eState2aVal->Text.c_str());
    cur_state=statevector->GetStateValue(ListBox2a->Text.c_str());
    if (desiredstate != cur_state)
       result2a=false;
    if (ListBox2b->Text != "")
       {
       desiredstate=atoi(eState2bVal->Text.c_str());
       cur_state=statevector->GetStateValue(ListBox2b->Text.c_str());
       if (desiredstate != cur_state)
          result2b=false;
       }
    }

 // either first (vertical) constraint or second constraint can be true
 if ((result1a && result1b) || (result2a && result2b) || ((ListBox1a->Text == "" && ListBox2a->Text == "")))
    return(true);
 else
    return(false);
}


// increment trial number if criterion for next trial is met
// returns cur_trial, or cur_trial+1
int TfMain::IncrementTrial(int cur_trial, STATEVECTOR *statevector)
{
static int old_state=-1;
int cur_state, desiredstate, retval;

 cur_state=statevector->GetStateValue(ITIstateListBox->Text.c_str());
 desiredstate=atoi(eITIStateValue->Text.c_str());
 if ((cur_state == desiredstate) && (cur_state != old_state))
    retval=cur_trial+1;
 else
    retval=cur_trial;

 old_state=cur_state;
 return(retval);
}


void __fastcall TfMain::bOpenFileClick(TObject *Sender)
{
 if (OpenDialog->Execute())
    eSourceFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::Button1Click(TObject *Sender)
{
        if (SaveDialog->Execute())
                eDestinationFile->Text=SaveDialog->FileName;
}
//---------------------------------------------------------------------------




void __fastcall TfMain::bDefineInputClick(TObject *Sender)
{
int     ret, state;

 frun->Enabled=true;
 lrun->Enabled=true;

 if (bci2000data) delete bci2000data;
 bci2000data=new BCI2000DATA;
 ret=bci2000data->Initialize(eSourceFile->Text.c_str(), 50000);
 if (ret != BCI2000ERR_NOERR)
    {
    Application->MessageBox("Error opening input file", "Error", MB_OK);
    delete bci2000data;
    return;
    }

 frun->Text= bci2000data->GetFirstRunNumber();
 lrun->Text= bci2000data->GetLastRunNumber();

 UpdateStateListBox(bci2000data->GetFirstRunNumber());

 bConvert->Enabled=true;
}
//---------------------------------------------------------------------------


void __fastcall TfMain::FormClose(TObject *Sender, TCloseAction &Action)
{
 if (bci2000data) delete bci2000data;
 bci2000data=NULL;
}
//---------------------------------------------------------------------------


void TfMain::UpdateStateListBox(int runnr)
{
int state;

 bci2000data->SetRun(runnr);

 cStateListBox->Clear();
 ListBox1a->Clear();
 ListBox1b->Clear();
 ListBox2a->Clear();
 ListBox2b->Clear();
 for (state=0; state < bci2000data->GetStateListPtr()->GetNumStates(); state++)
  {
  cStateListBox->Items->Add(bci2000data->GetStateListPtr()->GetStatePtr(state)->GetName());
  ListBox1a->Items->Add(bci2000data->GetStateListPtr()->GetStatePtr(state)->GetName());
  ListBox1b->Items->Add(bci2000data->GetStateListPtr()->GetStatePtr(state)->GetName());
  ListBox2a->Items->Add(bci2000data->GetStateListPtr()->GetStatePtr(state)->GetName());
  ListBox2b->Items->Add(bci2000data->GetStateListPtr()->GetStatePtr(state)->GetName());
  ITIstateListBox->Items->Add(bci2000data->GetStateListPtr()->GetStatePtr(state)->GetName());
  }

 ITIstateListBox->Text="InterTrialInterval";
 
 for (state=0; state < bci2000data->GetStateListPtr()->GetNumStates(); state++)
  cStateListBox->Checked[state]=true;
}

void __fastcall TfMain::frunChange(TObject *Sender)
{
 UpdateStateListBox(atoi(frun->Text.c_str()));
}
//---------------------------------------------------------------------------

