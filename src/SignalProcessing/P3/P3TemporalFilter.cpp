//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include "..\shared\defines.h"

#include "P3TemporalFilter.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

// **************************************************************************
// Function:   P3TemporalFilter
// Purpose:    This is the constructor for the P3TemporalFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
P3TemporalFilter::P3TemporalFilter(PARAMLIST *plist, STATELIST *slist)
{
char    line[512];
int     cur_buf;

 strcpy(line, "Visualize int VisualizeP3TemporalFiltering= 1 0 0 1  // visualize Temporal filtered signals (0=no 1=yes)");
 plist->AddParameter2List( line, strlen(line) );
 strcpy(line, "P3SignalProcessing int NumSamplesInERP= 144 144 0 1000  // Number of samples stored for each response");
 plist->AddParameter2List( line, strlen(line) );
 strcpy(line, "P3SignalProcessing int NumERPsToAverage= 15 15 0 1000  // Number of ERPs to average before doing DF");
 plist->AddParameter2List( line, strlen(line) );

 strcpy(line, "Visualize float ERPMinDispVal= 0 0 -16383 16384  // Minimum value for ERP display");
 plist->AddParameter2List( line, strlen(line) );
 strcpy(line, "Visualize float ERPMaxDispVal= 300 300 -16383 16384  // Maximum value for ERP display");
 plist->AddParameter2List( line, strlen(line) );

 // the code of the stimulus (e.g., which row/column in the P3 spelling paradigm)
 slist->AddState2List("StimulusCodeRes 5 0 0 0 \n");
 // the type of the stimulus (e.g., standard/oddball in the oddball or P3 spelling paradigm)
 slist->AddState2List("StimulusTypeRes 3 0 0 0 \n");

 // initialize ERP buffer variables
 for (cur_buf=0; cur_buf<MAX_ERPBUFFERS; cur_buf++)
  {
  ERPBufCode[cur_buf]=ERPBUFCODE_EMPTY;
  ERPBufType[cur_buf]=0;
  ERPBufSampleCount[cur_buf]=0;
  ERPBufSamples[cur_buf]=NULL;
  }

 vis=NULL;
}


// **************************************************************************
// Function:   ~P3TemporalFilter
// Purpose:    This is the destructor for the P3TemporalFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
P3TemporalFilter::~P3TemporalFilter()
{
 DeleteAllERPBuffers();

 if (vis) delete vis;
 vis=NULL;
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the P3TemporalFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int P3TemporalFilter::Initialize(PARAMLIST *paramlist, STATEVECTOR *new_statevector, CORECOMM *new_corecomm)
{
int     i,j;
int     visualizeyn;
char    cur_buf[256];

 statevector=new_statevector;
 corecomm=new_corecomm;

 try // in case one of the parameters is not defined (should always be, since we requested them)
  {
  visualizeyn= atoi(paramlist->GetParamPtr("VisualizeP3TemporalFiltering")->GetValue() );
  numsamplesinerp= atoi(paramlist->GetParamPtr("NumSamplesInERP")->GetValue() );
  numerpsnecessary= atoi(paramlist->GetParamPtr("NumERPsToAverage")->GetValue() );
  numchannels= atoi(paramlist->GetParamPtr("SpatialFilteredChannels")->GetValue() );
  mindispval= atof(paramlist->GetParamPtr("ERPMinDispVal")->GetValue() );
  maxdispval= atof(paramlist->GetParamPtr("ERPMaxDispVal")->GetValue() );
  }
 catch(...)
  { return(0); }


 if ( visualizeyn == 1 )
    {
    visualize=true;
    if (vis) delete vis;
    vis= new GenericVisualization( paramlist, corecomm );
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_WINDOWTITLE, "ERP");
    sprintf(cur_buf, "%d", numsamplesinerp);
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_NUMSAMPLES, cur_buf);
    sprintf(cur_buf, "%f", mindispval);
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_MINVALUE, cur_buf);
    sprintf(cur_buf, "%f", maxdispval);
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_MAXVALUE, cur_buf);
    }
 else
    visualize=false;

 OldStimulusCode=0;
 OldRunning=0;

 DeleteAllERPBuffers();

 return(1);
}


// **************************************************************************
// Function:   DeleteAllERPBuffers
// Purpose:    This function deletes all ERP buffers
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void P3TemporalFilter::DeleteAllERPBuffers()
{
int     cur_buf;

 // delete all erp buffer variables
 for (cur_buf=0; cur_buf<MAX_ERPBUFFERS; cur_buf++)
  DeleteERPBuffer(cur_buf);
}


// **************************************************************************
// Function:   DeleteERPBuffer
// Purpose:    This function deletes one particular ERP buffer
// Parameters: cur_buf - buffer number
// Returns:    N/A
// **************************************************************************
void P3TemporalFilter::DeleteERPBuffer(int cur_buf)
{
 // delete all erp buffer variables
 ERPBufCode[cur_buf]=ERPBUFCODE_EMPTY;
 ERPBufType[cur_buf]=0;
 ERPBufSampleCount[cur_buf]=0;
 if (ERPBufSamples[cur_buf])
    {
    delete ERPBufSamples[cur_buf];
    ERPBufSamples[cur_buf]=NULL;
    }
}


// **************************************************************************
// Function:   GetStates
// Purpose:    This function reads certain state variables into local variables
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void P3TemporalFilter::GetStates()
{
 CurrentRunning=statevector->GetStateValue("Running");
 CurrentStimulusCode=statevector->GetStateValue("StimulusCode");
 CurrentStimulusType=statevector->GetStateValue("StimulusType");
}


// **************************************************************************
// Function:   ApplyForNewERPBuffer
// Purpose:    This functions creates a new buffer to hold an ERP waveform
//             or it looks for an existing one with the same StimulusCode
// Parameters: StimulusCode - code of stimulus (e.g., flashing row/column, etc.)
//             StimulusType - type of stimulus (standard/oddball)
// Returns:    true  - no error
//             false - error (buffer table full)
// **************************************************************************
bool P3TemporalFilter::ApplyForNewERPBuffer(int StimulusCode, int StimulusType, int numchannels, int numsamples)
{
int     cur_buf, ch, samples;
bool    ret;

 ret=false;

 // look for an empty spot and allocate it
 for (cur_buf=0; cur_buf<MAX_ERPBUFFERS; cur_buf++)
  {
  // is this spot empty ? if yes, allocate it
  if (ERPBufCode[cur_buf] == ERPBUFCODE_EMPTY)
     {
     ERPBufCode[cur_buf]=StimulusCode;
     ERPBufType[cur_buf]=StimulusType;
     ERPBufSamples[cur_buf]=new GenericSignal(numchannels, numsamples);
     ERPBufSampleCount[cur_buf]=0;
     ret=true;
     break;
     }
  }

 return(ret);
}


// **************************************************************************
// Function:   AppendToERPBuffers
// Purpose:    This functions appends the current input signal to all ERP signal buffers
//             until the number of samples in a given buffer reached numsamplesinerp
//             subsequent procedures need to ensure that full buffers are processed and deleted
// Parameters: input - input signal from the spatial filter to be appended to our buffers
// Returns:    N/A
// **************************************************************************
void P3TemporalFilter::AppendToERPBuffers(GenericSignal *input)
{
int     cur_buf, samples, ch;
float   oldvalue;

 // go through all buffers that contain ERP signals
 for (cur_buf=0; cur_buf<MAX_ERPBUFFERS; cur_buf++)
  {
  // does this buffer slot contain data and is it active?
  // if yes, append the input signal
  if (ERPBufCode[cur_buf] != ERPBUFCODE_EMPTY)
     {
     // go through all samples and append them if we have not stored enough
     for (samples=0; samples<(int)input->MaxElements(); samples++)
      {
      if (ERPBufSampleCount[cur_buf] < numsamplesinerp)
         {
         for (ch=0; ch<(int)input->Channels(); ch++)
          {
          oldvalue=ERPBufSamples[cur_buf]->GetValue(ch, ERPBufSampleCount[cur_buf]);
          ERPBufSamples[cur_buf]->SetValue(ch, ERPBufSampleCount[cur_buf], oldvalue+input->GetValue(ch, samples));
          }
         ERPBufSampleCount[cur_buf]++;
         }
      }
     }
  }
}


// **************************************************************************
// Function:   ProcessERPBuffers
// Purpose:    This functions processes all ERP buffers that are full
//             in this case, the output signal equals the content of the ERP buffer
//             and the states StimulusCodeRes and StimulusTypeRes are set appropriately
// Parameters: output - output signal
// Returns:    1 - no error
//             0 - error
// **************************************************************************
int P3TemporalFilter::ProcessERPBuffers(GenericSignal *output)
{
int     cur_buf, samples, ch;

 // go through all buffers that contain ERP signals
 for (cur_buf=0; cur_buf<MAX_ERPBUFFERS; cur_buf++)
  {
  // does this buffer slot contain data, is the buffer filled with a complete waveform, and do we have enough waveforms accumulated ?
  // if yes, process it
  // if ((ERPBufCode[cur_buf] != ERPBUFCODE_EMPTY) && (ERPBufSampleCount[cur_buf] == numsamplesinerp) && (ERPBufAvgCount[cur_buf] == numerpsnecessary))
  if ((ERPBufCode[cur_buf] != ERPBUFCODE_EMPTY) && (ERPBufSampleCount[cur_buf] == numsamplesinerp))
     {
     // set the output signal to the content of the ERP waveform buffer
     for (ch=0; ch<numchannels; ch++)
      for (samples=0; samples<numsamplesinerp; samples++)
       output->SetValue(ch, samples, ERPBufSamples[cur_buf]->GetValue(ch, samples));
     // at the same time, communicate the code and type of this ERP waveform
     statevector->SetStateValue("StimulusCodeRes", (unsigned short)ERPBufCode[cur_buf]);
     statevector->SetStateValue("StimulusTypeRes", (unsigned short)ERPBufType[cur_buf]);
     // after we have processed a buffer, we need to clear it
     // only one buffer for each call (-> break;)
     DeleteERPBuffer(cur_buf);
     break;
     }
  }

 return(1);
}


// **************************************************************************
// Function:   Process
// Purpose:    This function applies the P3 temporal filtering routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int P3TemporalFilter::Process(GenericSignal *input, GenericSignal *output)
{
int     i, ret;
bool    cur_buf;

 GetStates();

 // don't do anything if we are not running
 if (CurrentRunning == 0) return(1);

 // check whether we are starting to get a response for a certain StimulusCode
 if ((CurrentStimulusCode > 0) && (OldStimulusCode == 0))
    {
    cur_buf=ApplyForNewERPBuffer(CurrentStimulusCode, CurrentStimulusType, numchannels, numsamplesinerp);
    if (!cur_buf)
       {
       error.SetErrorMsg("P3 Temporal Filter: Inconsistency or ran out of buffers");
       error.SetErrorCode(415);
       return(0);
       }
    }

 // append the signal to the ERP buffers, as appropriate
 AppendToERPBuffers(input);

 // process the ERP buffers
 // send out an ERP buffer, in case it has been filled
 // in this case, also set the right states; before, clear the states
 statevector->SetStateValue("StimulusCodeRes", 0);
 statevector->SetStateValue("StimulusTypeRes", 0);
 ret=ProcessERPBuffers(output);

 if ( visualize )
    {
    vis->SetSourceID(SOURCEID_TEMPORALFILT);
    vis->Send2Operator(output);
    }

 OldStimulusCode=CurrentStimulusCode;
 OldRunning=CurrentRunning;

 return(1);
}



