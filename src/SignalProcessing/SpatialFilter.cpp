#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "SpatialFilter.h"

#include "UParameter.h"
#include "UGenericVisualization.h"

RegisterFilter( SpatialFilter, 2.B );

// **************************************************************************
// Function:   SpatialFilter
// Purpose:    This is the constructor for the SpatialFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
SpatialFilter::SpatialFilter()
: vis( NULL )
{
 BEGIN_PARAMETER_DEFINITIONS
   "Filtering int SpatialFilteredChannels= 2 "
     "2 1 64 // Number of Spatially Filtered Channels",
   "Filtering matrix SpatialFilterKernal= 2 4 "
     "1 0 0 0 "
     "0 1 0 0 "
     "64 -100 100 // Spatial Filter Kernal Weights",
/*
   "Source int TransmitCh= 4 "
     "5 1 128 // this is the number of transmitted channels",
*/
   "Visualize int VisualizeSpatialFiltering= 1 "
     "0 0 1 // visualize spatial filtered signals (0=no 1=yes) (boolean)",
 END_PARAMETER_DEFINITIONS
}


// **************************************************************************
// Function:   ~SpatialFilter
// Purpose:    This is the destructor for the SpatialFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SpatialFilter::~SpatialFilter()
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
void SpatialFilter::Preflight( const SignalProperties& inSignalProperties,
                                     SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  Parameter("SampleBlockSize");
  PreflightCondition(
    Parameter("TransmitCh") == Parameter("SpatialFilterKernal")->GetNumValuesDimension2() );
  PreflightCondition(
    Parameter("SpatialFilteredChannels") == Parameter("SpatialFilterKernal")->GetNumValuesDimension1() );
  PreflightCondition(
     Parameter( "SpatialFilterKernal" )->GetNumValuesDimension1() <= MAX_M );
  PreflightCondition(
     Parameter( "SpatialFilterKernal" )->GetNumValuesDimension2() <= MAX_N );

  // Resource availability checks.
  /* The spatial filter seems not to depend on external resources. */

  // Input signal checks.
  PreflightCondition( Parameter( "TransmitCh" ) <= inSignalProperties.Channels() );

  // Requested output signal properties.
  outSignalProperties = SignalProperties(
                                 Parameter( "SpatialFilteredChannels" ),
                                 inSignalProperties.Elements() );
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the SpatialFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    N/A
// **************************************************************************
void SpatialFilter::Initialize()
{
  int visualizeyn = 0;

  samples=Parameter("SampleBlockSize");
  n_mat= Parameter("TransmitCh");
  m_mat= Parameter("SpatialFilteredChannels");
  visualizeyn= Parameter("VisualizeSpatialFiltering");

  for(int i=0;i<m_mat;i++)
  {
    for(int j=0;j<n_mat;j++)
    {
      mat[i][j]= Parameter("SpatialFilterKernal",i,j);
    }
  }

  if( visualizeyn == 1 )
  {
    visualize=true;
    delete vis;
    vis= new GenericVisualization;
    if (vis)
    {
      vis->SendCfg2Operator(SOURCEID_SPATFILT, CFGID_WINDOWTITLE, "Spatial Filter");
      vis->SendCfg2Operator(SOURCEID_SPATFILT, CFGID_MINVALUE, "-40");
      vis->SendCfg2Operator(SOURCEID_SPATFILT, CFGID_MAXVALUE, "40");
      vis->SendCfg2Operator(SOURCEID_SPATFILT, CFGID_NUMSAMPLES, "256");
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
// Purpose:    This function applies the Spatial routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    N/A
// **************************************************************************
void SpatialFilter::Process(const GenericSignal *input, GenericSignal *output)
{
  // actually perform the Spatial Filtering on the input and write it into the output signal
  for(size_t sample=0; sample<input->Elements(); sample++)
  {
    for(size_t out_channel= 0; out_channel<output->Channels(); out_channel++)
    {
      float value= 0;
      for(size_t in_channel=0; in_channel<input->Channels(); in_channel++)
      {
        value+= mat[out_channel][in_channel] * input->GetValue(in_channel, sample);
      }
      output->SetValue(out_channel, sample, value);
    }
  }
  if( visualize )
  {
    vis->SetSourceID(SOURCEID_SPATFILT);
    vis->Send2Operator(output);
  }
  return;
}



