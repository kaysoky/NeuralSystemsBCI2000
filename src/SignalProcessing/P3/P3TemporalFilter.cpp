//---------------------------------------------------------------------------

#include "PCHIncludes.h"
#pragma hdrstop

#include <stdio.h>
#include "..\shared\defines.h"
#include "UState.h"

#include "P3TemporalFilter.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

using namespace std;

RegisterFilter( P3TemporalFilter, 2.D );

// **************************************************************************
// Function:   P3TemporalFilter
// Purpose:    This is the constructor for the P3TemporalFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
P3TemporalFilter::P3TemporalFilter()
: vis( NULL ),
  vissignal( NULL ),
  mNumberOfSequences( 0 )
{


 BEGIN_PARAMETER_DEFINITIONS
  "Visualize int VisualizeP3TemporalFiltering= 1 0 0 1 "
    "// visualize Temporal filtered signals (0=no 1=yes) (boolean)",
  "P3SignalProcessing int NumSamplesInERP= 144 144 0 1000 "
    "// Number of samples stored for each response",
  "P3SignalProcessing int NumERPsToAverage= 15 15 0 1000 "
    "// Number of ERPs to average before doing DF",
  "P3SignalProcessing int TargetERPChannel= 1 1 0 128 "
    "// Target Channel for ERP Display in order of SigProc transfer",
  "Visualize float ERPMinDispVal= 0 0 -16383 16384 "
    "// Minimum value for ERP display",
  "Visualize float ERPMaxDispVal= 300 300 -16383 16384 "
    "// Maximum value for ERP display",
 END_PARAMETER_DEFINITIONS

 BEGIN_STATE_DEFINITIONS
 // the code of the stimulus (e.g., which row/column in the P3 spelling paradigm)
   "StimulusCodeRes 7 0 0 0",
 // the type of the stimulus (e.g., standard/oddball in the oddball or P3 spelling paradigm)
   "StimulusTypeRes 3 0 0 0",
 END_STATE_DEFINITIONS

 // initialize ERP buffer variables
 for (size_t cur_buf=0; cur_buf<MAX_ERPBUFFERS; cur_buf++)
  {
  ERPBufCode[cur_buf]=ERPBUFCODE_EMPTY;
  ERPBufType[cur_buf]=0;
  ERPBufSampleCount[cur_buf]=0;
  ERPBufSamples[cur_buf]=NULL;
  }
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
 delete vis;
 delete vissignal;

}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void P3TemporalFilter::Preflight( const SignalProperties& inSignalProperties,
                                        SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.

  // Resource availability checks.
  /* The P3 temporal filter seems not to depend on external resources. */

  // Required states.
  State( "Running" );
  State( "StimulusCode" );
  State( "StimulusType" );

  // Input signal checks.
  PreflightCondition( inSignalProperties >= SignalProperties(
    Parameter( "SpatialFilteredChannels" ), Parameter( "SampleBlockSize" ), SignalType::int16 ) );

  int NumERPsToAverage = Parameter( "NumERPsToAverage" ),
      NumberOfSequences = OptionalParameter( NumERPsToAverage, "NumberOfSequences" );
  PreflightCondition( NumERPsToAverage <= NumberOfSequences );

  // Requested output signal properties.
  outSignalProperties = SignalProperties(
                           Parameter( "SpatialFilteredChannels" ),
                           Parameter( "NumSamplesInERP" ) );
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
void P3TemporalFilter::Initialize()
{
int     i,j;
char    cur_buf[256];

  visualize= ( int )Parameter("VisualizeP3TemporalFiltering");
  targetERPchannel= Parameter("TargetERPChannel");
  numsamplesinERP= Parameter("NumSamplesInERP");
  numERPsnecessary= Parameter("NumERPsToAverage");
  mNumberOfSequences = OptionalParameter( numERPsnecessary, "NumberOfSequences" );
  numchannels= Parameter("SpatialFilteredChannels");
  mindispval= Parameter("ERPMinDispVal");
  maxdispval= Parameter("ERPMaxDispVal");

 // we can't average 0 ERPs or have 0 samples in an ERP
 if ((numERPsnecessary == 0) || (numsamplesinERP == 0))
    return;


 if ( visualize )
    {
    delete vis;
    vis= new GenericVisualization;
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_WINDOWTITLE, "ERP");
    sprintf(cur_buf, "%d", numsamplesinERP);
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_NUMSAMPLES, cur_buf);
    sprintf(cur_buf, "%f", mindispval);
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_MINVALUE, cur_buf);
    sprintf(cur_buf, "%f", maxdispval);
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_MAXVALUE, cur_buf);
    }

 OldStimulusCode=0;
 OldRunning=0;

 DeleteAllERPBuffers();
 mERPDone.clear();

 // create a signal that will display the ERPs (on a particular target channel)
 // for each of the twelve StimulusCodes
 delete vissignal;
 vissignal=new GenericSignal(12, numsamplesinERP);
 for (i=0; i<12; i++)
  for (j=0; j<numsamplesinERP; j++)
   vissignal->SetValue(i, j, 0);

 if ( visualize )
    {
    vis->SetSourceID(SOURCEID_TEMPORALFILT);
    vis->Send2Operator(vissignal);
    }
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

 maxstimuluscode=0;

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
 CurrentRunning=State("Running");
 CurrentStimulusCode=State("StimulusCode");
 CurrentStimulusType=State("StimulusType");
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

 if (StimulusCode > maxstimuluscode)
    maxstimuluscode=StimulusCode;

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
//             until the number of samples in a given buffer reached numsamplesinERP
//             subsequent procedures need to ensure that full buffers are processed and deleted
// Parameters: input - input signal from the spatial filter to be appended to our buffers
// Returns:    N/A
// **************************************************************************
void P3TemporalFilter::AppendToERPBuffers(const GenericSignal *input)
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
     for (samples=0; samples<(int)input->Elements(); samples++)
     {
      if (ERPBufSampleCount[cur_buf] < numsamplesinERP)
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
int     stimuluscode, stimulustype, count;
float   cur_output;

 // check, for each stimulus code, whether we have enough completed waveforms
 // in the matrix, we have 12 different stimuli, but we know the maximum stimulus code that we observed so far
 for (stimuluscode=1; stimuluscode<=maxstimuluscode; stimuluscode++)
  {
  count=0;
  // count the number of completed ERP waveforms for this StimulusCode
  for (cur_buf=0; cur_buf<MAX_ERPBUFFERS; cur_buf++)
   if ((ERPBufCode[cur_buf] == stimuluscode) && (ERPBufSampleCount[cur_buf] == numsamplesinERP))
      count++;
  // if we have enough waveforms of this type (i.e., StimulusCode)
  // calculate the average signal and assign it to the output signal
  if (count >= numERPsnecessary && !mERPDone[ stimuluscode ] )
     {
     // set the contents of the output signal to 0
     for (ch=0; ch<numchannels; ch++)
      for (samples=0; samples<numsamplesinERP; samples++)
       output->SetValue(ch, samples, 0);
     // now, accumulate the ERP waveforms in the output signal
     // this should be done count times
     for (cur_buf=0; cur_buf<MAX_ERPBUFFERS; cur_buf++)
      {
      if ((ERPBufCode[cur_buf] == stimuluscode) && (ERPBufSampleCount[cur_buf] == numsamplesinERP))
         {
         stimulustype=ERPBufType[cur_buf];
         for (ch=0; ch<numchannels; ch++)
          for (samples=0; samples<numsamplesinERP; samples++)
           {
           cur_output=output->GetValue(ch, samples);
           output->SetValue(ch, samples, cur_output+ERPBufSamples[cur_buf]->GetValue(ch, samples));
           }
         }
      }
     // this writes the averaged waveforms to files
     // there is a program p3test.m that compares these files to offline results (results should obviously be identical)
     /* char filename[256];
     sprintf(filename, "c:\\temp\\test%d.dat", stimuluscode);
     FILE *fp=fopen(filename, "ab"); */

     // now, calculate the AVERAGE output waveform and stick it into the output signal
     for (ch=0; ch<numchannels; ch++)
      {
      for (samples=0; samples<numsamplesinERP; samples++)
       {
       cur_output=output->GetValue(ch, samples)/(float)numERPsnecessary;
       output->SetValue(ch, samples, cur_output);
       // set the visualization output for this StimulusType to the TargetChannel's average waveform
       if (ch == targetERPchannel-1)
          {
          vissignal->SetValue(stimuluscode-1, samples, cur_output);
          // fprintf(fp, "%.2f ", cur_output);
          }
       }
      }
     // also, update the ERP display at the operator
     if ( visualize ) vis->Send2Operator(vissignal);
     // fprintf(fp, "\r\n");
     // fclose(fp);
     // at the same time, communicate the code and type of this ERP waveform
     State("StimulusCodeRes")=(unsigned short)stimuluscode;
     State("StimulusTypeRes")=(unsigned short)stimulustype;
     mERPDone[ stimuluscode ] = true;
    }
    if( count >= mNumberOfSequences )
     {
     // finally, we have to delete the ERP waveform buffers
     for (cur_buf=0; cur_buf<MAX_ERPBUFFERS; cur_buf++)
      if ((ERPBufCode[cur_buf] == stimuluscode) && (ERPBufSampleCount[cur_buf] == numsamplesinERP))
         DeleteERPBuffer(cur_buf);
     mERPDone[ stimuluscode ] = false;
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
void P3TemporalFilter::Process(const GenericSignal *input, GenericSignal *output)
{
int     i, ret;
bool    cur_buf;

 GetStates();

 // don't do anything if we are not running
 if (CurrentRunning == 0) return;

 // check whether we are starting to get a response for a certain StimulusCode
 if ((CurrentStimulusCode > 0) && (OldStimulusCode == 0))
    {
    cur_buf=ApplyForNewERPBuffer(CurrentStimulusCode, CurrentStimulusType, numchannels, numsamplesinERP);
    if (!cur_buf)
       {
       bcierr << 415 << " P3 Temporal Filter: Inconsistency or ran out of buffers" << endl;
       return;
       }
    }

 // append the signal to the ERP buffers, as appropriate
 AppendToERPBuffers(input);

 // process the ERP buffers
 // send out an ERP buffer, in case it has been filled
 // in this case, also set the right states; before, clear the states
 State("StimulusCodeRes")=0;
 State("StimulusTypeRes")=0;
 if( ProcessERPBuffers(output) != noError )
   bcierr << "ERPBuffer processing error" << endl;

 OldStimulusCode=CurrentStimulusCode;
 OldRunning=CurrentRunning;
}



