//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <math.h>
#include <stdio.h>

#include "UPeakDetector.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


// **************************************************************************
// Function:   PeakDetector
// Purpose:    This is the constructor for the PeakDetector class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
PeakDetector::PeakDetector(PARAMLIST *plist, STATELIST *slist)
{
char line[512];

 vis= NULL;

 strcpy(line,"PeakDetector float PosPeakThreshold= 0 0 0 2  // Threshold for positive peaks");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"PeakDetector float NegPeakThreshold= 0 0 0 2  // Threshold for negative peaks");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"PeakDetector int HistoryLength= 10 0 0 10      // Length of history of peak counts");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"PeakDetector int TargetChannelPos= 0 0 0 10      // Target channel for peak detection of positive peaks");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"PeakDetector int TargetChannelNeg= 0 0 0 10      // Target channel for peak detection of negative peaks");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line, "Visualize int VisualizePeakDetector= 1 0 0 1  // visualize peak detection results (0=no 1=yes)");
 plist->AddParameter2List( line, strlen(line) );
}


// **************************************************************************
// Function:   ~PeakDetector
// Purpose:    This is the destructor for the PeakDetector class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
PeakDetector::~PeakDetector()
{
  if( vis ) delete vis;
  vis= NULL;
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the PeakDetector
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void PeakDetector::Initialize(PARAMLIST *paramlist, STATEVECTOR *new_statevector, CORECOMM *new_corecomm)
{
int     i,j;
int     visualizeyn;
char    cur_buf[256];
int     nBuf;

        statevector=new_statevector;
        corecomm=new_corecomm;

 try // in case one of the parameters is not defined (should always be, since we requested them)
  {
        samples=     atoi(paramlist->GetParamPtr( "SampleBlockSize" )->GetValue());
        visualizeyn= atoi(paramlist->GetParamPtr("VisualizePeakDetector")->GetValue() );
        hz=          atoi(paramlist->GetParamPtr("SamplingRate")->GetValue());
        posthresh=   atof(paramlist->GetParamPtr("PosPeakThreshold")->GetValue());
        negthresh=   atof(paramlist->GetParamPtr("NegPeakThreshold")->GetValue());
        nBins=       atoi(paramlist->GetParamPtr("HistoryLength")->GetValue());;
        targetchpos=    atoi(paramlist->GetParamPtr("TargetChannelPos")->GetValue());;
        targetchneg=    atoi(paramlist->GetParamPtr("TargetChannelNeg")->GetValue());;
  }
 catch(...)
  { return; }

 if ( visualizeyn == 1 )
    {
    visualize=true;
    if (vis) delete vis;
    vis= new GenericVisualization( paramlist, corecomm);
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_WINDOWTITLE, "Peak Detector");
    sprintf(cur_buf, "%d", nBins);
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_NUMSAMPLES, cur_buf);
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_MINVALUE, "0");
    vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_MAXVALUE, "100");
    for (i=0; i<nBins; i++)
     {
     sprintf(cur_buf, "%03d %.0f", i, (float)(nBins-i));
     vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_XAXISLABEL, cur_buf);
     }
    }
 else
 {
        visualize=false;
 }

 return;
}


// **************************************************************************
// Function:   Process
// Purpose:    This function applies the peak detector
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void PeakDetector::Process(const GenericSignal *input, GenericSignal *output)
{
int   ch, sample;
int   num_pos_peaks, num_neg_peaks;
static bool first=true;

 // the first time, set output signal to 0
 if (first)
    {
    first=false;
    for (ch=0; ch < 2; ch++)
     for (sample=0; sample < output->MaxElements(); sample++)
      output->SetValue(ch, sample, 0);
    }

 // shift the number of peaks to the left to make room for the new number of peaks
 for (ch=0; ch < 2; ch++)
  for (sample=1; sample < output->MaxElements(); sample++)
   output->SetValue(ch, sample-1, output->GetValue(ch, sample));

 // calculate the new number of peaks and assign them to the right-most (i.e., most recent) position
 num_pos_peaks=get_num_pos_peaks(input, targetchpos);
 num_neg_peaks=get_num_neg_peaks(input, targetchneg);
 // channel 0 ... number of positive peaks
 output->SetValue(0, output->MaxElements()-1, num_pos_peaks);
 // channel 1 ... number of negative peaks
 output->SetValue(1, output->MaxElements()-1, num_neg_peaks);

 // FILE *fp=fopen("c:\\data.txt", "ab");
 // fprintf(fp, "%d\r\n", num_pos_peaks);
 // fclose(fp);

  if( visualize )
    {
    vis->SetSourceID(SOURCEID_TEMPORALFILT);
    vis->Send2Operator(output);
    }

 return;
}


// detect number of positive peaks
int PeakDetector::get_num_pos_peaks(const GenericSignal *input, int channel)
{
int     cur_idx, peak_ptr;
float   current_val, next_val;
bool    peak_flag;
int     num_peaks;

 peak_ptr=-1;
 peak_flag=false;
 num_peaks=0;

 for (cur_idx=0; cur_idx<input->MaxElements()-1; cur_idx++)
  {
  current_val=fabs((float)input->GetValue(channel, cur_idx));
  next_val=fabs((float)input->GetValue(channel, cur_idx+1));
  if (current_val < posthresh) peak_flag=false;
  if (current_val < next_val)               // detect a positive slope
     {
     peak_flag=true;
     peak_ptr=cur_idx;
     }
  else
     if ((current_val > next_val) && (peak_flag) && ((float)input->GetValue(channel, cur_idx) > posthresh))            // detect a negative slope
        {
        num_peaks++;
        peak_flag=false;
        }
//        return((cur_idx+peak_ptr+1)/2);
  }

 return(num_peaks);
}


// detect number of negative peaks
int PeakDetector::get_num_neg_peaks(const GenericSignal *input, int channel)
{
int     cur_idx, peak_ptr;
float   current_val, next_val;
bool    peak_flag;
int     num_peaks;

 peak_ptr=-1;
 peak_flag=false;
 num_peaks=0;

 for (cur_idx=0; cur_idx<input->MaxElements()-1; cur_idx++)
  {
  current_val=fabs((float)input->GetValue(channel, cur_idx));
  next_val=fabs((float)input->GetValue(channel, cur_idx+1));
  if (current_val < negthresh) peak_flag=false;
  if (current_val < next_val)               // detect a positive slope
     {
     peak_flag=true;
     peak_ptr=cur_idx;
     }
  else
     if ((current_val > next_val) && (peak_flag) && ((float)input->GetValue(channel, cur_idx) < negthresh))            // detect a negative slope
        {
        num_peaks++;
        peak_flag=false;
        }
//        return((cur_idx+peak_ptr+1)/2);
  }

 return(num_peaks);
}


