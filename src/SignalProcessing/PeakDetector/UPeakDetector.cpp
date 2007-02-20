/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include "PCHIncludes.h"
#pragma hdrstop

#include <math.h>
#include <stdio.h>

#include "UPeakDetector.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

RegisterFilter( PeakDetector, 2.C );

// **************************************************************************
// Function:   PeakDetector
// Purpose:    This is the constructor for the PeakDetector class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
PeakDetector::PeakDetector()
: vis( NULL )
{
 BEGIN_PARAMETER_DEFINITIONS
  "PeakDetector float PosPeakThreshold= 0 0 0 2 "
    "// Threshold for positive peaks",
  "PeakDetector float NegPeakThreshold= 0 0 0 2 "
    "// Threshold for negative peaks",
  "PeakDetector int HistoryLength= 10 0 0 10 "
    "// Length of history of peak counts",
  "PeakDetector int TargetChannelPos= 0 0 0 10 "
    "// Target channel for peak detection of positive peaks",
  "PeakDetector int TargetChannelNeg= 0 0 0 10 "
    "// Target channel for peak detection of negative peaks",
  "Visualize int VisualizePeakDetector= 1 0 0 1 "
    "// visualize peak detection results (0=no 1=yes) (boolean)",
 END_PARAMETER_DEFINITIONS
}


// **************************************************************************
// Function:   ~PeakDetector
// Purpose:    This is the destructor for the PeakDetector class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
PeakDetector::~PeakDetector()
{
  delete vis;
}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void PeakDetector::Preflight( const SignalProperties& inSignalProperties,
                                    SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.

  // Resource availability checks.
  /* The P3 temporal filter seems not to depend on external resources. */

  // Input signal checks.
  PreflightCondition( inSignalProperties >= SignalProperties(
    Parameter( "SpatialFilteredChannels" ), Parameter( "SampleBlockSize" ), SignalType::int16 ) );

  // Requested output signal properties.
  outSignalProperties = SignalProperties( 2, Parameter( "HistoryLength" ) );
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
void PeakDetector::Initialize()
{
int     i,j;
char    cur_buf[256];
int     nBuf;

  samples=     Parameter( "SampleBlockSize" );
  visualize=   ( int )Parameter("VisualizePeakDetector");
  hz=          Parameter("SamplingRate");
  posthresh=   Parameter("PosPeakThreshold");
  negthresh=   Parameter("NegPeakThreshold");
  nBins=       Parameter("HistoryLength");
  targetchpos= Parameter("TargetChannelPos");
  targetchneg= Parameter("TargetChannelNeg");

 if ( visualize )
    {
    delete vis;
    vis= new GenericVisualization;
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
size_t   ch, sample;
int   num_pos_peaks, num_neg_peaks;
static bool first=true;

 // the first time, set output signal to 0
 if (first)
    {
    first=false;
    for (ch=0; ch < 2; ch++)
     for (sample=0; sample < output->Elements(); sample++)
      output->SetValue(ch, sample, 0);
    }

 // shift the number of peaks to the left to make room for the new number of peaks
 for (ch=0; ch < 2; ch++)
  for (sample=1; sample < output->Elements(); sample++)
   output->SetValue(ch, sample-1, output->GetValue(ch, sample));

 // calculate the new number of peaks and assign them to the right-most (i.e., most recent) position
 num_pos_peaks=get_num_pos_peaks(input, targetchpos);
 num_neg_peaks=get_num_neg_peaks(input, targetchneg);
 // channel 0 ... number of positive peaks
 output->SetValue(0, output->Elements()-1, num_pos_peaks);
 // channel 1 ... number of negative peaks
 output->SetValue(1, output->Elements()-1, num_neg_peaks);

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
size_t  cur_idx, peak_ptr;
float   current_val, next_val;
bool    peak_flag;
int     num_peaks;

 peak_ptr=-1;
 peak_flag=false;
 num_peaks=0;

 for (cur_idx=0; cur_idx<input->Elements()-1; cur_idx++)
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
size_t  cur_idx, peak_ptr;
float   current_val, next_val;
bool    peak_flag;
int     num_peaks;

 peak_ptr=-1;
 peak_flag=false;
 num_peaks=0;

 for (cur_idx=0; cur_idx<input->Elements()-1; cur_idx++)
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




