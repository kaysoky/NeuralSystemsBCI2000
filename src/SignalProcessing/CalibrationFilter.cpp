//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "CalibrationFilter.h"
#include <stdio.h>

// FILE *calibf;

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
{

char line[512];

// calibf= fopen("CalibFile.asc","w+");
// fprintf(calibf,"Constructing \n");

 offset=gain=NULL;
// instance=my_instance;

 strcpy(line, "Filtering floatlist SourceChOffset= 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -500 500  // offset for channels in A/D units");
 plist->AddParameter2List(line, strlen(line));
 strcpy(line, "Filtering floatlist SourceChGain= 16 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 -500 500  // gain for each channel (A/D units -> muV)");
 plist->AddParameter2List(line, strlen(line));
 strcpy(line, "Filtering int AlignChannels= 0 0 0 1  // align channels in time (0=no, 1=yes) (does NOT work yet?)");
 plist->AddParameter2List(line, strlen(line));
 strcpy(line, "Visualize int VisualizeCalibration= 1 0 0 1  // visualize calibrated channels (0=no, 1=yes)");
 plist->AddParameter2List(line, strlen(line));

 vis=NULL;
}


// **************************************************************************
// Function:   ~CalibrationFilter
// Purpose:    This is the destructor for the CalibrationFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
CalibrationFilter::~CalibrationFilter()
{
 if (offset) delete[] offset;
 if (gain)   delete[] gain;
 if (vis)    delete vis;
 offset=gain=NULL;
 vis=NULL;

 // fprintf(calibf,"Destructing \n");
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the CalibrationFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int CalibrationFilter::Initialize(PARAMLIST *paramlist, STATEVECTOR *new_statevector, CORECOMM *new_corecomm)
{
int     alignyesno, visualizeyesno, numchoffset, numchgain, i;
int origchan;

 statevector=new_statevector;
 corecomm=new_corecomm;

 try // in case one of the parameters is not defined (should always be, since we requested them)
  {
  alignyesno=atoi(paramlist->GetParamPtr("AlignChannels")->GetValue());
  visualizeyesno=atoi(paramlist->GetParamPtr("VisualizeCalibration")->GetValue());
  numchoffset=paramlist->GetParamPtr("SourceChOffset")->GetNumValues();
  numchgain=paramlist->GetParamPtr("SourceChGain")->GetNumValues();

  recordedChans= atoi( paramlist->GetParamPtr("SoftwareCh")->GetValue() );
  transmittedChans= atoi( paramlist->GetParamPtr("TransmitCh")->GetValue() );

  //      paramlist.GetParamPtr("TransmitChList")->GetNumValues()
  //      paramlist.GetParamPtr("TransmitCh")->GetValue()

  }
 catch(...)
  { return(0); }

 // if the number of channels does not match for offset and gain, exit with an error
 if (numchoffset != numchgain) return(0);

 // allocate arrays for offsets and gains
 // we don't always want to query parameters
 if (offset) delete [] offset;
 if (gain) delete [] gain;

 offset=new float[numchoffset];
 gain=new float[numchgain];

 // copy the parameters in our private arrays
 for (i=0; i<numchoffset; i++)
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

    for(i=0;i<transmittedChans;i++)   // get original channel position
    {
        origchan= atoi(paramlist->GetParamPtr("TransmitChList")->GetValue(i));
        w2[i]= delta * (float)origchan;
        w1[i]= 1.0 - w2[i];
        old[i]= 0;
// fprintf(calibf,"Chan= %3d OrigChan = %3d \n",i,origchan);
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
    if (vis) delete vis;
    vis=new GenericVisualization(paramlist, corecomm);
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_WINDOWTITLE, "Calibration");
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_MINVALUE, "-40");
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_MAXVALUE, "40");
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_NUMSAMPLES, "256");
    }

 return(1);
}


// **************************************************************************
// Function:   Process
// Purpose:    This function applies the calibration routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int CalibrationFilter::Process(GenericSignal *input, GenericSignal *output)
{
int     channel, sample;
float   value;
float   temp;

 // actually perform the calibration on the input and write it into the output signal
 for(channel=0; channel<input->Channels; channel++)
  for(sample=0; sample<input->MaxElements; sample++)
   {
   value=(input->GetValue(channel, sample)-offset[channel])*gain[channel];

// fprintf(calibf,"value = %7.2f ",value);

   if( align == true )
   {
        temp= value;
        value= ( w1[channel]*temp ) + (w2[channel]*old[channel] );
        old[channel]= temp;
//        fprintf(calibf,"value2 = %7.2f w1[%3d]= %7.3f ",value,channel,w1[channel]);
   }

//   fprintf(calibf,"\n");

   output->SetValue(channel, sample, value);
   }

 // visualize the processed channels, if wanted
 if (visualize)
    {
    vis->SetSourceID(SOURCEID_CALIBRATION);
    vis->Send2Operator(output);
    }

 return(1);
}



