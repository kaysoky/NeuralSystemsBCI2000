#undef USE_LOGFILE
//---------------------------------------------------------------------------
#pragma hdrstop
#ifdef USE_LOGFILE
# include <stdio.h>
#endif // USE_LOGFILE
#include "UParameter.h"
#include "UGenericVisualization.h"
#include "CalibrationFilter.h"

#ifdef USE_LOGFILE
FILE *calibf;
#endif // USE_LOGFILE

//---------------------------------------------------------------------------

#pragma package(smart_init)

// **************************************************************************
// Function:   CalibrationFilter
// Purpose:    This is the constructor for the CalibrationFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
CalibrationFilter::CalibrationFilter(PARAMLIST *plist, STATELIST *slist)
: offset( NULL ),
  gain( NULL ),
  vis( NULL )
{
 const char* params[] =
 {
  "Filtering floatlist SourceChOffset= 16 "
    "0 0 0 0 "
    "0 0 0 0 "
    "0 0 0 0 "
    "0 0 0 0 "
    "0 -500 500 // offset for channels in A/D units",
  "Filtering floatlist SourceChGain= 16 "
    "0.033 0.033 0.033 0.033 "
    "0.033 0.033 0.033 0.033 "
    "0.033 0.033 0.033 0.033 "
    "0.033 0.033 0.033 0.033 "
    "0.033 -500 500 // gain for each channel (A/D units -> muV)",
  "Filtering int AlignChannels= 0 0 0 1 "
    "// align channels in time (0=no, 1=yes)",
  "Visualize int VisualizeCalibration= 1 0 0 1 "
    "// visualize calibrated channels (0=no, 1=yes)",
 };
 const size_t numParams = sizeof( params ) / sizeof( *params );
 for( size_t i = 0; i < numParams; ++i )
   plist->AddParameter2List( params[ i ] );
}


// **************************************************************************
// Function:   ~CalibrationFilter
// Purpose:    This is the destructor for the CalibrationFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
CalibrationFilter::~CalibrationFilter()
{
  delete[] offset;
  delete[] gain;
  delete vis;

#ifdef USE_LOGFILE
  fprintf(calibf,"Destructing \n");
#endif // USE_LOGFILE
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the CalibrationFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    N/A
// **************************************************************************
void CalibrationFilter::Initialize(PARAMLIST *paramlist, STATEVECTOR *new_statevector, CORECOMM *new_corecomm)
{
  statevector=new_statevector;
  corecomm=new_corecomm;

  int alignyesno = 0,
      visualizeyesno = 0,
      numchoffset = 0,
      numchgain = 0;

  try // in case one of the parameters is not defined (should always be, since we requested them)
  {
    alignyesno=atoi(paramlist->GetParamPtr("AlignChannels")->GetValue());
    visualizeyesno=atoi(paramlist->GetParamPtr("VisualizeCalibration")->GetValue());
    numchoffset=paramlist->GetParamPtr("SourceChOffset")->GetNumValues();
    numchgain=paramlist->GetParamPtr("SourceChGain")->GetNumValues();

    recordedChans= atoi( paramlist->GetParamPtr("SoftwareCh")->GetValue() );
    transmittedChans= atoi( paramlist->GetParamPtr("TransmitCh")->GetValue() );

    // paramlist.GetParamPtr("TransmitChList")->GetNumValues()
    // paramlist.GetParamPtr("TransmitCh")->GetValue()
  }
  catch(...)
  { return; }

  // if the number of channels does not match for offset and gain, exit with an error
  if (numchoffset != numchgain) return;

  // allocate arrays for offsets and gains
  // we don't always want to query parameters
  delete [] offset;
  delete [] gain;

  offset=new float[numchoffset];
  gain=new float[numchgain];

  // copy the parameters in our private arrays
  for (int i=0; i<numchoffset; i++)
  {
    offset[i]=atoi(paramlist->GetParamPtr("SourceChOffset")->GetValue(i));
    gain[i]=atof(paramlist->GetParamPtr("SourceChGain")->GetValue(i));
  }

  // do we want to align the samples in time ?
  if (alignyesno == 0)
    align=false;
  else
  {
    align=true;
    delta= (float)recordedChans;
    delta= 1/delta;

    for(int i=0;i<transmittedChans;i++)   // get original channel position
    {
      int origchan= atoi(paramlist->GetParamPtr("TransmitChList")->GetValue(i));
      w2[i]= delta * (float)origchan;
      w1[i]= 1.0 - w2[i];
      old[i]= 0;
#ifdef USE_LOGFILE
      fprintf(calibf,"Chan= %3d OrigChan = %3d \n",i,origchan);
#endif // USE_LOGFILE
    }
  }

  // do we want to visualize ?
  if (visualizeyesno == 0)
    visualize=false;
  else
  {
    visualize=true;
    // create an instance of GenericVisualization
    // it will handle the visualization to the operator
    delete vis;
    vis=new GenericVisualization(paramlist, corecomm);
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_WINDOWTITLE, "Calibration");
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_MINVALUE, "-40");
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_MAXVALUE, "40");
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_NUMSAMPLES, "256");
  }

  return;
}


// **************************************************************************
// Function:   Process
// Purpose:    This function applies the calibration routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    N/A
// **************************************************************************
void CalibrationFilter::Process(const GenericSignal *input, GenericSignal *output)
{
  // actually perform the calibration on the input and write it into the output signal
  for(size_t channel=0; channel<input->Channels(); channel++)
    for(size_t sample=0; sample<input->MaxElements(); sample++)
    {
      float value=(input->GetValue(channel, sample)-offset[channel])*gain[channel];

#ifdef USE_LOGFILE
      fprintf(calibf,"value = %7.2f ",value);
#endif // USE_LOGFILE

      if( align == true )
      {
        float temp= value;
        value= ( w1[channel]*temp ) + (w2[channel]*old[channel] );
        old[channel]= temp;
#ifdef USE_LOGFILE
        fprintf(calibf,"value2 = %7.2f w1[%3d]= %7.3f ",value,channel,w1[channel]);
#endif // USE_LOGFILE
      }

#ifdef USE_LOGFILE
      fprintf(calibf,"\n");
#endif // USE_LOGFILE

      output->SetValue(channel, sample, value);
    }

  // visualize the processed channels, if wanted
  if (visualize)
  {
    vis->SetSourceID(SOURCEID_CALIBRATION);
    vis->Send2Operator(output);
  }

  return;
}



