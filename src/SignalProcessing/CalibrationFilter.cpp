#undef USE_LOGFILE
//---------------------------------------------------------------------------
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

// **************************************************************************
// * 04/19/2004 - Introduced SourceChTimeOffset parameter (GS)              *
// **************************************************************************

#include "CalibrationFilter.h"
#include "UParameter.h"
#include "UGenericVisualization.h"

#ifdef USE_LOGFILE
# include <stdio.h>
FILE *calibf;
#endif // USE_LOGFILE

RegisterFilter( CalibrationFilter, 2.A );

// **************************************************************************
// Function:   CalibrationFilter
// Purpose:    This is the constructor for the CalibrationFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
CalibrationFilter::CalibrationFilter()
: offset( NULL ),
  gain( NULL ),
  vis( NULL )
{
 BEGIN_PARAMETER_DEFINITIONS
  "Filtering floatlist SourceChOffset= 16 "
    "0 0 0 0 "
    "0 0 0 0 "
    "0 0 0 0 "
    "0 0 0 0 "
    "0 -500 500 // offset for channels in A/D units",
  "Filtering floatlist SourceChGain= 16 "
    "0.003 0.003 0.003 0.003 "
    "0.003 0.003 0.003 0.003 "
    "0.003 0.003 0.003 0.003 "
    "0.003 0.003 0.003 0.003 "
    "0.003 -500 500 // gain for each channel (A/D units -> muV)",
  "Filtering floatlist SourceChTimeOffset= 1 -1 0 0 1 // time offset for all source channels (not used if -1)",
  "Filtering int AlignChannels= 0 0 0 1 "
    "// align channels in time (0=no, 1=yes)",
  "Visualize int VisualizeCalibration= 1 0 0 1 "
    "// visualize calibrated channels (0=no, 1=yes) (boolean)",
 END_PARAMETER_DEFINITIONS
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
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void CalibrationFilter::Preflight( const SignalProperties& inSignalProperties,
                                         SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  PreflightCondition( Parameter( "SoftwareCh" ) >= Parameter( "TransmitCh" ) );
  PreflightCondition( Parameter( "TransmitCh" ) ==
                                Parameter( "TransmitChList" )->GetNumValues() );
  for( size_t i = 0; i < Parameter( "TransmitChList" )->GetNumValues(); ++i )
    PreflightCondition( Parameter( "TransmitChList", i ) <= Parameter( "SoftwareCh" ) );

  // if the number of channels does not match for offset and gain there is an error
  PreflightCondition( Parameter( "SourceChOffset" )->GetNumValues() ==
                                 Parameter( "SourceChGain" )->GetNumValues() );

  // check the SourceChTimeOffset; only check if we do not use the default value
  if (Parameter( "SourceChTimeOffset", 0 ) >= 0)
     {
     // we need to have as many entries in SourceChTimeOffset as we have digitized channels
     PreflightCondition( Parameter( "SourceChTimeOffset" )->GetNumValues() == Parameter( "SoftwareCh" ) );
     // each value in SourceChTimeOffset has to be >= 0 and < 1
     for( size_t i = 0; i < Parameter( "SourceChTimeOffset" )->GetNumValues(); ++i )
      PreflightCondition( (Parameter( "SourceChTimeOffset", i ) >= 0) && (Parameter( "SourceChTimeOffset", i ) < 1) );
     }

  // Resource availability checks.
  /* The calibration filter seems not to depend on external resources. */

  // Input signal checks.
  PreflightCondition( inSignalProperties.Channels() == Parameter( "TransmitChList" )->GetNumValues() );

  // Requested output signal properties.
  outSignalProperties = inSignalProperties;
  outSignalProperties.Type() = SignalType::float32;
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the CalibrationFilter
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void CalibrationFilter::Initialize()
{
  recordedChans= Parameter("SoftwareCh");
  transmittedChans= Parameter("TransmitChList")->GetNumValues();

  // allocate arrays for offsets and gains
  // we don't always want to query parameters
  delete [] offset;
  delete [] gain;

  offset=new float[ transmittedChans ];
  gain=new float[ transmittedChans ];

  // copy the parameters in our private arrays
  for (int i=0; i< transmittedChans; i++)
  {
    int origChan = Parameter( "TransmitChList", i );
    offset[i]=Parameter("SourceChOffset", origChan - 1 );
    gain[i]=Parameter("SourceChGain", origChan - 1 );
  }

  // do we want to align the samples in time ?
  align = ( Parameter( "AlignChannels" ) == 1 );
  if( align )
  {
    // calculate the weight values if we do not use the default value
    // if we do use the default value, assume that all sampled channels are evenly distributed in time
    if (Parameter("SourceChTimeOffset", 0) >= 0)
       {
       for(int i=0;i<transmittedChans;i++)   // get original channel position
        {
        int origchan= Parameter("TransmitChList",i);
        w2[i]= Parameter("SourceChTimeOffset", origchan - 1 );
        w1[i]= 1.0 - w2[i];
        old[i]= 0;
        }
       }
    else // assume that all sampled channels are evenly distributed in time
       {
       delta= (float)recordedChans;
       delta= 1/delta;
       for(int i=0;i<transmittedChans;i++)   // get original channel position
        {
        int origchan= Parameter("TransmitChList",i);
        w2[i]= delta * (float)(origchan-1);
        w1[i]= 1.0 - w2[i];
        old[i]= 0;
        #ifdef USE_LOGFILE
        fprintf(calibf,"Chan= %3d OrigChan = %3d \n",i,origchan);
        #endif // USE_LOGFILE
        }
       }
  }

  // do we want to visualize ?
  visualize = ( Parameter( "VisualizeCalibration" ) == 1 );
  if( visualize )
  {
    // create an instance of GenericVisualization
    // it will handle the visualization to the operator
    delete vis;
    vis=new GenericVisualization;
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_WINDOWTITLE, "Calibration");
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_MINVALUE, "-40");
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_MAXVALUE, "40");
    vis->SendCfg2Operator(SOURCEID_CALIBRATION, CFGID_NUMSAMPLES, "256");
  }
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
    for(size_t sample=0; sample<input->Elements(); sample++)
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
}



